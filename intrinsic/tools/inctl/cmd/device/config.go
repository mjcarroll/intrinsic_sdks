// Copyright 2023 Intrinsic Innovation LLC

package device

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"

	backoff "github.com/cenkalti/backoff/v4"
	"github.com/spf13/cobra"
	"go.uber.org/multierr"
	"intrinsic/frontend/cloud/devicemanager/shared"
	"intrinsic/tools/inctl/cmd/device/projectclient"
	"intrinsic/tools/inctl/cmd/root"
	"intrinsic/tools/inctl/util/orgutil"
	"intrinsic/tools/inctl/util/printer"
)

var (
	errConfigGone = fmt.Errorf("config was rejected")
)

func prettyPrintStatusInterfaces(interfaces map[string]shared.StatusInterface) string {
	ret := ""
	for name, iface := range interfaces {
		ret = ret + fmt.Sprintf("\t%s: %v\n", name, iface.IPAddress)
	}

	return ret
}

var configCmd = &cobra.Command{
	Use:   "config",
	Short: "Tool to interact with device config",
}

type networkConfigInfo struct {
	Current map[string]shared.StatusInterface `json:"current"`
	Config  string                            `json:"config"`
}

func (info *networkConfigInfo) String() string {
	return "Current state:\n" + prettyPrintStatusInterfaces(info.Current) + "Current config: " + info.Config
}

var configGetCmd = &cobra.Command{
	Use:   "get",
	Short: "Retrieve current configuration",
	RunE: func(cmd *cobra.Command, args []string) error {
		prtr, err := printer.NewPrinter(root.FlagOutput)
		if err != nil {
			return err
		}

		projectName := viperLocal.GetString(orgutil.KeyProject)
		orgName := viperLocal.GetString(orgutil.KeyOrganization)

		client, err := projectclient.Client(projectName, orgName)
		if err != nil {
			return fmt.Errorf("get project client: %w", err)
		}

		var status shared.Status
		if err := client.GetJSON(cmd.Context(), clusterName, deviceID, "relay/v1alpha1/status", &status); err != nil {
			if errors.Is(err, projectclient.ErrNotFound) {
				fmt.Fprintf(os.Stderr, "Cluster does not exist. Either it does not exist, or you don't have access to it.\n")
				return err
			}

			return fmt.Errorf("get status: %w", err)
		}
		prettyPrintStatusInterfaces(status.Network)

		res, err := client.GetDevice(cmd.Context(), clusterName, deviceID, "relay/v1alpha1/config/network")
		if err != nil {
			return fmt.Errorf("get config: %w", err)
		}
		defer res.Body.Close()

		if res.StatusCode != http.StatusOK {
			io.Copy(os.Stderr, res.Body)
			return fmt.Errorf("http code %v", res.StatusCode)
		}
		body, err := io.ReadAll(res.Body)
		if err != nil {
			return fmt.Errorf("read config: %w", err)
		}
		prtr.Print(&networkConfigInfo{Current: status.Network, Config: string(body)})

		if res.StatusCode != 200 {
			return fmt.Errorf("request failed")
		}

		return nil
	},
}

// applyConfig tries to call the apply endpoint for the device periodically for a maximum of 3 minutes.
// This persists the network configuration to disk.
// The configuration was already sent and tentatively applied with POST /v1alpha1/config/network.
// We need to retry because the device may be briefly unreachable while it changes its network config.
func applyConfig(ctx context.Context, client *projectclient.AuthedClient, clusterName, deviceID string) error {
	ctx, stop := context.WithTimeout(ctx, time.Minute*3)
	defer stop()

	var connectionError error

	fmt.Printf("Trying to apply")
	os.Stdout.Sync()

	err := backoff.RetryNotify(func() error {
		// There's a shorter timeout on the actual request, because the network re-configuration can lead to a hung request.
		// Likely due to some drop in the relay.
		// The lower timeout here guarantees multiple requests in the 2 minute time frame.
		ctx, stop := context.WithTimeout(ctx, time.Second*10)
		defer stop()
		fmt.Printf(".")
		os.Stdout.Sync()

		resp, err := client.PostDevice(ctx, clusterName, deviceID, "relay/v1alpha1/config/network:persist", nil)
		if err != nil {
			return err
		}
		defer resp.Body.Close()

		if resp.StatusCode != 200 {
			// In this case, 404 signals an older OS which doesn't do the apply flow yet.
			// Return the error and adapt the output
			if resp.StatusCode == http.StatusNotFound {
				return backoff.Permanent(projectclient.ErrNotFound)
			}

			if resp.StatusCode == http.StatusGone {
				return backoff.Permanent(errConfigGone)
			}

			return fmt.Errorf("request failed: %v", resp.StatusCode)
		}

		return nil
	}, backoff.WithContext(backoff.NewConstantBackOff(time.Second*5), ctx),
		func(err error, _ time.Duration) { connectionError = multierr.Append(connectionError, err) })
	fmt.Printf("\n")

	if err != nil {
		return multierr.Append(err, connectionError)
	}

	return nil
}

func setConfig(ctx context.Context, client *projectclient.AuthedClient, clusterName, deviceID, config string) error {
	ctx, cancel := context.WithTimeout(ctx, time.Second*30)
	defer cancel()

	const timeoutWarning = "Warning: Timeout while sending config to the device. This may indicate that the config is unusable, but could also be a transient network error."
	resp, err := client.PostDevice(ctx, clusterName, deviceID, "relay/v1alpha1/config/network", strings.NewReader(config))
	if err != nil {
		if errors.Is(err, context.DeadlineExceeded) {
			fmt.Println(timeoutWarning)
			return nil
		}
		if errors.Is(err, projectclient.ErrNotFound) {
			fmt.Fprintf(os.Stderr, "Cluster does not exist. Either it does not exist, or you don't have access to it.\n")
			return err
		}

		return fmt.Errorf("post config: %w", err)
	}
	defer resp.Body.Close()

	switch resp.StatusCode {
	case http.StatusOK:
		// Do nothing
	case http.StatusGatewayTimeout:
		fmt.Println(timeoutWarning)
		return nil
	case http.StatusNotFound:
		fmt.Fprintf(os.Stderr, "Cluster does not exist. Either it does not exist, or you don't have access to it.\n")
		return fmt.Errorf("http code %v", resp.StatusCode)
	default:
		io.Copy(os.Stderr, resp.Body)
		return fmt.Errorf("server returned error: %v", resp.StatusCode)
	}

	return nil
}

var configSetCmd = &cobra.Command{
	Use:   "set",
	Short: "Set the network config",
	Args:  cobra.MatchAll(cobra.ExactArgs(1), cobra.OnlyValidArgs),
	ValidArgsFunction: func(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
		var comps []string
		if len(args) == 0 {
			comps = cobra.AppendActiveHelp(comps, "You must provide a valid network configuration in json format.")
		}

		return comps, cobra.ShellCompDirectiveNoFileComp
	},

	RunE: func(cmd *cobra.Command, args []string) error {
		configString := args[0]
		projectName := viperLocal.GetString(orgutil.KeyProject)
		orgName := viperLocal.GetString(orgutil.KeyOrganization)
		client, err := projectclient.Client(projectName, orgName)
		if err != nil {
			return fmt.Errorf("get project client: %w", err)
		}

		if err := json.Unmarshal([]byte(configString), &networkConfigInfo{}); err != nil {
			fmt.Fprintf(os.Stderr, "Provided configuration is not a valid configuration string.\n")
			return err
		}

		if err := setConfig(cmd.Context(), &client, clusterName, deviceID, configString); err != nil {
			return fmt.Errorf("set config: %w", err)
		}

		if err := applyConfig(cmd.Context(), &client, clusterName, deviceID); err != nil {
			if errors.Is(err, projectclient.ErrNotFound) {
				fmt.Println("The device is running an older version of INTRINSIC-OS. Please reboot manually")
				return nil
			}

			if errors.Is(err, errConfigGone) {
				fmt.Println("The device rejected the network configuration. This happens when it cannot connect to the configuration server with the new configuration.")
				return errConfigGone
			}

			fmt.Println("There was an unexpected error trying to configure the device. It may be in an undefined state.")
			return err
		}

		fmt.Println("Successfully applied new network configuration to the device.")
		return nil
	}}

func init() {
	deviceCmd.AddCommand(configCmd)
	configCmd.AddCommand(configGetCmd)
	configCmd.AddCommand(configSetCmd)
}