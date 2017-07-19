// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_UTILS_H_
#define FPDFSDK_PDFWINDOW_CPWL_UTILS_H_

#include "core/fpdfdoc/cpvt_wordrange.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

class CFX_Edit;
struct CFX_Color;

// checkbox & radiobutton style
#define PCS_CHECK 0
#define PCS_CIRCLE 1
#define PCS_CROSS 2
#define PCS_DIAMOND 3
#define PCS_SQUARE 4
#define PCS_STAR 5

// pushbutton layout style
#define PPBL_LABEL 0
#define PPBL_ICON 1
#define PPBL_ICONTOPLABELBOTTOM 2
#define PPBL_LABELTOPICONBOTTOM 3
#define PPBL_ICONLEFTLABELRIGHT 4
#define PPBL_LABELLEFTICONRIGHT 5
#define PPBL_LABELOVERICON 6

class CPWL_Utils {
 public:
  static CFX_ByteString GetColorAppStream(const CFX_Color& color,
                                          const bool& bFillOrStroke = true);
  static CFX_ByteString GetBorderAppStream(const CFX_FloatRect& rect,
                                           float fWidth,
                                           const CFX_Color& color,
                                           const CFX_Color& crLeftTop,
                                           const CFX_Color& crRightBottom,
                                           BorderStyle nStyle,
                                           const CPWL_Dash& dash);
  static CFX_ByteString GetRectFillAppStream(const CFX_FloatRect& rect,
                                             const CFX_Color& color);
  static CFX_ByteString GetEditAppStream(CFX_Edit* pEdit,
                                         const CFX_PointF& ptOffset,
                                         const CPVT_WordRange* pRange = nullptr,
                                         bool bContinuous = true,
                                         uint16_t SubWord = 0);
  static CFX_ByteString GetEditSelAppStream(
      CFX_Edit* pEdit,
      const CFX_PointF& ptOffset,
      const CPVT_WordRange* pRange = nullptr);
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_UTILS_H_
