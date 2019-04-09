// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_

#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_ViewLayoutItem : public CXFA_LayoutItem {
 public:
  explicit CXFA_ViewLayoutItem(CXFA_Node* pNode);
  ~CXFA_ViewLayoutItem() override;

  CXFA_LayoutProcessor* GetLayout() const;
  int32_t GetPageIndex() const;
  CFX_SizeF GetPageSize() const;
  CXFA_Node* GetMasterPage() const;

  UnownedPtr<CXFA_Node> m_pOldSubform;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
