// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_FFWidget;

class CXFA_ContentLayoutItem : public CXFA_LayoutItem {
 public:
  explicit CXFA_ContentLayoutItem(CXFA_Node* pNode);
  ~CXFA_ContentLayoutItem() override;

  virtual CXFA_FFWidget* AsFFWidget();

  CXFA_ContentLayoutItem* GetFirst();
  CXFA_ContentLayoutItem* GetLast();
  CXFA_ContentLayoutItem* GetPrev() const { return m_pPrev.Get(); }
  CXFA_ContentLayoutItem* GetNext() const { return m_pNext.Get(); }
  void InsertAfter(CXFA_ContentLayoutItem* pNext);

  CFX_RectF GetRect(bool bRelative) const;
  size_t GetIndex() const;

  CFX_PointF m_sPos;
  CFX_SizeF m_sSize;
  mutable uint32_t m_dwStatus = 0;

 private:
  void RemoveSelf();

  UnownedPtr<CXFA_ContentLayoutItem> m_pPrev;
  UnownedPtr<CXFA_ContentLayoutItem> m_pNext;
};

inline CXFA_FFWidget* ToFFWidget(CXFA_ContentLayoutItem* item) {
  return item ? item->AsFFWidget() : nullptr;
}

#endif  // XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_
