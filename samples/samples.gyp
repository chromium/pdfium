# Copyright 2014 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'target_defaults': {
    'type': 'executable',
    'dependencies': [
      '../pdfium.gyp:pdfium',
    ],
    'include_dirs': ['<(DEPTH)'],
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
        '../third_party/third_party.gyp:safemath',
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
