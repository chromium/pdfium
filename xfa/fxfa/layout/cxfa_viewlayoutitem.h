// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_FFPageView;

class CXFA_ViewLayoutItem final : public CXFA_LayoutItem {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CXFA_ViewLayoutItem() override;

  CXFA_FFPageView* GetPageView() const { return m_pFFPageView.get(); }
  CXFA_LayoutProcessor* GetLayout() const;
  int32_t GetPageIndex() const;
  CFX_SizeF GetPageSize() const;
  CXFA_Node* GetMasterPage() const;
  CXFA_Node* GetOldSubform() const { return m_pOldSubform.Get(); }
  void SetOldSubform(CXFA_Node* pSubform) { m_pOldSubform = pSubform; }

 private:
  CXFA_ViewLayoutItem(CXFA_Node* pNode,
                      std::unique_ptr<CXFA_FFPageView> pPageView);

  std::unique_ptr<CXFA_FFPageView> const m_pFFPageView;
  UnownedPtr<CXFA_Node> m_pOldSubform;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
