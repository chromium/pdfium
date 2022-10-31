// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_barcodetp.h"

#include "xfa/fwl/cfwl_barcode.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"

CFWL_BarcodeTP::CFWL_BarcodeTP() = default;

CFWL_BarcodeTP::~CFWL_BarcodeTP() = default;

void CFWL_BarcodeTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder:
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    case CFWL_ThemePart::Part::kBackground:
      FillBackground(pParams.GetGraphics(), pParams.m_PartRect,
                     pParams.m_matrix);
      break;
    default:
      break;
  }
}
