# Copyright 2014 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'target_defaults': {
    'type': 'executable',
    'dependencies': [
      '../pdfium.gyp:pdfium',
      '<(DEPTH)/v8/tools/gyp/v8.gyp:v8_libplatform',
    ],
    'include_dirs': [
      '<(DEPTH)',
      '<(DEPTH)/v8',
      '<(DEPTH)/v8/include',
    ],
  },
  'targets': [
    {
      'target_name': 'pdfium_test',
      'sources': [
        'pdfium_test.cc',
        'image_diff_png.cc',
      ],
    },
    {
      'target_name': 'pdfium_diff',
      'type': 'executable',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '../pdfium.gyp:fxcodec',
        '../third_party/third_party.gyp:pdfium_base',
      ],
      'include_dirs': [
        '../../',
      ],
      'sources': [
        'image_diff.cc',
        'image_diff_png.h',
        'image_diff_png.cc',
      ],
    },
  ],
}
