// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_fwladapter.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffdoc.h"
IFWL_AdapterNative* FWL_CreateFuelAdapterNative() {
  return NULL;
}
void FWL_ReleaseFuelAdapterNative(IFWL_AdapterNative* native) {}
void FWL_PostMessageToMainRoop(CFWL_Message* pMessage) {}
FX_BOOL FWL_ShowCaret(IFWL_Widget* pWidget,
                      FX_BOOL bVisible,
                      const CFX_RectF* pRtAnchor) {
  CXFA_FFWidget* pXFAWidget = (CXFA_FFWidget*)pWidget->GetPrivateData(pWidget);
  if (!pXFAWidget) {
    return FALSE;
  }
  IXFA_DocProvider* pDocProvider = pXFAWidget->GetDoc()->GetDocProvider();
  if (!pDocProvider) {
    return FALSE;
  }
  if (bVisible) {
    CFX_Matrix mt;
    pXFAWidget->GetRotateMatrix(mt);
    CFX_RectF rt(*pRtAnchor);
    mt.TransformRect(rt);
    pDocProvider->DisplayCaret(pXFAWidget, bVisible, &rt);
    return TRUE;
  }
  pDocProvider->DisplayCaret(pXFAWidget, bVisible, pRtAnchor);
  return TRUE;
}
FWL_ERR CXFA_FWLAdapterWidgetMgr::RepaintWidget(IFWL_Widget* pWidget,
                                                const CFX_RectF* pRect) {
  if (!pWidget)
    return FWL_ERR_Indefinite;
  CXFA_FFField* pField = (CXFA_FFField*)pWidget->GetPrivateData(pWidget);
  if (!pField)
    return FWL_ERR_Indefinite;
#ifdef _XFA_EMB
  CFX_RectF rtInvalidate;
  pWidget->GetWidgetRect(rtInvalidate);
  if (pRect) {
    rtInvalidate.left += pRect->left;
    rtInvalidate.top += pRect->top;
    rtInvalidate.width = pRect->width;
    rtInvalidate.height = pRect->height;
  }
  CFX_Matrix mt;
  pField->GetRotateMatrix(mt);
  mt.TransformRect(rtInvalidate);
  pField->AddInvalidateRect(&rtInvalidate);
#else
  pField->AddInvalidateRect(NULL);
#endif
  return FWL_ERR_Succeeded;
}
FX_BOOL CXFA_FWLAdapterWidgetMgr::GetPopupPos(IFWL_Widget* pWidget,
                                              FX_FLOAT fMinHeight,
                                              FX_FLOAT fMaxHeight,
                                              const CFX_RectF& rtAnchor,
                                              CFX_RectF& rtPopup) {
  CXFA_FFWidget* pFFWidget = (CXFA_FFWidget*)(pWidget->GetPrivateData(pWidget));
  CFX_Matrix mt;
  pFFWidget->GetRotateMatrix(mt);
  CFX_RectF rtRotateAnchor(rtAnchor);
  mt.TransformRect(rtRotateAnchor);
  pFFWidget->GetDoc()->GetDocProvider()->GetPopupPos(
      pFFWidget, fMinHeight, fMaxHeight, rtRotateAnchor, rtPopup);
  return TRUE;
}
