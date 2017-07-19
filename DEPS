use_relative_paths = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'pdfium_git': 'https://pdfium.googlesource.com',

  'android_ndk_revision': '26d93ec07f3ce2ec2cdfeae1b21ee6f12ff868d8',
  'build_revision': 'f671d3baeb64d9dba628ad582e867cf1aebc0207',
  'buildtools_revision': 'e6b510a9daf822bbe9f922c200c58150803d2fd8',
  'catapult_revision': '86352b966b0245d6883e5f7df27687856978b6d7',
  'clang_revision': 'ae881aab392c247eca831e079d1d45e1c200cce7',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'freetype_revision': '38bdf22bfe68432aebdd33c198a0bd11b4ebb96f',
  'gen_library_loader_revision': '916d4acd8b2cde67a390737dfba90b3c37de23a1',
  'gmock_revision': '29763965ab52f24565299976b936d1265cb6a271',
  'gtest_revision': '8245545b6dc9c4703e6496d1efd19e975ad2b038',
  'icu_revision': '73e24736676b4b438270fda44e5b2c83b49fdd80',
  'instrumented_lib_revision': '45f5814b1543e41ea0be54c771e3840ea52cca4a',
  'jinja2_revision': 'd34383206fa42d52faa10bb9931d6d538f3a57e0',
  'jpeg_turbo_revision': '7260e4d8b8e1e40b17f03fafdf1cd83296900f76',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'pdfium_tests_revision': 'ab561f0075518d699636b884b0db46293cd76261',
  'skia_revision': 'bb581ce30f55360fd3a12e7f5aa1fe324b16d085',
  'tools_memory_revision': '427f10475e1a8d72424c29d00bf689122b738e5d',
  'trace_event_revision': '06294c8a4a6f744ef284cd63cfe54dbf61eea290',
  'v8_revision': '539f7ab310f8865090c494b0251c45e7c52ae5f5',
  'yasm_binary_revision': '52f9b3f4b0aa06da24ef8b123058bb61ee468881',
  'yasm_source_revision': '7da28c6c7c6a1387217352ce02b31754deb54d2a',
  'zlib_revision': 'eed2732a73838fd581cb25819ab1c4f478d6b38f',
}

deps = {
  "base/trace_event/common":
    Var('chromium_git') + "/chromium/src/base/trace_event/common.git@" +
        Var('trace_event_revision'),

  "build":
    Var('chromium_git') + "/chromium/src/build.git@" + Var('build_revision'),

  "buildtools":
    Var('chromium_git') + "/chromium/buildtools.git@" +
        Var('buildtools_revision'),

  "testing/corpus":
    Var('pdfium_git') + "/pdfium_tests@" + Var('pdfium_tests_revision'),

  "testing/gmock":
    Var('chromium_git') + "/external/googlemock.git@" + Var('gmock_revision'),

  "testing/gtest":
    Var('chromium_git') + "/external/googletest.git@" + Var('gtest_revision'),

  "third_party/freetype/src":
    Var('chromium_git') + '/chromium/src/third_party/freetype2.git@' +
        Var('freetype_revision'),

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

  "tools/generate_library_loader":
    Var('chromium_git') + "/chromium/src/tools/generate_library_loader@" +
        Var('gen_library_loader_revision'),

  # TODO(GYP): Remove this when no tools rely on GYP anymore.
  "tools/gyp":
    Var('chromium_git') + '/external/gyp.git@' +
        'eb296f67da078ec01f5e3a9ea9cdc6d26d680161',

  "tools/memory":
    Var('chromium_git') + "/chromium/src/tools/memory@" +
        Var('tools_memory_revision'),

  "v8":
    Var('chromium_git') + "/v8/v8.git@" + Var('v8_revision'),
}

deps_os = {
  "android": {
    "third_party/android_ndk":
      Var('chromium_git') + "/android_ndk.git@" + Var('android_ndk_revision'),
    "third_party/catapult":
      Var('chromium_git') +
          "/external/github.com/catapult-project/catapult.git@" +
          Var('catapult_revision'),
  },
  "win": {
    'third_party/yasm/binaries':
      Var('chromium_git') + '/chromium/deps/yasm/binaries.git' + '@' +
          Var('yasm_binary_revision'),

    "v8/third_party/cygwin":
      Var('chromium_git') + "/chromium/deps/cygwin@" + Var('cygwin_revision'),
  },
}

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'buildtools',
]

include_rules = [
  # Basic stuff that everyone can use.
  # Note: public is not here because core cannot depend on public.
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
    'name': 'gn_win',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'pdfium/buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'pdfium/buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'pdfium/buildtools/linux64/gn.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
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
    # Pull clang
    'name': 'clang',
    'pattern': '.',
    'action': ['python',
               'pdfium/tools/clang/scripts/update.py'
    ],
  },
  {
    # Downloads the current stable linux sysroot to build/linux/ if needed.
    # This sysroot updates at about the same rate that the chrome build deps
    # change.
    'name': 'sysroot',
    'pattern': '.',
    'action': ['python', 'pdfium/build/linux/sysroot_scripts/install-sysroot.py',
               '--running-as-hook'],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'action': ['python', 'pdfium/build/vs_toolchain.py', 'update'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'action': ['python', 'pdfium/build/mac_toolchain.py'],
  },
  {
    # Pull sanitizer-instrumented third-party libraries if requested via
    # GYP_DEFINES.
    'name': 'instrumented_libraries',
    'pattern': '\\.sha1',
    'action': ['python', 'pdfium/third_party/instrumented_libraries/scripts/download_binaries.py'],
  },

]
