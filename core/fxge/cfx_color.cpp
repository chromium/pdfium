// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_color.h"

#include <algorithm>

namespace {

bool InRange(float comp) {
  return comp >= 0.0f && comp <= 1.0f;
}

CFX_Color ConvertCMYK2GRAY(float dC, float dM, float dY, float dK) {
  if (!InRange(dC) || !InRange(dM) || !InRange(dY) || !InRange(dK))
    return CFX_Color(COLORTYPE_GRAY);
  return CFX_Color(
      COLORTYPE_GRAY,
      1.0f - std::min(1.0f, 0.3f * dC + 0.59f * dM + 0.11f * dY + dK));
}

CFX_Color ConvertGRAY2CMYK(float dGray) {
  if (!InRange(dGray))
    return CFX_Color(COLORTYPE_CMYK);
  return CFX_Color(COLORTYPE_CMYK, 0.0f, 0.0f, 0.0f, 1.0f - dGray);
}

CFX_Color ConvertGRAY2RGB(float dGray) {
  if (!InRange(dGray))
    return CFX_Color(COLORTYPE_RGB);
  return CFX_Color(COLORTYPE_RGB, dGray, dGray, dGray);
}

CFX_Color ConvertRGB2GRAY(float dR, float dG, float dB) {
  if (!InRange(dR) || !InRange(dG) || !InRange(dB))
    return CFX_Color(COLORTYPE_GRAY);
  return CFX_Color(COLORTYPE_GRAY, 0.3f * dR + 0.59f * dG + 0.11f * dB);
}

CFX_Color ConvertCMYK2RGB(float dC, float dM, float dY, float dK) {
  if (!InRange(dC) || !InRange(dM) || !InRange(dY) || !InRange(dK))
    return CFX_Color(COLORTYPE_RGB);
  return CFX_Color(COLORTYPE_RGB, 1.0f - std::min(1.0f, dC + dK),
                   1.0f - std::min(1.0f, dM + dK),
                   1.0f - std::min(1.0f, dY + dK));
}

CFX_Color ConvertRGB2CMYK(float dR, float dG, float dB) {
  if (!InRange(dR) || !InRange(dG) || !InRange(dB))
    return CFX_Color(COLORTYPE_CMYK);

  float c = 1.0f - dR;
  float m = 1.0f - dG;
  float y = 1.0f - dB;
  return CFX_Color(COLORTYPE_CMYK, c, m, y, std::min(c, std::min(m, y)));
}

}  // namespace

CFX_Color CFX_Color::ConvertColorType(int32_t nConvertColorType) const {
  if (nColorType == nConvertColorType)
    return *this;

  CFX_Color ret;
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT:
      ret = *this;
      ret.nColorType = COLORTYPE_TRANSPARENT;
      break;
    case COLORTYPE_GRAY:
      switch (nConvertColorType) {
        case COLORTYPE_RGB:
          ret = ConvertGRAY2RGB(fColor1);
          break;
        case COLORTYPE_CMYK:
          ret = ConvertGRAY2CMYK(fColor1);
          break;
      }
      break;
    case COLORTYPE_RGB:
      switch (nConvertColorType) {
        case COLORTYPE_GRAY:
          ret = ConvertRGB2GRAY(fColor1, fColor2, fColor3);
          break;
        case COLORTYPE_CMYK:
          ret = ConvertRGB2CMYK(fColor1, fColor2, fColor3);
          break;
      }
      break;
    case COLORTYPE_CMYK:
      switch (nConvertColorType) {
        case COLORTYPE_GRAY:
          ret = ConvertCMYK2GRAY(fColor1, fColor2, fColor3, fColor4);
          break;
        case COLORTYPE_RGB:
          ret = ConvertCMYK2RGB(fColor1, fColor2, fColor3, fColor4);
          break;
      }
      break;
  }
  return ret;
}

FX_COLORREF CFX_Color::ToFXColor(int32_t nTransparency) const {
  CFX_Color ret;
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT: {
      ret = CFX_Color(COLORTYPE_TRANSPARENT, 0, 0, 0, 0);
      break;
    }
    case COLORTYPE_GRAY: {
      ret = ConvertGRAY2RGB(fColor1);
      ret.fColor4 = nTransparency;
      break;
    }
    case COLORTYPE_RGB: {
      ret = CFX_Color(COLORTYPE_RGB, fColor1, fColor2, fColor3);
      ret.fColor4 = nTransparency;
      break;
    }
    case COLORTYPE_CMYK: {
      ret = ConvertCMYK2RGB(fColor1, fColor2, fColor3, fColor4);
      ret.fColor4 = nTransparency;
      break;
    }
  }
  return ArgbEncode(ret.fColor4, static_cast<int32_t>(ret.fColor1 * 255),
                    static_cast<int32_t>(ret.fColor2 * 255),
                    static_cast<int32_t>(ret.fColor3 * 255));
}

CFX_Color CFX_Color::operator-(float fColorSub) const {
  CFX_Color sRet(nColorType);
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT:
      sRet.nColorType = COLORTYPE_RGB;
      sRet.fColor1 = std::max(1.0f - fColorSub, 0.0f);
      sRet.fColor2 = std::max(1.0f - fColorSub, 0.0f);
      sRet.fColor3 = std::max(1.0f - fColorSub, 0.0f);
      break;
    case COLORTYPE_RGB:
    case COLORTYPE_GRAY:
    case COLORTYPE_CMYK:
      sRet.fColor1 = std::max(fColor1 - fColorSub, 0.0f);
      sRet.fColor2 = std::max(fColor2 - fColorSub, 0.0f);
      sRet.fColor3 = std::max(fColor3 - fColorSub, 0.0f);
      sRet.fColor4 = std::max(fColor4 - fColorSub, 0.0f);
      break;
  }
  return sRet;
}

CFX_Color CFX_Color::operator/(float fColorDivide) const {
  CFX_Color sRet(nColorType);
  switch (nColorType) {
    case COLORTYPE_TRANSPARENT:
      sRet.nColorType = COLORTYPE_RGB;
      sRet.fColor1 = 1.0f / fColorDivide;
      sRet.fColor2 = 1.0f / fColorDivide;
      sRet.fColor3 = 1.0f / fColorDivide;
      break;
    case COLORTYPE_RGB:
    case COLORTYPE_GRAY:
    case COLORTYPE_CMYK:
      sRet = *this;
      sRet.fColor1 /= fColorDivide;
      sRet.fColor2 /= fColorDivide;
      sRet.fColor3 /= fColorDivide;
      sRet.fColor4 /= fColorDivide;
      break;
  }
  return sRet;
}
