// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cfx_systemhandler.h"

#include <memory>

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_gemodule.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "third_party/base/ptr_util.h"

CFX_SystemHandler::CFX_SystemHandler(CPDFSDK_FormFillEnvironment* pFormFillEnv)
    : m_pFormFillEnv(pFormFillEnv) {
  ASSERT(m_pFormFillEnv);
}

CFX_SystemHandler::~CFX_SystemHandler() = default;

void CFX_SystemHandler::InvalidateRect(PerWindowData* pWidgetData,
                                       const CFX_FloatRect& rect) {
  auto* pPrivateData = static_cast<CFFL_PrivateData*>(pWidgetData);
  CPDFSDK_Widget* widget = pPrivateData->pWidget.Get();
  if (!widget)
    return;

  CPDFSDK_PageView* pPageView = widget->GetPageView();
  IPDF_Page* pPage = widget->GetPage();
  if (!pPage || !pPageView)
    return;

  CFX_Matrix device2page = pPageView->GetCurrentMatrix().GetInverse();
  CFX_PointF left_top = device2page.Transform(CFX_PointF(rect.left, rect.top));
  CFX_PointF right_bottom =
      device2page.Transform(CFX_PointF(rect.right, rect.bottom));

  CFX_FloatRect rcPDF(left_top.x, right_bottom.y, right_bottom.x, left_top.y);
  rcPDF.Normalize();
  m_pFormFillEnv->Invalidate(pPage, rcPDF.GetOuterRect());
}

void CFX_SystemHandler::OutputSelectedRect(CFFL_FormFiller* pFormFiller,
                                           const CFX_FloatRect& rect) {
  if (!pFormFiller)
    return;

  CFX_PointF ptA = pFormFiller->PWLtoFFL(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptB = pFormFiller->PWLtoFFL(CFX_PointF(rect.right, rect.top));

  CPDFSDK_Annot* pAnnot = pFormFiller->GetSDKAnnot();
  IPDF_Page* pPage = pAnnot->GetPage();
  ASSERT(pPage);

  m_pFormFillEnv->OutputSelectedRect(pPage,
                                     CFX_FloatRect(ptA.x, ptA.y, ptB.x, ptB.y));
}

bool CFX_SystemHandler::IsSelectionImplemented() const {
  FPDF_FORMFILLINFO* pInfo = m_pFormFillEnv->GetFormFillInfo();
  return pInfo && pInfo->FFI_OutputSelectedRect;
}

void CFX_SystemHandler::SetCursor(int32_t nCursorType) {
  m_pFormFillEnv->SetCursor(nCursorType);
}

int32_t CFX_SystemHandler::SetTimer(int32_t uElapse,
                                    TimerCallback lpTimerFunc) {
  return m_pFormFillEnv->SetTimer(uElapse, lpTimerFunc);
}

void CFX_SystemHandler::KillTimer(int32_t nID) {
  m_pFormFillEnv->KillTimer(nID);
}
