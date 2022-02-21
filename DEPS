use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'
gclient_gn_args = [
  'checkout_google_benchmark',
]

vars = {
  # By default, we should check out everything needed to run on the main
  # chromium waterfalls. This var can be also be set to 'small', in order
  # to skip things are not strictly needed to build chromium for development
  # purposes.
  'checkout_configuration': 'default',

  # By default, do not check out Google Benchmark. This only exists to satisfy
  # V8-enabled builds that require this variable. Running Google Benchmark is
  # not supported with PDFium.
  'checkout_google_benchmark': False,

  'checkout_instrumented_libraries': 'checkout_linux and checkout_configuration != "small"',

  'chromium_git': 'https://chromium.googlesource.com',
  'pdfium_git': 'https://pdfium.googlesource.com',
  'skia_git': 'https://skia.googlesource.com',

  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling abseil
  # and whatever else without interference from each other.
  'abseil_revision': '9ffdb583473a319b3920c752badde40f91ffd609',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling android_ndk
  # and whatever else without interference from each other.
  'android_ndk_revision': '9644104c8cf85bf1bdce5b1c0691e9778572c3f8',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling build
  # and whatever else without interference from each other.
  'build_revision': '893ac785aa454dcf84dcc26af1a410095c1d4fa2',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling buildtools
  # and whatever else without interference from each other.
  'buildtools_revision': '8ccda4f4eb749ece6207087268d978404b6d6bc7',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling catapult
  # and whatever else without interference from each other.
  'catapult_revision': 'b718bf93ff62e1ccbdd971ee5e5d91b9511826fe',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling clang format
  # and whatever else without interference from each other.
  'clang_format_revision': 'e435ad79c17b1888b34df88d6a30a094936e3836',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling clang
  # and whatever else without interference from each other.
  'clang_revision': 'a8ca8222301c8133135774b395955504c3313319',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling clang_dsymutil
  # and whatever else without interference from each other.
  'clang_dsymutil_revision': 'M56jPzDv1620Rnm__jTMYS62Zi8rxHVq7yw0qeBFEgkC',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling code_coverage
  # and whatever else without interference from each other.
  'code_coverage_revision': 'dd85397301ba8942b4fa22d798ef9aa9a58631d0',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling depot_tools
  # and whatever else without interference from each other.
  'depot_tools_revision': 'df01c5ad5be408cd157b879a1cce983f90762765',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling freetype
  # and whatever else without interference from each other.
  'freetype_revision': 'dd8676b2779a5c2130d6265a4e966515ff0509f6',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling GN CIPD package version
  # and whatever else without interference from each other.
  'gn_version': 'git_revision:0725d7827575b239594fbc8fd5192873a1d62f44',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling gtest
  # and whatever else without interference from each other.
  'gtest_revision': '25ad42aabeaba6227f37795cdd2752e128e83827',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling icu
  # and whatever else without interference from each other.
  'icu_revision': '2e0f2989441ec2f55abec30f48e89981dbac2c34',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling instrumented_lib
  # and whatever else without interference from each other.
  'instrumented_lib_revision': 'e09c4b66b6e87116eb190651421f1a6e2f3b9c52',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling jinja2
  # and whatever else without interference from each other.
  'jinja2_revision': 'ee69aa00ee8536f61db6a451f3858745cf587de6',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling jpeg_turbo
  # and whatever else without interference from each other.
  'jpeg_turbo_revision': '02959c3ee17abacfd1339ec22ea93301292ffd56',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling libc++
  # and whatever else without interference from each other.
  # If you change this, also update the libc++ revision in
  # //buildtools/deps_revisions.gni.
  'libcxx_revision': '79a2e924d96e2fc1e4b937c42efd08898fa472d7',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling libc++abi
  # and whatever else without interference from each other.
  'libcxxabi_revision': '5b0a8cf88eddfb448ca5903128d3328db76e4001',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling libunwind
  # and whatever else without interference from each other.
  'libunwind_revision': '659c7d91af58bfeada9f3ed5bf1a192059b68b86',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling markupsafe
  # and whatever else without interference from each other.
  'markupsafe_revision': '1b882ef6372b58bfd55a3285f37ed801be9137cd',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling nasm_source
  # and whatever else without interference from each other.
  'nasm_source_revision': '9215e8e1d0fe474ffd3e16c1a07a0f97089e6224',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling pdfium_tests
  # and whatever else without interference from each other.
  'pdfium_tests_revision': 'ad661eea10c72d8689374cc0381e12a6d91291c7',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling skia
  # and whatever else without interference from each other.
  'skia_revision': '8b618c3e16870cf92ad3c153f888c7ceb0cdd40c',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling test_fonts
  # and whatever else without interference from each other.
  'test_fonts_revision': '7f51783942943e965cd56facf786544ccfc07713',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling tools_memory
  # and whatever else without interference from each other.
  'tools_memory_revision': '5b5485d9df586b17d0a25d668989b2fb0174f334',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling trace_event
  # and whatever else without interference from each other.
  'trace_event_revision': 'd115b033c4e53666b535cbd1985ffe60badad082',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling v8
  # and whatever else without interference from each other.
  'v8_revision': 'ef9b5c3a6d0ca76cb2a034abbe40da7caa33c84a',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling zlib
  # and whatever else without interference from each other.
  'zlib_revision': 'fc5cfd78a357d5bb7735a58f383634faaafe706a',
}

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, and the new host is not in Chromium's DEPS
# file's allowed_hosts list, contact Chrome infrastructure team.
allowed_hosts = [
  'chromium.googlesource.com',
  'pdfium.googlesource.com',
  'skia.googlesource.com',
]

deps = {
  'base/trace_event/common':
    Var('chromium_git') + '/chromium/src/base/trace_event/common.git@' +
        Var('trace_event_revision'),

  'build':
    Var('chromium_git') + '/chromium/src/build.git@' + Var('build_revision'),

  'buildtools':
    Var('chromium_git') + '/chromium/src/buildtools.git@' +
        Var('buildtools_revision'),

  'buildtools/clang_format/script':
    Var('chromium_git') +
        '/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@' +
        Var('clang_format_revision'),

  'buildtools/linux64': {
    'packages': [
      {
        'package': 'gn/gn/linux-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux"',
  },

  'buildtools/mac': {
    'packages': [
      {
        'package': 'gn/gn/mac-${{arch}}',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac"',
  },

  'buildtools/third_party/libc++/trunk':
    Var('chromium_git') +
        '/external/github.com/llvm/llvm-project/libcxx.git@' +
        Var('libcxx_revision'),

  'buildtools/third_party/libc++abi/trunk':
    Var('chromium_git') +
        '/external/github.com/llvm/llvm-project/libcxxabi.git@' +
        Var('libcxxabi_revision'),

  'buildtools/third_party/libunwind/trunk':
    Var('chromium_git') +
        '/external/github.com/llvm/llvm-project/libunwind.git@' +
        Var('libunwind_revision'),

  'buildtools/win': {
    'packages': [
      {
        'package': 'gn/gn/windows-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win"',
  },

  'testing/corpus':
    Var('pdfium_git') + '/pdfium_tests@' + Var('pdfium_tests_revision'),

  'third_party/abseil-cpp':
    Var('chromium_git') + '/chromium/src/third_party/abseil-cpp.git@' +
        Var('abseil_revision'),

  'third_party/android_ndk': {
    'url': Var('chromium_git') + '/android_ndk.git@' +
        Var('android_ndk_revision'),
    'condition': 'checkout_android',
  },

  'third_party/catapult': {
    'url': Var('chromium_git') + '/catapult.git@' + Var('catapult_revision'),
    'condition': 'checkout_android',
  },

  'third_party/depot_tools':
    Var('chromium_git') + '/chromium/tools/depot_tools.git@' +
        Var('depot_tools_revision'),

  'third_party/freetype/src':
    Var('chromium_git') + '/chromium/src/third_party/freetype2.git@' +
        Var('freetype_revision'),

  'third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git@' +
        Var('gtest_revision'),

  'third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git@' + Var('icu_revision'),

  'third_party/instrumented_libraries':
    Var('chromium_git') +
        '/chromium/src/third_party/instrumented_libraries.git@' +
        Var('instrumented_lib_revision'),

  'third_party/jinja2':
    Var('chromium_git') + '/chromium/src/third_party/jinja2.git@' +
        Var('jinja2_revision'),

  'third_party/libjpeg_turbo':
    Var('chromium_git') + '/chromium/deps/libjpeg_turbo.git@' +
        Var('jpeg_turbo_revision'),

  'third_party/markupsafe':
    Var('chromium_git') + '/chromium/src/third_party/markupsafe.git@' +
        Var('markupsafe_revision'),

  'third_party/nasm':
    Var('chromium_git') + '/chromium/deps/nasm.git@' +
        Var('nasm_source_revision'),

  'third_party/skia':
    Var('skia_git') + '/skia.git@' + Var('skia_revision'),

  'third_party/test_fonts':
    Var('chromium_git') + '/chromium/src/third_party/test_fonts.git@' +
        Var('test_fonts_revision'),

  'third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib.git@' +
        Var('zlib_revision'),

  'tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang@' + Var('clang_revision'),

  'tools/clang/dsymutil': {
    'packages': [
      {
        'package': 'chromium/llvm-build-tools/dsymutil',
        'version': Var('clang_dsymutil_revision'),
      }
    ],
    'condition': 'checkout_mac or checkout_ios',
    'dep_type': 'cipd',
  },

  'tools/code_coverage':
    Var('chromium_git') + '/chromium/src/tools/code_coverage.git@' +
        Var('code_coverage_revision'),

  'tools/memory':
    Var('chromium_git') + '/chromium/src/tools/memory@' +
        Var('tools_memory_revision'),

  # TODO(crbug.com/pdfium/1650): Set up autorollers for goldctl.
  'tools/skia_goldctl/linux': {
    'packages': [
      {
        'package': 'skia/tools/goldctl/linux-amd64',
        'version': 'git_revision:11b8d9e1b976c7ef4dd60521c87d00a8970f889b',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_linux',
  },

  # TODO(crbug.com/pdfium/1650, crbug.com/1280002): Remove this entry once there
  # is an autoroller has been updated to roll the mac_amd64 version.
  'tools/skia_goldctl/mac': {
    'packages': [
      {
        'package': 'skia/tools/goldctl/mac-amd64',
        'version': 'git_revision:11b8d9e1b976c7ef4dd60521c87d00a8970f889b',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_mac',
  },

  'tools/skia_goldctl/mac_amd64': {
    'packages': [
      {
        'package': 'skia/tools/goldctl/mac-amd64',
        'version': 'git_revision:11b8d9e1b976c7ef4dd60521c87d00a8970f889b',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_mac',
  },

  'tools/skia_goldctl/mac_arm64': {
    'packages': [
      {
        'package': 'skia/tools/goldctl/mac-arm64',
        'version': 'git_revision:11b8d9e1b976c7ef4dd60521c87d00a8970f889b',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_mac',
  },

  'tools/skia_goldctl/win': {
    'packages': [
      {
        'package': 'skia/tools/goldctl/windows-amd64',
        'version': 'git_revision:11b8d9e1b976c7ef4dd60521c87d00a8970f889b',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_win',
  },

  'v8':
    Var('chromium_git') + '/v8/v8.git@' + Var('v8_revision'),
}

recursedeps = []

include_rules = [
  # Basic stuff that everyone can use.
  # Note: public is not here because core cannot depend on public.
  '+build/build_config.h',
  '+constants',
  '+testing',
  '+third_party/base',

  # Abseil features must be allowlisted explicitly for now. See Chromium's
  # //styleguide/c++/c++11.html. Allowed features' headers will be listed
  # explicitly here.
  '-absl',
  '-third_party/abseil-cpp',
  '+third_party/abseil-cpp/absl/types/optional.h',
  '+third_party/abseil-cpp/absl/types/variant.h',
]

specific_include_rules = {
  # Allow embedder tests to use public APIs.
  '(.*embeddertest\.cpp)': [
    '+public',
  ]
}

hooks = [
  {
    # Ensure that the DEPS'd "depot_tools" has its self-update capability
    # disabled.
    'name': 'disable_depot_tools_selfupdate',
    'pattern': '.',
    'action': [ 'python3',
                'third_party/depot_tools/update_depot_tools_toggle.py',
                '--disable',
    ],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "linux"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python3', 'build/mac_toolchain.py'],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python3',
               'tools/clang/scripts/update.py'
    ],
  },
  {
    'name': 'sysroot_arm',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm'],
  },
  {
    'name': 'sysroot_arm64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm64',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm64'],
  },
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_mips',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_mips',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=mips'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_x64',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    'name': 'test_fonts',
    'pattern': '.',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-fonts',
                '-s', 'third_party/test_fonts/test_fonts.tar.gz.sha1',
    ],
  },
  {
    'name': 'msan_chained_origins',
    'pattern': '.',
    'condition': 'checkout_instrumented_libraries',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-instrumented-libraries',
                '-s', 'third_party/instrumented_libraries/binaries/msan-chained-origins.tgz.sha1',
              ],
  },
  {
    'name': 'msan_no_origins',
    'pattern': '.',
    'condition': 'checkout_instrumented_libraries',
    'action': [ 'python3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-instrumented-libraries',
                '-s', 'third_party/instrumented_libraries/binaries/msan-no-origins.tgz.sha1',
              ],
  },
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },
]
