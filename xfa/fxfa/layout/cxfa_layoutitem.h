// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/retained_tree_node.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_ContentLayoutItem;
class CXFA_LayoutProcessor;
class CXFA_ViewLayoutItem;

class CXFA_LayoutItem : public RetainedTreeNode<CXFA_LayoutItem> {
 public:
  ~CXFA_LayoutItem() override;

  bool IsViewLayoutItem() const { return m_ItemType == kViewItem; }
  bool IsContentLayoutItem() const { return m_ItemType == kContentItem; }
  CXFA_ViewLayoutItem* AsViewLayoutItem();
  const CXFA_ViewLayoutItem* AsViewLayoutItem() const;
  CXFA_ContentLayoutItem* AsContentLayoutItem();
  const CXFA_ContentLayoutItem* AsContentLayoutItem() const;

  const CXFA_ViewLayoutItem* GetPage() const;
  CXFA_Node* GetFormNode() const { return m_pFormNode.Get(); }
  void SetFormNode(CXFA_Node* pNode) { m_pFormNode = pNode; }

 protected:
  enum ItemType { kViewItem, kContentItem };
  CXFA_LayoutItem(CXFA_Node* pNode, ItemType type);

 private:
  const ItemType m_ItemType;
  UnownedPtr<CXFA_Node> m_pFormNode;
};

inline CXFA_ViewLayoutItem* ToViewLayoutItem(CXFA_LayoutItem* item) {
  return item ? item->AsViewLayoutItem() : nullptr;
}

inline CXFA_ContentLayoutItem* ToContentLayoutItem(CXFA_LayoutItem* item) {
  return item ? item->AsContentLayoutItem() : nullptr;
}

void XFA_ReleaseLayoutItem(const RetainPtr<CXFA_LayoutItem>& pLayoutItem);

#endif  // XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_
