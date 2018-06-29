// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LAYOUTITEM_H_
#define XFA_FXFA_PARSER_CXFA_LAYOUTITEM_H_

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/parser/cxfa_document.h"

class CXFA_ContainerLayoutItem;
class CXFA_ContentLayoutItem;
class CXFA_LayoutProcessor;

class CXFA_LayoutItem {
 public:
  virtual ~CXFA_LayoutItem();

  bool IsContainerLayoutItem() const { return !m_bIsContentLayoutItem; }
  bool IsContentLayoutItem() const { return m_bIsContentLayoutItem; }
  CXFA_ContainerLayoutItem* AsContainerLayoutItem();
  CXFA_ContentLayoutItem* AsContentLayoutItem();

  CXFA_ContainerLayoutItem* GetPage() const;
  CFX_RectF GetRect(bool bRelative) const;

  CXFA_Node* GetFormNode() const { return m_pFormNode.Get(); }
  void SetFormNode(CXFA_Node* pNode) { m_pFormNode = pNode; }

  int32_t GetIndex() const;
  int32_t GetCount() const;

  CXFA_LayoutItem* GetParent() const { return m_pParent; }
  CXFA_LayoutItem* GetFirst();
  const CXFA_LayoutItem* GetLast() const;
  CXFA_LayoutItem* GetPrev() const;
  CXFA_LayoutItem* GetNext() const;

  void AddChild(CXFA_LayoutItem* pChildItem);
  void AddHeadChild(CXFA_LayoutItem* pChildItem);
  void RemoveChild(CXFA_LayoutItem* pChildItem);
  void InsertChild(CXFA_LayoutItem* pBeforeItem, CXFA_LayoutItem* pChildItem);

  CXFA_LayoutItem* m_pParent = nullptr;       // Raw, intra-tree pointer.
  CXFA_LayoutItem* m_pNextSibling = nullptr;  // Raw, intra-tree pointer.
  CXFA_LayoutItem* m_pFirstChild = nullptr;   // Raw, intra-tree pointer.

 protected:
  CXFA_LayoutItem(CXFA_Node* pNode, bool bIsContentLayoutItem);

  bool m_bIsContentLayoutItem;
  UnownedPtr<CXFA_Node> m_pFormNode;
};

void XFA_ReleaseLayoutItem(CXFA_LayoutItem* pLayoutItem);

#endif  // XFA_FXFA_PARSER_CXFA_LAYOUTITEM_H_
