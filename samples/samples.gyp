# Copyright 2014 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'pdf_enable_v8%': 1,
  },
  'target_defaults': {
    'type': 'executable',
    'dependencies': [
      '../pdfium.gyp:pdfium',
    ],
    'include_dirs': [
      '<(DEPTH)',
    ],
    'defines' : [
      'PNG_PREFIX',
      'PNGPREFIX_H',
      'PNG_USE_READ_MACROS',
    ],
    'conditions': [
      ['pdf_enable_v8==1', {
        'defines': [
          'PDF_ENABLE_V8',
        ],
        'include_dirs': [
          '<(DEPTH)/v8',
          '<(DEPTH)/v8/include',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'pdfium_test',
      'type': 'executable',
      'dependencies': [
        '../pdfium.gyp:pdfium',
        '../pdfium.gyp:test_support',
      ],
      'sources': [
        'pdfium_test.cc',
        'image_diff_png.cc',
      ],
      'conditions': [
        ['pdf_enable_v8==1', {
          'dependencies': [
            '<(DEPTH)/v8/tools/gyp/v8.gyp:v8_libplatform',
          ],
        }],
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
