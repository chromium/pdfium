use_relative_paths = True

vars = {
  # By default, we should check out everything needed to run on the main
  # chromium waterfalls. This var can be also be set to "small", in order
  # to skip things are not strictly needed to build chromium for development
  # purposes.
  'checkout_configuration': 'default',

  'checkout_instrumented_libraries': 'checkout_linux and checkout_configuration != "small"',

  'chromium_git': 'https://chromium.googlesource.com',
  'pdfium_git': 'https://pdfium.googlesource.com',

  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling android_ndk
  # and whatever else without interference from each other.
  'android_ndk_revision': '27c0a8d090c666a50e40fceb4ee5b40b1a2d3f87',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling binutils
  # and whatever else without interference from each other.
  'binutils_revision': 'ffd1fdb902d5ce2d28b6a18bebed2c42c863e22e',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling build
  # and whatever else without interference from each other.
  'build_revision': '36cc563890d733d7aefa52bd27fcd49bcb37b030',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling buildtools
  # and whatever else without interference from each other.
  'buildtools_revision': '4164a305626786b1912d467003acf4c4995bec7d',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling catapult
  # and whatever else without interference from each other.
  'catapult_revision': 'df670f0bed9e3f58ab3061f54842e4606fd9ba29',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling clang
  # and whatever else without interference from each other.
  'clang_revision': '6e39ba4a2cedc07e77e63fe39788370370fd98d9',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling clang_dsymutil
  # and whatever else without interference from each other.
  'clang_dsymutil_revision': 'M56jPzDv1620Rnm__jTMYS62Zi8rxHVq7yw0qeBFEgkC',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling code_coverage
  # and whatever else without interference from each other.
  'code_coverage_revision': 'a8f20a1dacc200010cd99f4eabdb0da7d94be112',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling depot_tools
  # and whatever else without interference from each other.
  'depot_tools_revision': '464e9ff4f3682426b0cb3b68ee38e7be6fa4a2be',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling freetype
  # and whatever else without interference from each other.
  'freetype_revision': '6a431038c9113d906d66836cd7d216a5c630be7c',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling gtest
  # and whatever else without interference from each other.
  'gtest_revision': '10b1902d893ea8cc43c69541d70868f91af3646b',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling icu
  # and whatever else without interference from each other.
  'icu_revision': 'd7aff76cf6bb0fbef3afa6c07718f78a80a70f8f',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling instrumented_lib
  # and whatever else without interference from each other.
  'instrumented_lib_revision': 'bb3f1802c237dd19105dd0f7919f99e536a39d10',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling jinja2
  # and whatever else without interference from each other.
  'jinja2_revision': 'b41863e42637544c2941b574c7877d3e1f663e25',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling jpeg_turbo
  # and whatever else without interference from each other.
  'jpeg_turbo_revision': 'ce0e57e8e636f5132fe6f0590a4dba91f92fd935',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling markupsafe
  # and whatever else without interference from each other.
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling pdfium_tests
  # and whatever else without interference from each other.
  'pdfium_tests_revision': 'b4e9a126d289b49002d0f0acac71d1fdc26953b3',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling skia
  # and whatever else without interference from each other.
  'skia_revision': '1171d314efc7e131b039c94126d26575cfb9e70c',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling tools_memory
  # and whatever else without interference from each other.
  'tools_memory_revision': '89552acb6e60f528fe3c98eac7b445d4c34183ee',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling trace_event
  # and whatever else without interference from each other.
  'trace_event_revision': 'dab187b372fc17e51f5b9fad8201813d0aed5129',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling v8
  # and whatever else without interference from each other.
  'v8_revision': '633234050d9781560f814e4914c31954459aecd4',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling yasm_source
  # and whatever else without interference from each other.
  'yasm_source_revision': '720b70524a4424b15fc57e82263568c8ba0496ad',
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling zlib
  # and whatever else without interference from each other.
  'zlib_revision': '156be8c52f80cde343088b4a69a80579101b6e67',
}

deps = {
  "base/trace_event/common":
    Var('chromium_git') + "/chromium/src/base/trace_event/common.git@" +
        Var('trace_event_revision'),

  "build":
    Var('chromium_git') + "/chromium/src/build.git@" + Var('build_revision'),

  "buildtools":
    Var('chromium_git') + "/chromium/src/buildtools.git@" +
        Var('buildtools_revision'),

  "testing/corpus":
    Var('pdfium_git') + "/pdfium_tests@" + Var('pdfium_tests_revision'),

  "third_party/android_ndk": {
    'url': Var('chromium_git') + "/android_ndk.git@" + Var('android_ndk_revision'),
    'condition': 'checkout_android',
  },

  "third_party/binutils":
    Var('chromium_git') + "/chromium/src/third_party/binutils.git@" +
        Var('binutils_revision'),

  "third_party/catapult": {
    'url': Var('chromium_git') + '/catapult.git' + '@' + Var('catapult_revision'),
    'condition': 'checkout_android',
  },

  'third_party/depot_tools':
    Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' +
        Var('depot_tools_revision'),

  "third_party/freetype/src":
    Var('chromium_git') + '/chromium/src/third_party/freetype2.git@' +
        Var('freetype_revision'),

  "third_party/googletest/src":
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' +
        Var('gtest_revision'),

  "third_party/icu":
    Var('chromium_git') + "/chromium/deps/icu.git@" + Var('icu_revision'),

  "third_party/instrumented_libraries":
    Var('chromium_git') +
        "/chromium/src/third_party/instrumented_libraries.git@" +
        Var('instrumented_lib_revision'),

  "third_party/jinja2":
    Var('chromium_git') + "/chromium/src/third_party/jinja2.git@" +
        Var('jinja2_revision'),

  "third_party/markupsafe":
    Var('chromium_git') + "/chromium/src/third_party/markupsafe.git@" +
        Var('markupsafe_revision'),

  "third_party/libjpeg_turbo":
    Var('chromium_git') + "/chromium/deps/libjpeg_turbo.git@" +
        Var('jpeg_turbo_revision'),

  "third_party/skia":
    Var('chromium_git') + '/skia.git@' +  Var('skia_revision'),

  "third_party/zlib":
    Var('chromium_git') + "/chromium/src/third_party/zlib.git@" +
        Var('zlib_revision'),

  'third_party/yasm/source/patched-yasm':
    Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git@' +
        Var('yasm_source_revision'),

  "tools/clang":
    Var('chromium_git') + "/chromium/src/tools/clang@" +  Var('clang_revision'),

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

  "tools/code_coverage":
    Var('chromium_git') + "/chromium/src/tools/code_coverage.git@" +
        Var('code_coverage_revision'),

  "tools/memory":
    Var('chromium_git') + "/chromium/src/tools/memory@" +
        Var('tools_memory_revision'),

  "v8":
    Var('chromium_git') + "/v8/v8.git@" + Var('v8_revision'),
}

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'buildtools',
]

include_rules = [
  # Basic stuff that everyone can use.
  # Note: public is not here because core cannot depend on public.
  '+build/build_config.h',
  '+constants',
  '+testing',
  '+third_party/base',
]

specific_include_rules = {
  # Allow embedder tests to use public APIs.
  "(.*embeddertest\.cpp)": [
      "+public",
  ]
}

hooks = [
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "linux"',
    'action': [ 'python',
                'pdfium/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'pdfium/build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python', 'pdfium/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'action': ['python', 'pdfium/build/mac_toolchain.py'],
  },
  {
    # Pull clang-format binaries using checked-in hashes.
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'pdfium/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'pdfium/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'pdfium/buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python',
               'pdfium/tools/clang/scripts/update.py'
    ],
  },
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'condition': 'host_os == "linux"',
    'action': [
        'python',
        'pdfium/third_party/binutils/download.py',
    ],
  },
  {
    'name': 'sysroot_arm',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm'],
  },
  {
    'name': 'sysroot_arm64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_arm64',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=arm64'],
  },
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_mips',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_mips',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=mips'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and checkout_x64',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    'name': 'msan_chained_origins',
    'pattern': '.',
    'condition': 'checkout_instrumented_libraries',
    'action': [ 'python',
                'pdfium/third_party/depot_tools/download_from_google_storage.py',
                "--no_resume",
                "--no_auth",
                "--bucket", "chromium-instrumented-libraries",
                "-s", "pdfium/third_party/instrumented_libraries/binaries/msan-chained-origins-trusty.tgz.sha1",
              ],
  },
  {
    'name': 'msan_no_origins',
    'pattern': '.',
    'condition': 'checkout_instrumented_libraries',
    'action': [ 'python',
                'pdfium/third_party/depot_tools/download_from_google_storage.py',
                "--no_resume",
                "--no_auth",
                "--bucket", "chromium-instrumented-libraries",
                "-s", "pdfium/third_party/instrumented_libraries/binaries/msan-no-origins-trusty.tgz.sha1",
              ],
  },
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'pdfium/build/util/lastchange.py',
               '-o', 'pdfium/build/util/LASTCHANGE'],
  },
]
