// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_utils.h"

#include <algorithm>
#include <memory>
#include <sstream>

#include "core/fpdfdoc/cpvt_word.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_icon.h"

CFX_ByteString CPWL_Utils::GetRectFillAppStream(const CFX_FloatRect& rect,
                                                const CFX_Color& color) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor = GetColorAppStream(color, true);
  if (sColor.GetLength() > 0) {
    sAppStream << "q\n" << sColor;
    sAppStream << rect.left << " " << rect.bottom << " "
               << rect.right - rect.left << " " << rect.top - rect.bottom
               << " re f\nQ\n";
  }

  return CFX_ByteString(sAppStream);
}

CFX_ByteString CPWL_Utils::GetEditAppStream(CFX_Edit* pEdit,
                                            const CFX_PointF& ptOffset,
                                            const CPVT_WordRange* pRange,
                                            bool bContinuous,
                                            uint16_t SubWord) {
  return CFX_Edit::GetEditAppearanceStream(pEdit, ptOffset, pRange, bContinuous,
                                           SubWord);
}

CFX_ByteString CPWL_Utils::GetEditSelAppStream(CFX_Edit* pEdit,
                                               const CFX_PointF& ptOffset,
                                               const CPVT_WordRange* pRange) {
  return CFX_Edit::GetSelectAppearanceStream(pEdit, ptOffset, pRange);
}

CFX_ByteString CPWL_Utils::GetColorAppStream(const CFX_Color& color,
                                             const bool& bFillOrStroke) {
  std::ostringstream sColorStream;

  switch (color.nColorType) {
    case COLORTYPE_RGB:
      sColorStream << color.fColor1 << " " << color.fColor2 << " "
                   << color.fColor3 << " " << (bFillOrStroke ? "rg" : "RG")
                   << "\n";
      break;
    case COLORTYPE_GRAY:
      sColorStream << color.fColor1 << " " << (bFillOrStroke ? "g" : "G")
                   << "\n";
      break;
    case COLORTYPE_CMYK:
      sColorStream << color.fColor1 << " " << color.fColor2 << " "
                   << color.fColor3 << " " << color.fColor4 << " "
                   << (bFillOrStroke ? "k" : "K") << "\n";
      break;
  }

  return CFX_ByteString(sColorStream);
}

CFX_ByteString CPWL_Utils::GetBorderAppStream(const CFX_FloatRect& rect,
                                              float fWidth,
                                              const CFX_Color& color,
                                              const CFX_Color& crLeftTop,
                                              const CFX_Color& crRightBottom,
                                              BorderStyle nStyle,
                                              const CPWL_Dash& dash) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor;

  float fLeft = rect.left;
  float fRight = rect.right;
  float fTop = rect.top;
  float fBottom = rect.bottom;

  if (fWidth > 0.0f) {
    float fHalfWidth = fWidth / 2.0f;

    sAppStream << "q\n";

    switch (nStyle) {
      default:
      case BorderStyle::SOLID:
        sColor = CPWL_Utils::GetColorAppStream(color, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " "
                     << fTop - fBottom << " re\n";
          sAppStream << fLeft + fWidth << " " << fBottom + fWidth << " "
                     << fRight - fLeft - fWidth * 2 << " "
                     << fTop - fBottom - fWidth * 2 << " re\n";
          sAppStream << "f*\n";
        }
        break;
      case BorderStyle::DASH:
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fWidth << " w"
                     << " [" << dash.nDash << " " << dash.nGap << "] "
                     << dash.nPhase << " d\n";
          sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2
                     << " m\n";
          sAppStream << fLeft + fWidth / 2 << " " << fTop - fWidth / 2
                     << " l\n";
          sAppStream << fRight - fWidth / 2 << " " << fTop - fWidth / 2
                     << " l\n";
          sAppStream << fRight - fWidth / 2 << " " << fBottom + fWidth / 2
                     << " l\n";
          sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2
                     << " l S\n";
        }
        break;
      case BorderStyle::BEVELED:
      case BorderStyle::INSET:
        sColor = CPWL_Utils::GetColorAppStream(crLeftTop, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth
                     << " m\n";
          sAppStream << fLeft + fHalfWidth << " " << fTop - fHalfWidth
                     << " l\n";
          sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth
                     << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l f\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(crRightBottom, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth
                     << " m\n";
          sAppStream << fRight - fHalfWidth << " " << fBottom + fHalfWidth
                     << " l\n";
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l f\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(color, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " "
                     << fTop - fBottom << " re\n";
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " "
                     << fRight - fLeft - fHalfWidth * 2 << " "
                     << fTop - fBottom - fHalfWidth * 2 << " re f*\n";
        }
        break;
      case BorderStyle::UNDERLINE:
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fWidth << " w\n";
          sAppStream << fLeft << " " << fBottom + fWidth / 2 << " m\n";
          sAppStream << fRight << " " << fBottom + fWidth / 2 << " l S\n";
        }
        break;
    }

    sAppStream << "Q\n";
  }

  return CFX_ByteString(sAppStream);
}
