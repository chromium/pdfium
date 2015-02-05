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
      ],
    },
  ],
}
