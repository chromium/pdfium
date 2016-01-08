// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_rendercontext.h"
#include "xfa_ffwidget.h"
#define XFA_RENDERCONTEXT_MaxCount 30
IXFA_RenderContext* XFA_RenderContext_Create() {
  return new CXFA_RenderContext;
}
CXFA_RenderContext::CXFA_RenderContext() {
  m_pWidgetIterator = NULL;
  m_pWidget = NULL;
  m_pPageView = NULL;
  m_pGS = NULL;
  m_dwStatus = 0;
  m_matrix.SetIdentity();
  m_rtClipRect.Reset();
}
CXFA_RenderContext::~CXFA_RenderContext() {
  StopRender();
}
int32_t CXFA_RenderContext::StartRender(IXFA_PageView* pPageView,
                                        CFX_Graphics* pGS,
                                        const CFX_Matrix& matrix,
                                        const CXFA_RenderOptions& options) {
  m_pPageView = pPageView;
  m_pGS = pGS;
  m_matrix = matrix;
  m_options = options;
  CFX_RectF rtPage;
  pGS->GetClipRect(rtPage);
  CFX_Matrix mtRes;
  mtRes.SetReverse(matrix);
  m_rtClipRect.Set(rtPage.left, rtPage.top, rtPage.width, rtPage.height);
  mtRes.TransformRect(m_rtClipRect);
  m_dwStatus = m_options.m_bHighlight ? XFA_WIDGETSTATUS_Highlight : 0;
  FX_DWORD dwFilterType = XFA_WIDGETFILTER_Visible | XFA_WIDGETFILTER_AllType |
                          (m_options.m_bPrint ? XFA_WIDGETSTATUS_Printable
                                              : XFA_WIDGETSTATUS_Viewable);
  m_pWidgetIterator =
      m_pPageView->CreateWidgetIterator(XFA_TRAVERSEWAY_Form, dwFilterType);
  m_pWidget = m_pWidgetIterator->MoveToNext();
  return XFA_RENDERSTATUS_Ready;
}
int32_t CXFA_RenderContext::DoRender(IFX_Pause* pPause) {
  int32_t iCount = 0;
  while (m_pWidget) {
    CXFA_FFWidget* pWidget = (CXFA_FFWidget*)m_pWidget;
    CFX_RectF rtWidgetBox;
    pWidget->GetBBox(rtWidgetBox, XFA_WIDGETSTATUS_Visible);
    rtWidgetBox.width += 1;
    rtWidgetBox.height += 1;
    if (rtWidgetBox.IntersectWith(m_rtClipRect)) {
      pWidget->RenderWidget(m_pGS, &m_matrix, m_dwStatus);
    }
    m_pWidget = m_pWidgetIterator->MoveToNext();
    iCount++;
    if (iCount > XFA_RENDERCONTEXT_MaxCount && pPause &&
        pPause->NeedToPauseNow()) {
      return XFA_RENDERSTATUS_ToBeContinued;
    }
  }
  return XFA_RENDERSTATUS_Done;
}
void CXFA_RenderContext::StopRender() {
  if (m_pWidgetIterator) {
    m_pWidgetIterator->Release();
    m_pWidgetIterator = NULL;
  }
}
