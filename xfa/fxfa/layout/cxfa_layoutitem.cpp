// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/layout/cxfa_layoutitem.h"

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_layoutprocessor.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

void XFA_ReleaseLayoutItem(CXFA_LayoutItem* pLayoutItem) {
  CXFA_LayoutItem* pNode = pLayoutItem->GetFirstChild();
  while (pNode) {
    CXFA_LayoutItem* pNext = pNode->GetNextSibling();
    XFA_ReleaseLayoutItem(pNode);
    pNode = pNext;
  }
  CXFA_Document* pDocument = pLayoutItem->GetFormNode()->GetDocument();
  CXFA_FFNotify* pNotify = pDocument->GetNotify();
  auto* pDocLayout = CXFA_LayoutProcessor::FromDocument(pDocument);
  pNotify->OnLayoutItemRemoving(pDocLayout, pLayoutItem);
  if (pLayoutItem->GetFormNode()->GetElementType() == XFA_Element::PageArea) {
    pNotify->OnPageViewEvent(ToViewLayoutItem(pLayoutItem),
                             CXFA_FFDoc::PageViewEvent::kPostRemoved);
  }
  pLayoutItem->RemoveSelfIfParented();
}

CXFA_LayoutItem::CXFA_LayoutItem(CXFA_Node* pNode, ItemType type)
    : m_ItemType(type), m_pFormNode(pNode) {}

CXFA_LayoutItem::~CXFA_LayoutItem() = default;

void CXFA_LayoutItem::PreFinalize() {
  if (!m_pFormNode)
    return;

  auto* pJSObj = m_pFormNode->JSObject();
  if (pJSObj && pJSObj->GetLayoutItem() == this)
    pJSObj->SetLayoutItem(nullptr);
}

void CXFA_LayoutItem::Trace(cppgc::Visitor* visitor) const {
  GCedTreeNode<CXFA_LayoutItem>::Trace(visitor);
  visitor->Trace(m_pFormNode);
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

void CXFA_LayoutItem::SetFormNode(CXFA_Node* pNode) {
  // Not in header, assignment requires complete type, not just forward decl.
  m_pFormNode = pNode;
}
