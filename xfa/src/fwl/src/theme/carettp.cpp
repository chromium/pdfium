// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
CFWL_CaretTP::CFWL_CaretTP() {}
CFWL_CaretTP::~CFWL_CaretTP() {}
FX_BOOL CFWL_CaretTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_Caret;
}
FX_BOOL CFWL_CaretTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_CAT_Background: {
      if (!(pParams->m_dwStates & FWL_PARTSTATE_CAT_HightLight)) {
        return TRUE;
      }
      DrawCaretBK(pParams->m_pGraphics, pParams->m_dwStates,
                  &(pParams->m_rtPart), (CFX_Color*)pParams->m_pData,
                  &(pParams->m_matrix));
      break;
    }
  }
  return TRUE;
}
void CFWL_CaretTP::DrawCaretBK(CFX_Graphics* pGraphics,
                               FX_DWORD dwStates,
                               const CFX_RectF* pRect,
                               CFX_Color* crFill,
                               CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  CFX_Color crFilltemp;
  crFill ? crFilltemp = *crFill : crFilltemp = ArgbEncode(255, 0, 0, 0);
  CFX_RectF rect = *pRect;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  pGraphics->SetFillColor(&crFilltemp);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
}
