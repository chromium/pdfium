use_relative_paths = True

deps = {
  "build/gyp":
    "https://chromium.googlesource.com/external/gyp",

  "buildtools":
    "https://chromium.googlesource.com/chromium/buildtools.git@c2f259809d5ede3275df5ea0842f0431990c4f98",

  "testing/corpus":
     "https://pdfium.googlesource.com/pdfium_tests@45f88c6914fcac26ad930bb0ebbfa468c21db0a5",

  "testing/gmock":
     "https://chromium.googlesource.com/external/googlemock.git@29763965ab52f24565299976b936d1265cb6a271",

  "testing/gtest":
     "https://chromium.googlesource.com/external/googletest.git@8245545b6dc9c4703e6496d1efd19e975ad2b038",

  "tools/clang":
    "https://chromium.googlesource.com/chromium/src/tools/clang",

  "v8":
    "https://chromium.googlesource.com/v8/v8.git@88b764d7f74008fe5f30d59e3881e374d538b72e",

  "v8/third_party/icu":
    "https://chromium.googlesource.com/chromium/deps/icu46",
}

deps_os = {
  "win": {
    "v8/third_party/cygwin":
      "https://chromium.googlesource.com/chromium/deps/cygwin@c89e446b273697fadf3a10ff1007a97c0b7de6df",
  },
}

include_rules = [
  # Basic stuff that everyone can use.
  # Note: public is not here because core cannot depend on public.
  '+testing',
  '+third_party/base',
]

hooks = [
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    'name': 'gyp',
    'pattern': '.',
    'action': ['python', 'pdfium/build/gyp_pdfium'],
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
    # Pull clang if needed or requested via GYP_DEFINES.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'pdfium/tools/clang/scripts/update.py', '--if-needed'],
  },
]
