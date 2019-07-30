// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_layoutitem.h"

#include <utility>

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

void XFA_ReleaseLayoutItem(const RetainPtr<CXFA_LayoutItem>& pLayoutItem) {
  RetainPtr<CXFA_LayoutItem> pNode(pLayoutItem->GetFirstChild());
  while (pNode) {
    RetainPtr<CXFA_LayoutItem> pNext(pNode->GetNextSibling());
    XFA_ReleaseLayoutItem(pNode);
    pNode = std::move(pNext);
  }
  CXFA_Document* pDocument = pLayoutItem->GetFormNode()->GetDocument();
  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(pDocument);
  pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem.Get());
  if (pLayoutItem->GetFormNode()->GetElementType() == XFA_Element::PageArea) {
    pNotify->OnPageEvent(ToViewLayoutItem(pLayoutItem.Get()),
                         XFA_PAGEVIEWEVENT_PostRemoved);
  }
  pLayoutItem->RemoveSelfIfParented();
}

CXFA_LayoutItem::CXFA_LayoutItem(CXFA_Node* pNode, ItemType type)
    : m_ItemType(type), m_pFormNode(pNode) {}

CXFA_LayoutItem::~CXFA_LayoutItem() {
  CHECK(!GetParent());
}

CXFA_ViewLayoutItem* CXFA_LayoutItem::AsViewLayoutItem() {
  return IsViewLayoutItem() ? static_cast<CXFA_ViewLayoutItem*>(this) : nullptr;
}

const CXFA_ViewLayoutItem* CXFA_LayoutItem::AsViewLayoutItem() const {
  return IsViewLayoutItem() ? static_cast<const CXFA_ViewLayoutItem*>(this)
                            : nullptr;
}

CXFA_ContentLayoutItem* CXFA_LayoutItem::AsContentLayoutItem() {
  return IsContentLayoutItem() ? static_cast<CXFA_ContentLayoutItem*>(this)
                               : nullptr;
}

const CXFA_ContentLayoutItem* CXFA_LayoutItem::AsContentLayoutItem() const {
  return IsContentLayoutItem()
             ? static_cast<const CXFA_ContentLayoutItem*>(this)
             : nullptr;
}

const CXFA_ViewLayoutItem* CXFA_LayoutItem::GetPage() const {
  for (CXFA_LayoutItem* pCurNode = const_cast<CXFA_LayoutItem*>(this); pCurNode;
       pCurNode = pCurNode->GetParent()) {
    if (pCurNode->m_pFormNode->GetElementType() == XFA_Element::PageArea)
      return pCurNode->AsViewLayoutItem();
  }
  return nullptr;
}
