// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_

#include <memory>

#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_FFPageView;

class CXFA_ViewLayoutItem : public CXFA_LayoutItem {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ~CXFA_ViewLayoutItem() override;

  CXFA_FFPageView* GetPageView() const { return m_pFFPageView.get(); }
  CXFA_LayoutProcessor* GetLayout() const;
  int32_t GetPageIndex() const;
  CFX_SizeF GetPageSize() const;
  CXFA_Node* GetMasterPage() const;

  UnownedPtr<CXFA_Node> m_pOldSubform;

 private:
  CXFA_ViewLayoutItem(CXFA_Node* pNode,
                      std::unique_ptr<CXFA_FFPageView> pPageView);

  std::unique_ptr<CXFA_FFPageView> const m_pFFPageView;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
