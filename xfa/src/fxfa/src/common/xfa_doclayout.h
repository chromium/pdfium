// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCLAYOUT_H_
#define _XFA_DOCLAYOUT_H_
#define _XFA_LAYOUTITEM_ProcessCACHE_

class IXFA_LayoutPage;

class CXFA_LayoutItem {
 public:
  CXFA_LayoutItem(CXFA_Node* pNode, FX_BOOL bIsContentLayoutItem);
  virtual ~CXFA_LayoutItem();

  IXFA_LayoutPage* GetPage() const;
  CXFA_Node* GetFormNode() const;
  void GetRect(CFX_RectF& rtLayout, FX_BOOL bRelative = FALSE) const;
  int32_t GetIndex() const;
  int32_t GetCount() const;
  CXFA_LayoutItem* GetParent() const;
  const CXFA_LayoutItem* GetFirst() const;
  CXFA_LayoutItem* GetFirst();
  const CXFA_LayoutItem* GetLast() const;
  CXFA_LayoutItem* GetLast();
  CXFA_LayoutItem* GetPrev() const;
  CXFA_LayoutItem* GetNext() const;

  FX_BOOL IsContentLayoutItem() { return m_bIsContentLayoutItem; }
  void AddChild(CXFA_LayoutItem* pChildItem);
  void AddHeadChild(CXFA_LayoutItem* pChildItem);
  void RemoveChild(CXFA_LayoutItem* pChildItem);
  void InsertChild(CXFA_LayoutItem* pBeforeItem, CXFA_LayoutItem* pChildItem);

 public:
  CXFA_Node* m_pFormNode;
  CXFA_LayoutItem* m_pParent;
  CXFA_LayoutItem* m_pNextSibling;
  CXFA_LayoutItem* m_pFirstChild;
  FX_BOOL m_bIsContentLayoutItem;
};
class CXFA_ContainerLayoutItem : public CXFA_LayoutItem {
 public:
  CXFA_ContainerLayoutItem(CXFA_Node* pNode);

 public:
  CXFA_Node* m_pOldSubform;
};
#define XFA_WIDGETSTATUS_Access 0x80000000
#define XFA_WIDGETSTATUS_Disabled 0x40000000
#define XFA_WIDGETSTATUS_RectCached 0x20000000
#define XFA_WIDGETSTATUS_ButtonDown 0x10000000
#define XFA_WIDGETSTATUS_Highlight 0x08000000
#define XFA_WIDGETSTATUS_TextEditValueChanged 0x04000000
class CXFA_ContentLayoutItem : public CXFA_LayoutItem {
 public:
  CXFA_ContentLayoutItem(CXFA_Node* pNode);
  virtual ~CXFA_ContentLayoutItem();

 public:
  CXFA_ContentLayoutItem* m_pPrev;
  CXFA_ContentLayoutItem* m_pNext;
  CFX_PointF m_sPos;
  CFX_SizeF m_sSize;
  FX_DWORD m_dwStatus;
};
class CXFA_TraverseStrategy_LayoutItem {
 public:
  static inline CXFA_LayoutItem* GetFirstChild(CXFA_LayoutItem* pLayoutItem) {
    return pLayoutItem->m_pFirstChild;
  }
  static inline CXFA_LayoutItem* GetNextSibling(CXFA_LayoutItem* pLayoutItem) {
    return pLayoutItem->m_pNextSibling;
  }
  static inline CXFA_LayoutItem* GetParent(CXFA_LayoutItem* pLayoutItem) {
    return pLayoutItem->m_pParent;
  }
};
class IXFA_LayoutPage {
 public:
  IXFA_DocLayout* GetLayout() const;
  int32_t GetPageIndex() const;
  void GetPageSize(CFX_SizeF& size);
  CXFA_Node* GetMasterPage() const;
};
class IXFA_DocLayout {
 public:
  virtual ~IXFA_DocLayout() {}
  virtual CXFA_Document* GetDocument() const = 0;
  virtual int32_t StartLayout(FX_BOOL bForceRestart = FALSE) = 0;
  virtual int32_t DoLayout(IFX_Pause* pPause = NULL) = 0;
  virtual FX_BOOL IncrementLayout() = 0;
  virtual int32_t CountPages() const = 0;
  virtual IXFA_LayoutPage* GetPage(int32_t index) const = 0;
  virtual CXFA_LayoutItem* GetLayoutItem(CXFA_Node* pFormItem) = 0;
};
#endif
