use_relative_paths = True

deps = {
  "build/gyp":
    "https://chromium.googlesource.com/external/gyp",

  "buildtools":
    "https://chromium.googlesource.com/chromium/buildtools.git@46ce8cb60364e9e0b21a81136c7debdddfd063a8",

  "testing/corpus":
     "https://pdfium.googlesource.com/pdfium_tests@2ddcfbd23aa7ef0a7424ef24a3fac7acdfb39ee5",

  "testing/gmock":
     "https://chromium.googlesource.com/external/googlemock.git@29763965ab52f24565299976b936d1265cb6a271",

  "testing/gtest":
     "https://chromium.googlesource.com/external/googletest.git@8245545b6dc9c4703e6496d1efd19e975ad2b038",

  "v8":
    "https://chromium.googlesource.com/v8/v8.git",

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
  '+public',
  '+testing',
  '+third_party/base',
  '+v8',
]

hooks = [
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    'name': 'gyp',
    'pattern': '.',
    'action': ['python', 'build/gyp_pdfium'],
  },
]