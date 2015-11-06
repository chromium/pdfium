# Copyright 2014 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'pdf_enable_v8%': 1,
  },
  'target_defaults': {
    'defines' : [
      'PNG_PREFIX',
      'PNGPREFIX_H',
      'PNG_USE_READ_MACROS',
    ],
    'include_dirs': [
      # This is implicit in GN.
      '<(DEPTH)',
      '..',
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
        'fx_lpng',
        '../pdfium.gyp:pdfium',
        '../pdfium.gyp:test_support',
        # Regardless of whether the library ships against system freetype,
        # always link this binary against the bundled one for consistency
        # of results across platforms.
        '../third_party/third_party.gyp:fx_freetype',
      ],
      'sources': [
        'pdfium_test.cc',
        'image_diff_png.cc',
      ],
      'link_settings': {
        'libraries!': [
          '-lfreetype',
        ],
      },
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
        'fx_lpng',
        '../pdfium.gyp:pdfium',
        '../third_party/third_party.gyp:pdfium_base',
      ],
      'sources': [
        'image_diff.cc',
        'image_diff_png.h',
        'image_diff_png.cc',
      ],
    },
    {
      'target_name': 'fx_lpng',
      'type': 'static_library',
      'dependencies': [
        '../pdfium.gyp:fxcodec',
      ],
      'include_dirs': [
        '../core/src/fxcodec/fx_zlib/include/',
      ],
      'sources': [
        'fx_lpng/include/fx_png.h',
        'fx_lpng/src/fx_png.c',
        'fx_lpng/src/fx_pngerror.c',
        'fx_lpng/src/fx_pngget.c',
        'fx_lpng/src/fx_pngmem.c',
        'fx_lpng/src/fx_pngpread.c',
        'fx_lpng/src/fx_pngread.c',
        'fx_lpng/src/fx_pngrio.c',
        'fx_lpng/src/fx_pngrtran.c',
        'fx_lpng/src/fx_pngrutil.c',
        'fx_lpng/src/fx_pngset.c',
        'fx_lpng/src/fx_pngtrans.c',
        'fx_lpng/src/fx_pngwio.c',
        'fx_lpng/src/fx_pngwrite.c',
        'fx_lpng/src/fx_pngwtran.c',
        'fx_lpng/src/fx_pngwutil.c',
      ],
    },
  ],
}
