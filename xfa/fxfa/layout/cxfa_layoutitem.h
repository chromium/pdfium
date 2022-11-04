// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_

#include "fxjs/gc/gced_tree_node.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"

class CXFA_ContentLayoutItem;
class CXFA_Node;
class CXFA_ViewLayoutItem;

class CXFA_LayoutItem : public GCedTreeNode<CXFA_LayoutItem> {
  CPPGC_USING_PRE_FINALIZER(CXFA_LayoutItem, PreFinalize);

 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_LayoutItem() override;

  void PreFinalize();

  // GCedTreeNode:
  void Trace(cppgc::Visitor* visitor) const override;

  bool IsViewLayoutItem() const { return m_ItemType == kViewItem; }
  bool IsContentLayoutItem() const { return m_ItemType == kContentItem; }
  CXFA_ViewLayoutItem* AsViewLayoutItem();
  const CXFA_ViewLayoutItem* AsViewLayoutItem() const;
  CXFA_ContentLayoutItem* AsContentLayoutItem();
  const CXFA_ContentLayoutItem* AsContentLayoutItem() const;

  const CXFA_ViewLayoutItem* GetPage() const;
  CXFA_Node* GetFormNode() const { return m_pFormNode; }
  void SetFormNode(CXFA_Node* pNode);

 protected:
  enum ItemType { kViewItem, kContentItem };
  CXFA_LayoutItem(CXFA_Node* pNode, ItemType type);

 private:
  const ItemType m_ItemType;
  cppgc::Member<CXFA_Node> m_pFormNode;
};

inline CXFA_ViewLayoutItem* ToViewLayoutItem(CXFA_LayoutItem* item) {
  return item ? item->AsViewLayoutItem() : nullptr;
}

inline CXFA_ContentLayoutItem* ToContentLayoutItem(CXFA_LayoutItem* item) {
  return item ? item->AsContentLayoutItem() : nullptr;
}

void XFA_ReleaseLayoutItem(CXFA_LayoutItem* pLayoutItem);

#endif  // XFA_FXFA_LAYOUT_CXFA_LAYOUTITEM_H_
