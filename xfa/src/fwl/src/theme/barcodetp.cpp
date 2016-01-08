// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
CFWL_BarcodeTP::CFWL_BarcodeTP() {}
CFWL_BarcodeTP::~CFWL_BarcodeTP() {}
FX_BOOL CFWL_BarcodeTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_Barcode;
}
FX_BOOL CFWL_BarcodeTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_BCD_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_BCD_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_BCD_Background: {
      FillBackground(pParams->m_pGraphics, &pParams->m_rtPart,
                     &pParams->m_matrix);
      break;
    }
    default: {}
  }
  return TRUE;
}
