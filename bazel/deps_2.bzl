# Copyright 2023 Intrinsic Innovation LLC

"""Workspace dependencies needed for the Intrinsic SDKs as a 3rd-party consumer (part 2)."""

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")
load("@local_config_python//:defs.bzl", "interpreter")
load("@rules_oci//oci:dependencies.bzl", "rules_oci_dependencies")
load("@rules_oci//oci:repositories.bzl", "LATEST_CRANE_VERSION", "oci_register_toolchains")
load("@rules_python//python:pip.bzl", "pip_parse")
load("//bazel:extension_for_rules_oci.bzl", "extension_for_rules_oci")

def intrinsic_sdks_deps_2():
    """Loads workspace dependencies needed for the Intrinsic SDKs.

    This is one out of several non-optional parts. Please see intrinsic_sdks_deps_0() in
    intrinsic_sdks_deps_0.bzl for more details."""

    # CC toolchain
    llvm_register_toolchains()

    # gRPC
    grpc_extra_deps()

    # Python pip dependencies
    # Load pip dependencies lazily according to
    # https://github.com/bazelbuild/rules_python/blob/deb43b03397d3dba810ce570a4ac89b40aaf2723/README.md#fetch-pip-dependencies-lazily
    pip_parse(
        name = "ai_intrinsic_sdks_pip_deps",
        python_interpreter_target = interpreter,
        requirements_lock = Label("//:requirements.txt"),
    )

    # Container images
    rules_oci_dependencies()
    oci_register_toolchains(
        name = "oci",
        crane_version = LATEST_CRANE_VERSION,
    )
    extension_for_rules_oci()
