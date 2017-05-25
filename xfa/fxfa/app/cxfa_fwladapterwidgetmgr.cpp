// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_fwladapterwidgetmgr.h"

#include "xfa/fxfa/app/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

CXFA_FWLAdapterWidgetMgr::CXFA_FWLAdapterWidgetMgr() {}

CXFA_FWLAdapterWidgetMgr::~CXFA_FWLAdapterWidgetMgr() {}

void CXFA_FWLAdapterWidgetMgr::RepaintWidget(CFWL_Widget* pWidget) {
  if (!pWidget)
    return;

  CXFA_FFWidget* pFFWidget = pWidget->GetLayoutItem();
  if (!pFFWidget)
    return;

  pFFWidget->AddInvalidateRect(nullptr);
}

bool CXFA_FWLAdapterWidgetMgr::GetPopupPos(CFWL_Widget* pWidget,
                                           float fMinHeight,
                                           float fMaxHeight,
                                           const CFX_RectF& rtAnchor,
                                           CFX_RectF& rtPopup) {
  CXFA_FFWidget* pFFWidget = pWidget->GetLayoutItem();
  CFX_RectF rtRotateAnchor(rtAnchor);
  pFFWidget->GetRotateMatrix().TransformRect(rtRotateAnchor);
  pFFWidget->GetDoc()->GetDocEnvironment()->GetPopupPos(
      pFFWidget, fMinHeight, fMaxHeight, rtRotateAnchor, rtPopup);
  return true;
}
