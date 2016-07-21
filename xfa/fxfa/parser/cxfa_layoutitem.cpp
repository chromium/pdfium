// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_layoutitem.h"

#include "xfa/fxfa/parser/cxfa_containerlayoutitem.h"
#include "xfa/fxfa/parser/cxfa_contentlayoutitem.h"

CXFA_LayoutItem::CXFA_LayoutItem(CXFA_Node* pNode, FX_BOOL bIsContentLayoutItem)
    : m_pFormNode(pNode),
      m_pParent(nullptr),
      m_pNextSibling(nullptr),
      m_pFirstChild(nullptr),
      m_bIsContentLayoutItem(bIsContentLayoutItem) {}

CXFA_LayoutItem::~CXFA_LayoutItem() {}

CXFA_ContainerLayoutItem* CXFA_LayoutItem::AsContainerLayoutItem() {
  return IsContainerLayoutItem() ? static_cast<CXFA_ContainerLayoutItem*>(this)
                                 : nullptr;
}
CXFA_ContentLayoutItem* CXFA_LayoutItem::AsContentLayoutItem() {
  return IsContentLayoutItem() ? static_cast<CXFA_ContentLayoutItem*>(this)
                               : nullptr;
}
