// Copyright 2023 Intrinsic Innovation LLC

// Package uninstall defines the skill command which uninstalls a skill.
package uninstall

import (
	"fmt"
	"log"

	"github.com/google/go-containerregistry/pkg/v1/google"
	"github.com/google/go-containerregistry/pkg/v1/remote"
	"github.com/spf13/cobra"
	"intrinsic/assets/cmdutils"
	"intrinsic/assets/imagetransfer"
	"intrinsic/assets/imageutils"
	installerpb "intrinsic/kubernetes/workcell_spec/proto/installer_go_grpc_proto"
	"intrinsic/skills/tools/skill/cmd"
	"intrinsic/skills/tools/skill/cmd/dialerutil"
	"intrinsic/skills/tools/skill/cmd/solutionutil"
)

var cmdFlags = cmdutils.NewCmdFlags()

var uninstallCmd = &cobra.Command{
	Use:   "uninstall --type=TYPE TARGET",
	Short: "Remove a skill",
	Example: `Stop a running skill using its build target
$ inctl skill uninstall --type=build //abc:skill.tar --context=minikube

Stop a running skill using an already-built image file
$ inctl skill uninstall --type=archive abc/skill.tar --context=minikube

Stop a running skill using an already-pushed image
$ inctl skill uninstall --type=image gcr.io/my-workcell/abc@sha256:20ab4f --context=minikube

Use the solution flag to automatically resolve the context (requires the solution to run)
$ inctl skill uninstall --type=archive abc/skill.tar --solution=my-solution

Stop a running skill by specifying its id
$ inctl skill uninstall --type=id com.foo.skill

Stop a running skill by specifying its name [deprecated]
$ inctl skill uninstall --type=id skill
`,
	Args: cobra.ExactArgs(1),
	Aliases: []string{
		"stop",
		"unload",
	},
	RunE: func(command *cobra.Command, args []string) error {
		target := args[0]
		targetType := imageutils.TargetType(cmdFlags.GetFlagSideloadStopType())
		if targetType != imageutils.Build && targetType != imageutils.Archive && targetType != imageutils.Image && targetType != imageutils.ID && targetType != imageutils.Name {
			return fmt.Errorf("type must be one of (%s, %s, %s, %s, %s)", imageutils.Build, imageutils.Archive, imageutils.Image, imageutils.ID, imageutils.Name)
		}

		context, solution := cmdFlags.GetFlagsSideloadContextSolution()
		installerAddress := cmdFlags.GetNormalizedInstallerAddress()
		project := cmdFlags.GetFlagProject()
		org := cmdFlags.GetFlagOrganization()

		skillID, err := imageutils.SkillIDFromTarget(target, imageutils.TargetType(targetType), imagetransfer.RemoteTransferer(remote.WithAuthFromKeychain(google.Keychain)))
		if err != nil {
			return fmt.Errorf("could not get skill ID: %v", err)
		}

		ctx, conn, err := dialerutil.DialConnectionCtx(command.Context(), dialerutil.DialInfoParams{
			Address:  installerAddress,
			CredName: project,
			CredOrg:  org,
		})
		if err != nil {
			return fmt.Errorf("could not create connection: %w", err)
		}
		defer conn.Close()

		cluster, err := solutionutil.GetClusterNameFromSolutionOrDefault(
			ctx,
			conn,
			solution,
			context,
		)
		if err != nil {
			return fmt.Errorf("could not resolve solution to cluster: %s", err)
		}

		// Remove the skill from the registry
		ctx, conn, err = dialerutil.DialConnectionCtx(command.Context(), dialerutil.DialInfoParams{
			Address:  installerAddress,
			Cluster:  cluster,
			CredName: project,
		})
		if err != nil {
			return fmt.Errorf("could not remove the skill: %w", err)
		}

		log.Printf("Removing skill %q using the installer service at %q", skillID, installerAddress)
		if err := imageutils.RemoveContainer(ctx, &imageutils.RemoveContainerParams{
			Address:    installerAddress,
			Connection: conn,
			Request: &installerpb.RemoveContainerAddonRequest{
				Id:   skillID,
				Type: installerpb.AddonType_ADDON_TYPE_SKILL,
			},
		}); err != nil {
			return fmt.Errorf("could not remove the skill: %w", err)
		}
		log.Print("Finished removing the skill")

		return nil
	},
}

func init() {
	cmd.SkillCmd.AddCommand(uninstallCmd)
	cmdFlags.SetCommand(uninstallCmd)

	cmdFlags.AddFlagInstallerAddress()
	cmdFlags.AddFlagsProjectOrg()
	cmdFlags.AddFlagsSideloadContextSolution("skill")
	cmdFlags.AddFlagSideloadStopType("skill")
}
