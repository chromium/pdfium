// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/fx_dib.h"

#include <tuple>
#include <utility>

#include "build/build_config.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#endif

#if BUILDFLAG(IS_WIN)
static_assert(sizeof(FX_COLORREF) == sizeof(COLORREF),
              "FX_COLORREF vs. COLORREF mismatch");
#endif

FXDIB_Format MakeRGBFormat(int bpp) {
  switch (bpp) {
    case 1:
      return FXDIB_Format::k1bppRgb;
    case 8:
      return FXDIB_Format::k8bppRgb;
    case 24:
      return FXDIB_Format::kRgb;
    case 32:
      return FXDIB_Format::kRgb32;
    default:
      return FXDIB_Format::kInvalid;
  }
}

FXDIB_ResampleOptions::FXDIB_ResampleOptions() = default;

bool FXDIB_ResampleOptions::HasAnyOptions() const {
  return bInterpolateBilinear || bHalftone || bNoSmoothing || bLossy;
}

std::tuple<int, int, int, int> ArgbDecode(FX_ARGB argb) {
  return std::make_tuple(FXARGB_A(argb), FXARGB_R(argb), FXARGB_G(argb),
                         FXARGB_B(argb));
}

std::pair<int, FX_COLORREF> ArgbToAlphaAndColorRef(FX_ARGB argb) {
  return {FXARGB_A(argb), ArgbToColorRef(argb)};
}

FX_COLORREF ArgbToColorRef(FX_ARGB argb) {
  return FXSYS_BGR(FXARGB_B(argb), FXARGB_G(argb), FXARGB_R(argb));
}

FX_ARGB AlphaAndColorRefToArgb(int a, FX_COLORREF colorref) {
  return ArgbEncode(a, FXSYS_GetRValue(colorref), FXSYS_GetGValue(colorref),
                    FXSYS_GetBValue(colorref));
}
