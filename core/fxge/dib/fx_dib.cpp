// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/fx_dib.h"

#include <tuple>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_extension.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#if defined(OS_WIN)
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

FX_RECT FXDIB_SwapClipBox(const FX_RECT& clip,
                          int width,
                          int height,
                          bool bFlipX,
                          bool bFlipY) {
  FX_RECT rect;
  if (bFlipY) {
    rect.left = height - clip.top;
    rect.right = height - clip.bottom;
  } else {
    rect.left = clip.top;
    rect.right = clip.bottom;
  }
  if (bFlipX) {
    rect.top = width - clip.left;
    rect.bottom = width - clip.right;
  } else {
    rect.top = clip.left;
    rect.bottom = clip.right;
  }
  rect.Normalize();
  return rect;
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

#if defined(PDF_ENABLE_XFA)
FX_ARGB StringToFXARGB(WideStringView view) {
  static constexpr FX_ARGB kDefaultValue = 0xff000000;
  if (view.IsEmpty())
    return kDefaultValue;

  int cc = 0;
  const wchar_t* str = view.unterminated_c_str();
  int len = view.GetLength();
  while (cc < len && FXSYS_iswspace(str[cc]))
    cc++;

  if (cc >= len)
    return kDefaultValue;

  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  while (cc < len) {
    if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
      break;

    r = r * 10 + str[cc] - '0';
    cc++;
  }
  if (cc < len && str[cc] == ',') {
    cc++;
    while (cc < len && FXSYS_iswspace(str[cc]))
      cc++;

    while (cc < len) {
      if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
        break;

      g = g * 10 + str[cc] - '0';
      cc++;
    }
    if (cc < len && str[cc] == ',') {
      cc++;
      while (cc < len && FXSYS_iswspace(str[cc]))
        cc++;

      while (cc < len) {
        if (str[cc] == ',' || !FXSYS_IsDecimalDigit(str[cc]))
          break;

        b = b * 10 + str[cc] - '0';
        cc++;
      }
    }
  }
  return ArgbEncode(0xFF, r, g, b);
}
#endif  // defined(PDF_ENABLE_XFA)
