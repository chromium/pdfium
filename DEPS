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

  'android_ndk_revision': '4e2cea441bfd43f0863d14f57b1e1844260b9884',
  'binutils_revision': '92bfa0a5dcee9dc01173e39e5bff226b09af0254',
  'build_revision': '578c661466b793e965955deee4cdb79f5fd3259b',
  'buildtools_revision': 'c79f3482c8152172a31e5c17823a27835a511dac',
  'catapult_revision': 'cca9447f62824814d1ae1dd6c6268a66745d9fab',
  'clang_revision': '4e32649d7a532a0aa550baedadc3309970fb7642',
  'code_coverage_revision': '90f3a1ede923558ce02fa6e92ed9689ed52b199f',
  'depot_tools_revision': 'ccd2b4da9a7e6ddf126c0b4437db75201836154c',
  'freetype_revision': '31757f969fba60d75404f31e8f1168bef5011770',
  'gtest_revision': 'f71fb4f9a912ec945401cc49a287a759b6131026',
  'icu_revision': '64e5d7d43a1ff205e3787ab6150bbc1a1837332b',
  'instrumented_lib_revision': 'a959e4f0cb643003f2d75d179cede449979e3e77',
  'jinja2_revision': '45571de473282bd1d8b63a8dfcb1fd268d0635d2',
  'jpeg_turbo_revision': '2de84a43e683c2c3c8ff4922da16b9053f024144',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'pdfium_tests_revision': '5a68e87859476eb75d3e068bd406c4921a6be8e2',
  'skia_revision': '0c229970754242a038c91328873197c62d60770d',
  'tools_memory_revision': 'f7b00daf4df7f6c469f5fbc68d7f40f6bd15d6e6',
  'trace_event_revision': '936ba8a963284a6b3737cf2f0474a7131073abee',
  'v8_revision': 'c4c480a413d23f62301e839970be1378c2c7f1f0',
  'yasm_source_revision': '720b70524a4424b15fc57e82263568c8ba0496ad',
  'zlib_revision': '8f2b9d41fbd341ab8ceaa726fb544bd72afd9b82',

  # GN CIPD package version.
  'gn_version': 'git_revision:0790d3043387c762a6bacb1ae0a9ebe883188ab2',
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

  'pdfium/buildtools/linux64': {
    'packages': [
      {
        'package': 'gn/gn/linux-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux"',
  },

  'pdfium/buildtools/mac': {
    'packages': [
      {
        'package': 'gn/gn/mac-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac"',
  },

  'pdfium/buildtools/win': {
    'packages': [
      {
        'package': 'gn/gn/windows-amd64',
        'version': Var('gn_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win"',
  },

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
