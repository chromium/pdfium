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

# Dicts for OS-specific dimensions.
_LINUX_DIMENSIONS = {
    "cores": "8",
    "cpu": "x86-64",
    "os": "Ubuntu-18.04",
}
_MACOS_DIMENSIONS = {
    "cpu": "x86-64",
    "os": "Mac-11",
}
_WINDOWS_DIMENSIONS = {
    "cores": "8",
    "cpu": "x86-64",
    "os": "Windows-10",
}

# Dicts for goma properties.
_GOMA_ATS_ENABLED = {
    "enable_ats": True,
    "rpc_extra_params": "?prod",
    "server_host": "goma.chromium.org",
}
_GOMA_ATS_DISABLED = {
    "rpc_extra_params": "?prod",
    "server_host": "goma.chromium.org",
}

# Dicts for OS-specifc properties.
_ANDROID_PROPERTIES = {
    "$build/goma": _GOMA_ATS_ENABLED,
    "skip_test": True,
    "target_os": "android",
}
_LINUX_PROPERTIES = {
    "$build/goma": _GOMA_ATS_ENABLED,
}
_MACOS_PROPERTIES = {
    "$build/goma": _GOMA_ATS_DISABLED,
}
_WINDOWS_PROPERTIES = _LINUX_PROPERTIES

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
    if name.startswith("linux"):
        properties.update(_LINUX_PROPERTIES)
    elif name.startswith("mac"):
        properties.update(_MACOS_PROPERTIES)
    elif name.startswith("win"):
        properties.update(_WINDOWS_PROPERTIES)
    else:
        # Android
        properties.update(_ANDROID_PROPERTIES)

    if name.endswith("no_v8"):
        properties.update({"v8": False})

    if name.find("asan") != -1:
        properties.update({"memory_tool": "asan"})
        if not name.endswith("lsan"):
            properties.update({"rel": True})

    if name.startswith("win") and name.endswith("asan"):
        properties.update({"clang": True})

    if name.endswith("msan"):
        properties.update({"memory_tool": "msan", "rel": True})

    if name.endswith("ubsan"):
        properties.update({"memory_tool": "ubsan", "rel": True})

    if name.endswith("rel"):
        properties.update({"rel": True})

    if name.find("xfa") != -1:
        properties.update({"xfa": True})

    if name.find("msvc") != -1:
        properties.update({"msvc": True})
        properties["$build/goma"] = None

    if name.endswith("32"):
        properties.update({"target_cpu": "x86"})

    if name.find("component") != -1:
        properties.update({"component": True})

    if name.endswith("skia"):
        properties.update({"skia": True, "selected_tests_only": True})

    if name.endswith("skia_paths"):
        properties.update({"skia_paths": True, "selected_tests_only": True})

    return properties

def pdfium_internal_builder(name, bucket):
    """Creates a builder based on the builder name and the bucket name.

    Args:
      name: The builder name.
      bucket: The name of the bucket that the builder belongs to.

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
        notifies = ["pdfium main notifier"]
        properties.update({"builder_group": "client.pdfium"})
        service_account = "pdfium-ci-builder@chops-service-accounts.iam.gserviceaccount.com"
        triggered_by = ["pdfium-gitiles-trigger"]
    else:
        dimensions.update({"pool": "luci.flex.try"})
        properties.update({"builder_group": "tryserver.client.pdfium"})
        service_account = "pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com"

    # Set configs depending on the OS.
    if name.startswith("linux"):
        dimensions.update(_LINUX_DIMENSIONS)
    elif name.startswith("mac"):
        dimensions.update(_MACOS_DIMENSIONS)
        caches = [swarming.cache("osx_sdk", name = "osx_sdk")]
    elif name.startswith("win"):
        dimensions.update(_WINDOWS_DIMENSIONS)
    else:
        # Android
        dimensions.update(_LINUX_DIMENSIONS)

    # Update properties based on the builder name.
    properties.update(get_properties_by_name(name))

    return luci.builder(
        name = name,
        bucket = bucket,
        caches = caches,
        dimensions = dimensions,
        executable = "pdfium",
        notifies = notifies,
        properties = properties,
        service_account = service_account,
        triggered_by = triggered_by,
    )

def add_entries_for_builder(name, category = None, short_name = None, skip_ci_builder = False):
    """Creates builder, tryjob verifier and view entries for a given builder name.

    Args:
      name: The name for the builder to be created.
      category: The category name for the console view entry.
      short_name: The short name to be displayed in the console view.
      skip_ci_builder: Whether to skip creating a builder in the "ci" bucket.
          When set to True, it will also skip making the matching console view
          entry for that "ci" builder.
    """

    # Add builders in buckets.
    if not skip_ci_builder:
        pdfium_internal_builder(name, bucket = "ci")

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

    pdfium_internal_builder(name, bucket = "try")

    # Add the matching list view entry.
    luci.list_view_entry(
        list_view = "try",
        builder = "try/" + name,
    )

# End of helpers and constants. Below we actually generate the configs.

# Enable LUCI Realms support.
lucicfg.enable_experiment("crbug.com/1085650")
luci.builder.defaults.experiments.set({"luci.use_realms": 100})

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
        "tricium-prod.cfg",
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
    tricium = "tricium-prod.appspot.com",
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

luci.notifier(
    name = "pdfium main notifier",
    on_new_status = ["FAILURE"],
    notify_emails = [
        "awscreen@chromium.org",
        "dhoss@chromium.org",
        "kmoon@chromium.org",
        "nigi@chromium.org",
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
    use_bbagent=True,
    use_python3=True,
)

luci.recipe(
    name = "pdfium_analysis",
    cipd_package = _CIPD_PACKAGE,
    use_bbagent=True,
    use_python3=True,
)

luci.recipe(
    name = "presubmit",
    cipd_package = _CIPD_PACKAGE,
    use_bbagent=True,
    use_python3=True,
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
            # Allow Tricium prod to trigger analyzer tryjobs.
            users = [
                "tricium-prod@appspot.gserviceaccount.com",
            ],
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

# Builders
luci.builder(
    name = "pdfium_analysis",
    bucket = "try",
    executable = "pdfium_analysis",
    service_account = "pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    dimensions = {
        "cores": "8",
        "cpu": "x86-64",
        "os": "Ubuntu-18.04",
        "pool": "luci.flex.try",
    },
    properties = {
        "builder_group": "tryserver.client.pdfium",
        "$build/goma": _GOMA_ATS_ENABLED,
    },
)

luci.builder(
    name = "pdfium_presubmit",
    bucket = "try",
    executable = "presubmit",
    service_account = "pdfium-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    dimensions = {
        "cores": "8",
        "cpu": "x86-64",
        "os": "Ubuntu-18.04",
        "pool": "luci.flex.try",
    },
    properties = {
        "builder_group": "tryserver.client.pdfium",
        "repo_name": "pdfium",
        "$build/goma": _GOMA_ATS_ENABLED,
        "$depot_tools/presubmit": {
            "runhooks": True,
            "timeout_s": 480,
        },
    },
)

add_entries_for_builder(name = "android", category = "main|android")
add_entries_for_builder(name = "android_no_v8", category = "no v8", short_name = "android")
add_entries_for_builder(name = "linux", category = "main|linux")
add_entries_for_builder(name = "linux_asan_lsan", category = "main|linux", short_name = "asan")
add_entries_for_builder(name = "linux_msan", category = "main|linux", short_name = "msan")
add_entries_for_builder(name = "linux_no_v8", category = "no v8", short_name = "linux")
add_entries_for_builder(name = "linux_skia", category = "skia", short_name = "linux")
add_entries_for_builder(name = "linux_skia_paths", category = "skia_paths", short_name = "linux")
add_entries_for_builder(name = "linux_ubsan", category = "main|linux", short_name = "ubsan")
add_entries_for_builder(name = "linux_xfa", category = "xfa|linux")
add_entries_for_builder(name = "linux_xfa_asan_lsan", category = "xfa|linux", short_name = "asan")
add_entries_for_builder(name = "linux_xfa_component", category = "xfa|linux", short_name = "comp")
add_entries_for_builder(name = "linux_xfa_msan", category = "xfa|linux", short_name = "msan")
add_entries_for_builder(name = "linux_xfa_rel", category = "xfa|linux", short_name = "rel")
add_entries_for_builder(name = "linux_xfa_ubsan", category = "xfa|linux", short_name = "ubsan")
add_entries_for_builder(name = "mac", category = "main|mac")
add_entries_for_builder(name = "mac_asan", skip_ci_builder = True)
add_entries_for_builder(name = "mac_no_v8", category = "no v8", short_name = "mac")
add_entries_for_builder(name = "mac_skia", category = "skia", short_name = "mac")
add_entries_for_builder(name = "mac_skia_paths", category = "skia_paths", short_name = "mac")
add_entries_for_builder(name = "mac_xfa", category = "xfa|mac")
add_entries_for_builder(name = "mac_xfa_asan", skip_ci_builder = True)
add_entries_for_builder(name = "mac_xfa_component", category = "xfa|mac", short_name = "comp")
add_entries_for_builder(name = "mac_xfa_rel", category = "xfa|mac", short_name = "rel")
add_entries_for_builder(name = "win", category = "main|win")
add_entries_for_builder(name = "win_asan", category = "main|win", short_name = "asan")
add_entries_for_builder(name = "win_no_v8", category = "no v8", short_name = "win")
add_entries_for_builder(name = "win_skia", category = "skia", short_name = "win")
add_entries_for_builder(name = "win_skia_paths", category = "skia_paths", short_name = "win")
add_entries_for_builder(name = "win_xfa", category = "xfa|win")
add_entries_for_builder(name = "win_xfa_32", category = "xfa|win", short_name = "32")
add_entries_for_builder(name = "win_xfa_asan", category = "xfa|win", short_name = "asan")
add_entries_for_builder(name = "win_xfa_component", category = "xfa|win", short_name = "comp")
add_entries_for_builder(name = "win_xfa_msvc", category = "xfa|win|msvc", short_name = "64")
add_entries_for_builder(name = "win_xfa_msvc_32", category = "xfa|win|msvc", short_name = "32")
add_entries_for_builder(name = "win_xfa_rel", category = "xfa|win", short_name = "rel")

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
    builder = "try/pdfium_analysis",
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
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "pdfium:try/pdfium_analysis",
            owner_whitelist = ["project-pdfium-tryjob-access"],
            mode_allowlist = [cq.MODE_ANALYZER_RUN],
        ),
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
