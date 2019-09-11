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

  'android_ndk_revision': '099a70aef125058228f92f3ae6944904b477f97f',
  'binutils_revision': '01aa7745b0bab64ae22600f09fd6483c60f22ebf',
  'build_revision': 'b9e0d9224b62f89c03b617dbfd5a1fee8b5c83fc',
  'buildtools_revision': 'fce87d1a3208b830194ee24b16aca7afa7916577',
  'catapult_revision': '884c81e1703f23320657fab45aaeca226f64de25',
  'clang_revision': '8215b086137d5de08cd573955799a24cd40b4f1f',
  'code_coverage_revision': 'b53d904eb74afb18f4ddc27db4f75552b1237514',
  'depot_tools_revision': 'e5641be5fe309f40aad850d4d1e1ca607768572c',
  'freetype_revision': '3de1b8d0b0983cf49a187a4227e7950395a3b08f',
  'gtest_revision': '3a45039862471cc2785e92bd19bd146c70344986',
  'icu_revision': '53f6b233a41ec982d8445996247093f7aaf41639',
  'instrumented_lib_revision': 'b1c3ca20848c117eb935b02c25d441f03e6fbc5e',
  'jinja2_revision': '45571de473282bd1d8b63a8dfcb1fd268d0635d2',
  'jpeg_turbo_revision': 'baa5dc24258bf9af873b9105e8988c558c425a17',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'pdfium_tests_revision': '02dd653ec62649b6f1aa4e4526071cc32d903f54',
  'skia_revision': '8590026dbf0d22291c8c42ed0eddb73d66da446d',
  'tools_memory_revision': 'f7b00daf4df7f6c469f5fbc68d7f40f6bd15d6e6',
  'trace_event_revision': '5e4fce17a9d2439c44a7b57ceecef6df9287ec2f',
  'v8_revision': '33faa512cb633005dd8e13a91ca4fb12033ba376',
  'yasm_source_revision': '720b70524a4424b15fc57e82263568c8ba0496ad',
  'zlib_revision': '8c79b6c8db8bafc9cf5db82f2aad649a86947345',
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
