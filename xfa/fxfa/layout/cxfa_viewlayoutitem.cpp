// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"

#include <utility>

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_medium.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ViewLayoutItem::CXFA_ViewLayoutItem(
    CXFA_Node* pNode,
    std::unique_ptr<CXFA_FFPageView> pPageView)
    : CXFA_LayoutItem(pNode, kViewItem), m_pFFPageView(std::move(pPageView)) {
  if (m_pFFPageView)
    m_pFFPageView->SetLayoutItem(this);
}

CXFA_ViewLayoutItem::~CXFA_ViewLayoutItem() {
  if (m_pFFPageView)
    m_pFFPageView->SetLayoutItem(nullptr);
}

CXFA_LayoutProcessor* CXFA_ViewLayoutItem::GetLayout() const {
  return CXFA_LayoutProcessor::FromDocument(GetFormNode()->GetDocument());
}

int32_t CXFA_ViewLayoutItem::GetPageIndex() const {
  auto* pLayout =
      CXFA_LayoutProcessor::FromDocument(GetFormNode()->GetDocument());
  return pLayout->GetLayoutPageMgr()->GetPageIndex(this);
}

CFX_SizeF CXFA_ViewLayoutItem::GetPageSize() const {
  CFX_SizeF size;
  CXFA_Medium* pMedium =
      GetFormNode()->GetFirstChildByClass<CXFA_Medium>(XFA_Element::Medium);
  if (!pMedium)
    return size;

  size = CFX_SizeF(
      pMedium->JSObject()->GetMeasureInUnit(XFA_Attribute::Short, XFA_Unit::Pt),
      pMedium->JSObject()->GetMeasureInUnit(XFA_Attribute::Long, XFA_Unit::Pt));
  if (pMedium->JSObject()->GetEnum(XFA_Attribute::Orientation) ==
      XFA_AttributeValue::Landscape) {
    size = CFX_SizeF(size.height, size.width);
  }
  return size;
}

CXFA_Node* CXFA_ViewLayoutItem::GetMasterPage() const {
  return GetFormNode();
}
