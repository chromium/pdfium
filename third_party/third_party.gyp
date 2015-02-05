# Copyright 2014 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'bigint',
      'type': 'static_library',
      'sources': [
        'bigint/BigInteger.hh',
        'bigint/BigIntegerLibrary.hh',
        'bigint/BigIntegerUtils.hh',
        'bigint/BigUnsigned.hh',
        'bigint/NumberlikeArray.hh',
        'bigint/BigUnsignedInABase.hh',
        'bigint/BigInteger.cc',
        'bigint/BigIntegerUtils.cc',
        'bigint/BigUnsigned.cc',
        'bigint/BigUnsignedInABase.cc',
      ],
    },
    {
      'target_name': 'freetype',
      'type': 'static_library',
      'defines': [
        'FT2_BUILD_LIBRARY',
      ],
      'include_dirs': [
         'freetype/include',
      ],
      'sources': [
        'freetype/include/freetype.h',
        'freetype/include/ft2build.h',
        'freetype/include/ftmm.h',
        'freetype/include/ftotval.h',
        'freetype/include/ftoutln.h',
        'freetype/include/tttables.h',
        'freetype/include/internal/ftobjs.h',
        'freetype/include/internal/ftstream.h',
        'freetype/include/internal/tttypes.h',
        'freetype/src/cff/cffobjs.h',
        'freetype/src/cff/cfftypes.h',
        'freetype/src/cff/cff.c',
        'freetype/src/base/ftbase.c',
        'freetype/src/base/ftbitmap.c',
        'freetype/src/base/ftglyph.c',
        'freetype/src/base/ftinit.c',
        'freetype/src/base/ftlcdfil.c',
        'freetype/src/base/ftmm.c',
        'freetype/src/base/ftsystem.c',
        'freetype/src/psaux/psaux.c',
        'freetype/src/pshinter/pshinter.c',
        'freetype/src/psnames/psmodule.c',
        'freetype/src/raster/raster.c',
        'freetype/src/sfnt/sfnt.c',
        'freetype/src/smooth/smooth.c',
        'freetype/src/truetype/truetype.c',
        'freetype/src/type1/type1.c',
        'freetype/src/cid/type1cid.c',
      ],
    },
    {
      'target_name': 'pdfium_base',
      'type': 'none',
      'sources': [
        'base/logging.h',
        'base/macros.h',
        'base/template_util.h',
        'base/numerics/safe_conversions.h',
        'base/numerics/safe_conversions_impl.h',
        'base/numerics/safe_math.h',
        'base/numerics/safe_math_impl.h',
      ],
    },
  ],
}
