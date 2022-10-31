// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fftext.h"

#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/layout/cfgas_linkuserdata.h"
#include "xfa/fwl/fwl_widgethit.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/parser/cxfa_margin.h"

CXFA_FFText::CXFA_FFText(CXFA_Node* pNode) : CXFA_FFWidget(pNode) {}

CXFA_FFText::~CXFA_FFText() = default;

void CXFA_FFText::RenderWidget(CFGAS_GEGraphics* pGS,
                               const CFX_Matrix& matrix,
                               HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);

  CXFA_TextLayout* pTextLayout = m_pNode->GetTextLayout();
  if (!pTextLayout)
    return;

  CFX_RenderDevice* pRenderDevice = pGS->GetRenderDevice();
  CFX_RectF rtText = GetRectWithoutRotate();
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin) {
    CXFA_ContentLayoutItem* pItem = GetLayoutItem();
    if (!pItem->GetPrev() && !pItem->GetNext()) {
      XFA_RectWithoutMargin(&rtText, margin);
    } else {
      float fTopInset = 0;
      float fBottomInset = 0;
      if (!pItem->GetPrev())
        fTopInset = margin->GetTopInset();
      else if (!pItem->GetNext())
        fBottomInset = margin->GetBottomInset();

      rtText.Deflate(margin->GetLeftInset(), fTopInset, margin->GetRightInset(),
                     fBottomInset);
    }
  }

  CFX_Matrix mt(1, 0, 0, 1, rtText.left, rtText.top);
  CFX_RectF rtClip = mtRotate.TransformRect(rtText);
  mt.Concat(mtRotate);
  pTextLayout->DrawString(pRenderDevice, mt, rtClip,
                          GetLayoutItem()->GetIndex());
}

bool CXFA_FFText::IsLoaded() {
  CXFA_TextLayout* pTextLayout = m_pNode->GetTextLayout();
  return pTextLayout && !pTextLayout->HasBlock();
}

bool CXFA_FFText::PerformLayout() {
  CXFA_FFWidget::PerformLayout();
  CXFA_TextLayout* pTextLayout = m_pNode->GetTextLayout();
  if (!pTextLayout)
    return false;
  if (!pTextLayout->HasBlock())
    return true;

  pTextLayout->ClearBlocks();
  CXFA_ContentLayoutItem* pItem = GetLayoutItem();
  if (!pItem->GetPrev() && !pItem->GetNext())
    return true;

  pItem = pItem->GetFirst();
  while (pItem) {
    CFX_RectF rtText = pItem->GetAbsoluteRect();
    CXFA_Margin* margin = m_pNode->GetMarginIfExists();
    if (margin) {
      if (!pItem->GetPrev())
        rtText.height -= margin->GetTopInset();
      else if (!pItem->GetNext())
        rtText.height -= margin->GetBottomInset();
    }
    pTextLayout->ItemBlocks(rtText, pItem->GetIndex());
    pItem = pItem->GetNext();
  }
  pTextLayout->ResetHasBlock();
  return true;
}

bool CXFA_FFText::AcceptsFocusOnButtonDown(
    Mask<XFA_FWL_KeyFlag> dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  return command == CFWL_MessageMouse::MouseCommand::kLeftButtonDown &&
         GetRectWithoutRotate().Contains(point) &&
         !GetLinkURLAtPoint(point).IsEmpty();
}

bool CXFA_FFText::OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                const CFX_PointF& point) {
  SetButtonDown(true);
  return true;
}

bool CXFA_FFText::OnMouseMove(Mask<XFA_FWL_KeyFlag> dwFlags,
                              const CFX_PointF& point) {
  return GetRectWithoutRotate().Contains(point) &&
         !GetLinkURLAtPoint(point).IsEmpty();
}

bool CXFA_FFText::OnLButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                              const CFX_PointF& point) {
  if (!IsButtonDown())
    return false;

  SetButtonDown(false);
  WideString wsURLContent = GetLinkURLAtPoint(point);
  if (wsURLContent.IsEmpty())
    return false;

  GetDoc()->GotoURL(wsURLContent);
  return true;
}

FWL_WidgetHit CXFA_FFText::HitTest(const CFX_PointF& point) {
  if (GetRectWithoutRotate().Contains(point) &&
      !GetLinkURLAtPoint(point).IsEmpty()) {
    return FWL_WidgetHit::HyperLink;
  }
  return FWL_WidgetHit::Unknown;
}

WideString CXFA_FFText::GetLinkURLAtPoint(const CFX_PointF& point) {
  CXFA_TextLayout* pTextLayout = m_pNode->GetTextLayout();
  if (!pTextLayout)
    return WideString();

  CFX_RectF rect = GetRectWithoutRotate();
  return pTextLayout->GetLinkURLAtPoint(point - rect.TopLeft());
}
