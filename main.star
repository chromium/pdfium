#!/usr/bin/env lucicfg
#
# Copyright 2021 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""LUCI project configuration for pdfium CQ and CI."""

# Defaults.
luci.builder.defaults.build_numbers.set(True)
luci.builder.defaults.execution_timeout.set(3 * time.hour)
luci.builder.defaults.swarming_host.set("chromium-swarm.appspot.com")

# Constants.

_CIPD_PACKAGE = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build"
_CONSOLE_HEADER = {
    "tree_status_host": "pdfium-status.appspot.com",
}

# How long to wait for a swarmed test task to start executing. If the task fails
# to schedule, the swarming pool probably is busy, and the PDFium recipe falls
# back to executing the test locally.
_SWARMING_TEST_SCHEDULE_TIMEOUT = 5 * time.minute

# How long to wait for a swarmed test task to finish executing.
_SWARMING_TEST_EXECUTION_TIMEOUT = 20 * time.minute

# Dicts for OS-specific dimensions.
_LINUX_FOCAL_DIMENSIONS = {
    "cores": "8",
    "cpu": "x86-64",
    "os": "Ubuntu-20.04",
}
_MACOS_INTEL_DIMENSIONS = {
    "cpu": "x86-64",
    "os": "Mac-12",
}
_MACOS_ARM_DIMENSIONS = {
    "cpu": "arm64",
    "os": "Mac-13",
}
_WINDOWS_INTEL_DIMENSIONS = {
    "cores": "8",
    "cpu": "x86-64",
    "os": "Windows-10",
}
_WINDOWS_ARM_DIMENSIONS = {
    "cpu": "arm64",
    "os": "Windows-11",
}

# Dicts for OS-specifc properties.
_ANDROID_PROPERTIES = {
    "skip_test": True,
    "target_os": "android",
}

# Helper functions
def get_properties_by_name(name):
    """Updates builder properties by parsing the given builder name.

    Args:
      name: The builder name.

    Returns:
      A dictionary of builder properties.
    """
    properties = {}

    # Sets OS-specific properties.
    if name.startswith("android"):
        properties.update(_ANDROID_PROPERTIES)

    if name.find("no_v8") != -1:
        properties.update({"v8": False})

    if name.find("asan") != -1:
        properties.update({"memory_tool": "asan"})
        if not name.endswith("lsan"):
            properties.update({"rel": True})

    if name.endswith("msan"):
        properties.update({"memory_tool": "msan", "rel": True})

    if name.endswith("ubsan"):
        properties.update({"memory_tool": "ubsan", "rel": True})

    if name.endswith("rel"):
        properties.update({"rel": True})

    if name.find("xfa") != -1:
        properties.update({"xfa": True})

    is_32_bit = name.endswith("32")
    if name.startswith("android"):
        # Android bots default to "arm". (32-bit)
        if not is_32_bit:
            properties.update({"target_cpu": "arm64"})
    elif is_32_bit:
        properties.update({"target_cpu": "x86"})

    if name.find("component") != -1:
        properties.update({"component": True})

    if name.find("rust") != -1:
        properties.update({"rust": True})

    if name.find("skia") != -1:
        properties.update({"skia": True})

    renderers = []
    if "gdi" in name:
        renderers.append("gdi")
    if renderers:
        properties.update({"renderers": renderers})

    return properties

def get_rbe_properties(name, bucket):
    """Returns RBE properties for a given builder name and bucket

    Args:
        name: name of the builder
        bucket: builder's bucket

    Returns:
        $build/{reclient, siso} properties
    """
    reclient_props = {
        "instance": "rbe-chromium-trusted" if bucket == "ci" else "rbe-chromium-untrusted",
        "metrics_project": "chromium-reclient-metrics",
    }
    siso_props = {
        "project": "rbe-chromium-trusted" if bucket == "ci" else "rbe-chromium-untrusted",
        "configs": ["builder"],
        "enable_cloud_profiler": True,
        "enable_cloud_trace": True,
        "enable_monitoring": True,
    }
    if name.startswith("mac"):
        reclient_props.update({"scandeps_server": True})
    return {
        "$build/reclient": reclient_props,
        "$build/siso": siso_props,
    }

def pdfium_internal_builder(name, bucket, swarm_tests):
    """Creates a builder based on the builder name and the bucket name.

    Args:
      name: The builder name.
      bucket: The name of the bucket that the builder belongs to.
      swarm_tests: Whether to enable swarming for test tasks.

    Returns:
      The desired builder.
    """
    caches = None
    dimensions = {}
    notifies = None
    properties = {}
    triggered_by = None

    # Set bucket-specific configs.
    if bucket == "ci":
        dimensions.update({"pool": "luci.flex.ci"})
        notifies = ["pdfium main notifier", "pdfium tree closer"]
        properties.update({"builder_group": "client.pdfium"})
        service_account = "pdfium-ci-builder@chops-service-accounts.iam.gserviceaccount.com"
        triggered_by = ["pdfium-gitiles-trigger"]
    else:
        dimensions.update({"pool": "luci.flex.try"})
        properties.update({"builder_group": "tryserver.client.pdfium"})
        service_account = "pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com"

    # Set configs depending on the OS.
    if name.startswith("linux"):
        dimensions.update(_LINUX_FOCAL_DIMENSIONS)
    elif name.startswith("mac") and "_arm" in name:
        dimensions.update(_MACOS_ARM_DIMENSIONS)
        caches = [swarming.cache("osx_sdk", name = "osx_sdk")]
    elif name.startswith("mac"):
        dimensions.update(_MACOS_INTEL_DIMENSIONS)
        caches = [swarming.cache("osx_sdk", name = "osx_sdk")]
    elif name.startswith("win") and "_arm" in name:
        dimensions.update(_WINDOWS_ARM_DIMENSIONS)
    elif name.startswith("win"):
        dimensions.update(_WINDOWS_INTEL_DIMENSIONS)
    elif name.startswith("android"):
        dimensions.update(_LINUX_FOCAL_DIMENSIONS)
    else:
        fail()

    # Update properties based on the builder name.
    properties.update(get_properties_by_name(name))

    properties.update(get_rbe_properties(name, bucket))

    if swarm_tests:
        properties.update({
            "swarming": {
                "dimensions": dimensions,
                "execution_timeout_secs": _SWARMING_TEST_EXECUTION_TIMEOUT / time.second,
                "expiration_secs": _SWARMING_TEST_SCHEDULE_TIMEOUT / time.second,

                # TODO(crbug.com/1465963): Ideally would be a test-only account.
                "service_account": service_account,
            },
        })

    return luci.builder(
        name = name,
        bucket = bucket,
        caches = caches,
        dimensions = dimensions,
        executable = "pdfium",
        notifies = notifies,
        properties = properties,
        service_account = service_account,
        resultdb_settings = resultdb.settings(enable = True),
        triggered_by = triggered_by,
    )

def add_entries_for_builder(
        name,
        category = None,
        short_name = None,
        skip_ci_builder = False,
        swarm_tests = False):
    """Creates builder, tryjob verifier and view entries for a given builder name.

    Args:
      name: The name for the builder to be created.
      category: The category name for the console view entry.
      short_name: The short name to be displayed in the console view.
      skip_ci_builder: Whether to skip creating a builder in the "ci" bucket.
          When set to True, it will also skip making the matching console view
          entry for that "ci" builder.
      swarm_tests: Whether to enable swarming for test tasks.
    """

    # Add builders in buckets.
    if not skip_ci_builder:
        pdfium_internal_builder(
            name,
            bucket = "ci",
            swarm_tests = swarm_tests,
        )

        # Add the matching console view entry.
        luci.console_view_entry(
            console_view = "main",
            builder = "ci/" + name,
            category = category,
            short_name = short_name,
        )

        # Add the matching tryjob verifier.
        luci.cq_tryjob_verifier(
            builder = "pdfium:try/" + name,
            cq_group = "pdfium",
        )

    pdfium_internal_builder(
        name,
        bucket = "try",
        swarm_tests = swarm_tests,
    )

    # Add the matching list view entry.
    luci.list_view_entry(
        list_view = "try",
        builder = "try/" + name,
    )

# End of helpers and constants. Below we actually generate the configs.

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

lucicfg.config(
    config_dir = "generated",
    tracked_files = [
        "commit-queue.cfg",
        "cr-buildbucket.cfg",
        "luci-logdog.cfg",
        "luci-milo.cfg",
        "luci-notify.cfg",
        "luci-notify/email-templates/*.template",
        "luci-scheduler.cfg",
        "project.cfg",
        "realms.cfg",
    ],
    fail_on_warnings = True,
    lint_checks = ["default"],
)

# Project
luci.project(
    name = "pdfium",
    buildbucket = "cr-buildbucket.appspot.com",
    logdog = "luci-logdog.appspot.com",
    milo = "luci-milo.appspot.com",
    notify = "luci-notify.appspot.com",
    scheduler = "luci-scheduler.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(
            roles = [
                acl.BUILDBUCKET_READER,
                acl.LOGDOG_READER,
                acl.PROJECT_CONFIGS_READER,
                acl.SCHEDULER_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            roles = [
                acl.SCHEDULER_OWNER,
            ],
            groups = "project-pdfium-admins",
        ),
        acl.entry(
            roles = [
                acl.LOGDOG_WRITER,
            ],
            groups = [
                "luci-logdog-chromium-writers",
            ],
        ),
    ],
)

luci.logdog(gs_bucket = "chromium-luci-logdog")

luci.milo(
    logo = "https://storage.googleapis.com/chrome-infra/pdfium-logo.png",
)

luci.notify(
    tree_closing_enabled = True,
)

luci.tree_closer(
    name = "pdfium tree closer",
    tree_status_host = "pdfium-status.appspot.com",
)
luci.notifier(
    name = "pdfium main notifier",
    on_new_status = ["FAILURE"],
    notify_emails = [
        "andyphan@chromium.org",
        "awscreen@chromium.org",
        "thestig@chromium.org",
    ],
    notify_blamelist = True,
    template = "tree_closure_email_template",
)

luci.notifier_template(
    name = "tree_closure_email_template",
    body = io.read_file("template/tree_closure_email.template"),
)

# Recipes
luci.recipe(
    name = "pdfium",
    cipd_package = _CIPD_PACKAGE,
    use_bbagent = True,
    use_python3 = True,
)

luci.recipe(
    name = "presubmit",
    cipd_package = _CIPD_PACKAGE,
    use_bbagent = True,
    use_python3 = True,
)

luci.recipe(
    name = "gcs_dep_autoroller",
    cipd_package = "infra/recipe_bundles/chromium.googlesource.com/infra/infra",
    use_bbagent = True,
    use_python3 = True,
)

# Buckets
luci.bucket(
    name = "ci",
    acls = [
        acl.entry(
            acl.BUILDBUCKET_OWNER,
            groups = "project-pdfium-admins",
        ),
    ],
)

luci.bucket(
    name = "try",
    acls = [
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
            groups = [
                "project-pdfium-tryjob-access",
                "service-account-cq",
            ],
        ),
        acl.entry(
            acl.BUILDBUCKET_OWNER,
            groups = "project-pdfium-admins",
        ),
    ],
)

luci.bucket(
    name = "try.shadow",
    shadows = "try",
    constraints = luci.bucket_constraints(
        pools = ["luci.flex.try"],
        service_accounts = ["pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com"],
    ),
    bindings = [
        luci.binding(
            roles = "role/buildbucket.creator",
            groups = "project-pdfium-tryjob-access",
        ),
    ],
    dynamic = True,
)

# Builders
luci.builder(
    name = "pdfium_gcs_dep_autoroller",
    bucket = "ci",
    executable = "gcs_dep_autoroller",
    service_account = "pdfium-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    dimensions = {
        "cores": "8",
        "cpu": "x86-64",
        "os": "Ubuntu-20.04",
        "pool": "luci.flex.ci",
    },
    properties = {
        "source_url": "https://chromium.googlesource.com/chromium/src.git",
        "destination_url": "https://pdfium.googlesource.com/pdfium.git",
        "source_packages": "src/third_party/llvm-build/Release+Asserts,src/third_party/rust-toolchain",
        "destination_packages": "third_party/llvm-build/Release+Asserts,third_party/rust-toolchain",
    },
    # Run every 2 weeks on the 1st and the 15th of each month at 1:30 AM
    schedule = "30 1 1,15 * *",
)

luci.builder(
    name = "pdfium_presubmit",
    bucket = "try",
    executable = "presubmit",
    service_account = "pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    dimensions = {
        "cores": "8",
        "cpu": "x86-64",
        "os": "Ubuntu-20.04",
        "pool": "luci.flex.try",
    },
    # Give this the highest priority, so CLs that are waiting to land can get
    # through the presubmit earlier. Presubmit builds run quickly, so they will
    # not have a big effect on other pending builds.
    priority = 20,
    properties = {
        "builder_group": "tryserver.client.pdfium",
        "repo_name": "pdfium",
        "$depot_tools/presubmit": {
            "runhooks": True,
            "timeout_s": 480,
        },
    },
)

add_entries_for_builder(name = "android", category = "main|android")
add_entries_for_builder(name = "android_32", category = "main|android")
add_entries_for_builder(name = "android_no_v8", category = "no v8", short_name = "android")
add_entries_for_builder(name = "android_no_v8_32", category = "no v8", short_name = "android_32")
add_entries_for_builder(name = "linux", category = "main|linux", swarm_tests = True)
add_entries_for_builder(name = "linux_asan_lsan", category = "main|linux", short_name = "asan", swarm_tests = True)
add_entries_for_builder(name = "linux_msan", category = "main|linux", short_name = "msan", swarm_tests = True)
add_entries_for_builder(name = "linux_no_v8", category = "no v8", short_name = "linux", swarm_tests = True)
add_entries_for_builder(name = "linux_skia", category = "skia|linux", swarm_tests = True)
add_entries_for_builder(name = "linux_skia_asan_lsan", category = "skia|linux", short_name = "asan", swarm_tests = True)
add_entries_for_builder(name = "linux_skia_msan", category = "skia|linux", short_name = "msan", swarm_tests = True)
add_entries_for_builder(name = "linux_skia_rust", category = "skia|linux", swarm_tests = True)
add_entries_for_builder(name = "linux_skia_ubsan", category = "skia|linux", short_name = "ubsan", swarm_tests = True)
add_entries_for_builder(name = "linux_ubsan", category = "main|linux", short_name = "ubsan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa", category = "xfa|linux", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_asan_lsan", category = "xfa|linux", short_name = "asan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_component", category = "xfa|linux", short_name = "comp", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_msan", category = "xfa|linux", short_name = "msan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_rel", category = "xfa|linux", short_name = "rel", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_skia", category = "skia|linux", short_name = "xfa", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_skia_asan_lsan", category = "skia|linux", short_name = "asan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_skia_component", category = "skia|linux", short_name = "comp", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_skia_msan", category = "skia|linux", short_name = "msan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_skia_ubsan", category = "skia|linux", short_name = "ubsan", swarm_tests = True)
add_entries_for_builder(name = "linux_xfa_ubsan", category = "xfa|linux", short_name = "ubsan", swarm_tests = True)
add_entries_for_builder(name = "mac", category = "main|mac", swarm_tests = True)
add_entries_for_builder(name = "mac_asan", skip_ci_builder = True)
add_entries_for_builder(name = "mac_no_v8", category = "no v8", short_name = "mac")
add_entries_for_builder(name = "mac_skia", category = "skia|mac")
add_entries_for_builder(name = "mac_xfa", category = "xfa|mac")
add_entries_for_builder(name = "mac_xfa_arm", category = "xfa|mac", skip_ci_builder = True)
add_entries_for_builder(name = "mac_xfa_asan", skip_ci_builder = True)
add_entries_for_builder(name = "mac_xfa_component", category = "xfa|mac", short_name = "comp")
add_entries_for_builder(name = "mac_xfa_rel", category = "xfa|mac", short_name = "rel")
add_entries_for_builder(name = "mac_xfa_skia", category = "skia|mac", short_name = "xfa")
add_entries_for_builder(name = "mac_xfa_skia_rust", category = "skia|mac", short_name = "rust", skip_ci_builder = True)
add_entries_for_builder(name = "mac_xfa_skia_component", category = "skia|mac", short_name = "comp")
add_entries_for_builder(name = "win", category = "main|win", swarm_tests = True)
add_entries_for_builder(name = "win_asan", category = "main|win", short_name = "asan")
add_entries_for_builder(name = "win_no_v8", category = "no v8", short_name = "win")
add_entries_for_builder(name = "win_skia", category = "skia|win")
add_entries_for_builder(name = "win_skia_asan", category = "skia|win", short_name = "asan")
add_entries_for_builder(name = "win_xfa", category = "xfa|win")
add_entries_for_builder(name = "win_xfa_32", category = "xfa|win", short_name = "32")
add_entries_for_builder(name = "win_xfa_arm", category = "xfa|win", skip_ci_builder = True)
add_entries_for_builder(name = "win_xfa_asan", category = "xfa|win", short_name = "asan")
add_entries_for_builder(name = "win_xfa_component", category = "xfa|win", short_name = "comp")
add_entries_for_builder(name = "win_xfa_gdi", category = "xfa|win", short_name = "gdi")
add_entries_for_builder(name = "win_xfa_rel", category = "xfa|win", short_name = "rel")
add_entries_for_builder(name = "win_xfa_skia", category = "skia|win", short_name = "xfa")
add_entries_for_builder(name = "win_xfa_skia_asan", category = "skia|win", short_name = "asan")
add_entries_for_builder(name = "win_xfa_skia_component", category = "skia|win", short_name = "comp")
add_entries_for_builder(name = "win_xfa_skia_gdi", category = "skia|win", short_name = "gdi")
add_entries_for_builder(name = "win_xfa_skia_rust", category = "skia|win", short_name = "rust", skip_ci_builder = True)

# Console Views
luci.console_view(
    name = "main",
    header = _CONSOLE_HEADER,
    title = "PDFium Main Console",
    repo = "https://pdfium.googlesource.com/pdfium",
    include_experimental_builds = True,
)

# List views
luci.list_view(
    name = "try",
    title = "PDFium Try Builders",
)

luci.list_view_entry(
    list_view = "try",
    builder = "try/pdfium_presubmit",
)

# CQ
luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_burst_delay = 480 * time.second,
    submit_max_burst = 4,
)

luci.cq_group(
    name = "pdfium",
    watch = cq.refset(
        "https://pdfium.googlesource.com/pdfium",
        refs = [r"refs/heads/main", r"refs/heads/chromium/.+"],
    ),
    tree_status_host = "pdfium-status.appspot.com",
    acls = [
        acl.entry(
            acl.CQ_COMMITTER,
            groups = "project-pdfium-committers",
        ),
        acl.entry(
            acl.CQ_DRY_RUNNER,
            groups = "project-pdfium-tryjob-access",
        ),
    ],
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 1,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    user_limit_default = cq.user_limit(
        name = "default-limit",
        run = cq.run_limits(max_active = 4),
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "pdfium:try/pdfium_presubmit",
            disable_reuse = True,
        ),
    ],
)

# Scheduler
luci.gitiles_poller(
    name = "pdfium-gitiles-trigger",
    bucket = "ci",
    repo = "https://pdfium.googlesource.com/pdfium",
    refs = [
        "refs/heads/main",
    ],
)
