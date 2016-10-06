use_relative_paths = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'pdfium_git': 'https://pdfium.googlesource.com',

  'android_ndk_revision': '5022f40f12953c02b2614c5f7beb981ec5d0e833',
  'build_revision': 'b73bafdd0ff0efc8835de6e646b47eec0a2a38a4',
  'buildtools_revision': '5fd66957f08bb752dca714a591c84587c9d70762',
  'catapult_revision': '327256cb1c387e62a2c7e6960e2398867665fb51',
  'clang_revision': '797371be0a75c9892d6acb486d0430915ea538a6',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'gen_library_loader_revision': '916d4acd8b2cde67a390737dfba90b3c37de23a1',
  'gmock_revision': '29763965ab52f24565299976b936d1265cb6a271',
  'gtest_revision': '8245545b6dc9c4703e6496d1efd19e975ad2b038',
  'icu_revision': '2341038bf72869a5683a893a2b319a48ffec7f62',
  'pdfium_tests_revision': '06411790a46e6497e5293f921ea9aa9fb2615e41',
  'skia_revision': 'b55ebf071dce33794f5518dcdc3a9af006285796',
  'tools_memory_revision': '427f10475e1a8d72424c29d00bf689122b738e5d',
  'trace_event_revision': '6232c13e4edb36c84c61653fdae5a4afb5af9745',
  'v8_revision': 'a964163e6ac4f1124b22ea7e46b8afe26dec02e0',
}

deps = {
  "base/trace_event/common":
    Var('chromium_git') + "/chromium/src/base/trace_event/common.git@" +
        Var('trace_event_revision'),

  "build":
    Var('chromium_git') + "/chromium/src/build.git@" + Var('build_revision'),

  "buildtools":
    Var('chromium_git') + "/chromium/buildtools.git@" + Var('buildtools_revision'),

  "testing/corpus":
    Var('pdfium_git') + "/pdfium_tests@" + Var('pdfium_tests_revision'),

  "testing/gmock":
    Var('chromium_git') + "/external/googlemock.git@" + Var('gmock_revision'),

  "testing/gtest":
    Var('chromium_git') + "/external/googletest.git@" + Var('gtest_revision'),

  "third_party/icu":
    Var('chromium_git') + "/chromium/deps/icu.git@" + Var('icu_revision'),

  "third_party/skia":
    Var('chromium_git') + '/skia.git' + '@' +  Var('skia_revision'),

  "tools/clang":
    Var('chromium_git') + "/chromium/src/tools/clang@" +  Var('clang_revision'),

  "tools/generate_library_loader":
    Var('chromium_git') + "/chromium/src/tools/generate_library_loader@" +
        Var('gen_library_loader_revision'),

  # TODO(GYP): Remove this when no tools rely on GYP anymore.
  "tools/gyp":
    Var('chromium_git') + '/external/gyp.git' + '@' + 'c61b0b35c8396bfd59efc6cfc11401d912b0f510',

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
      Var('chromium_git') + "/external/github.com/catapult-project/catapult.git@" + Var('catapult_revision'),
  },
  "win": {
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
  # Pull GN binaries. This needs to be before running GYP below.
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
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'pattern': '.',
    'action': ['python', 'pdfium/build/vs_toolchain.py', 'update'],
  },
]
