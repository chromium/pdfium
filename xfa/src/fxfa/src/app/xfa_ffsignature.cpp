// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffpageview.h"
#include "xfa_ffsignature.h"
#include "xfa_ffdoc.h"
CXFA_FFSignature::CXFA_FFSignature(CXFA_FFPageView* pPageView,
                                   CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pPageView, pDataAcc) {}
CXFA_FFSignature::~CXFA_FFSignature() {}
FX_BOOL CXFA_FFSignature::LoadWidget() {
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFSignature::RenderWidget(CFX_Graphics* pGS,
                                    CFX_Matrix* pMatrix,
                                    FX_DWORD dwStatus,
                                    int32_t iRotate) {
  if (!IsMatchVisibleStatus(dwStatus)) {
    return;
  }
  CFX_Matrix mtRotate;
  GetRotateMatrix(mtRotate);
  if (pMatrix) {
    mtRotate.Concat(*pMatrix);
  }
  CXFA_FFWidget::RenderWidget(pGS, &mtRotate, dwStatus);
  CXFA_Border borderUI = m_pDataAcc->GetUIBorder();
  DrawBorder(pGS, borderUI, m_rtUI, &mtRotate);
  RenderCaption(pGS, &mtRotate);
  DrawHighlight(pGS, &mtRotate, dwStatus, FALSE);
  CFX_RectF rtWidget = m_rtUI;
  IXFA_DocProvider* pDocProvider = m_pDataAcc->GetDoc()->GetDocProvider();
  FXSYS_assert(pDocProvider);
  pDocProvider->RenderCustomWidget(this, pGS, &mtRotate, rtWidget);
}
FX_BOOL CXFA_FFSignature::OnMouseEnter() {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnMouseExit() {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnLButtonDown(FX_DWORD dwFlags,
                                        FX_FLOAT fx,
                                        FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnLButtonUp(FX_DWORD dwFlags,
                                      FX_FLOAT fx,
                                      FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnLButtonDblClk(FX_DWORD dwFlags,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnMouseMove(FX_DWORD dwFlags,
                                      FX_FLOAT fx,
                                      FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnMouseWheel(FX_DWORD dwFlags,
                                       int16_t zDelta,
                                       FX_FLOAT fx,
                                       FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnRButtonDown(FX_DWORD dwFlags,
                                        FX_FLOAT fx,
                                        FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnRButtonUp(FX_DWORD dwFlags,
                                      FX_FLOAT fx,
                                      FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnRButtonDblClk(FX_DWORD dwFlags,
                                          FX_FLOAT fx,
                                          FX_FLOAT fy) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnKeyDown(FX_DWORD dwKeyCode, FX_DWORD dwFlags) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnKeyUp(FX_DWORD dwKeyCode, FX_DWORD dwFlags) {
  return FALSE;
}
FX_BOOL CXFA_FFSignature::OnChar(FX_DWORD dwChar, FX_DWORD dwFlags) {
  return FALSE;
}
FX_DWORD CXFA_FFSignature::OnHitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pNormalWidget) {
    FX_FLOAT ffx = fx, ffy = fy;
    FWLToClient(ffx, ffy);
    FX_DWORD dwWidgetHit = m_pNormalWidget->HitTest(ffx, ffy);
    if (dwWidgetHit != FWL_WGTHITTEST_Unknown) {
      return FWL_WGTHITTEST_Client;
    }
  }
  CFX_RectF rtBox;
  GetRectWithoutRotate(rtBox);
  if (!rtBox.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Unknown;
  }
  if (m_rtCaption.Contains(fx, fy)) {
    return FWL_WGTHITTEST_Titlebar;
  }
  return FWL_WGTHITTEST_Client;
}
FX_BOOL CXFA_FFSignature::OnSetCursor(FX_FLOAT fx, FX_FLOAT fy) {
  return FALSE;
}
