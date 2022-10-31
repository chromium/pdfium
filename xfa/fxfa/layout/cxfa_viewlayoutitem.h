// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_

#include "core/fxcrt/fx_coordinates.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_FFPageView;
class CXFA_LayoutProcessor;

class CXFA_ViewLayoutItem final : public CXFA_LayoutItem {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ViewLayoutItem() override;

  void Trace(cppgc::Visitor* visitor) const override;

  CXFA_FFPageView* GetPageView() const { return m_pFFPageView; }
  CXFA_LayoutProcessor* GetLayout() const;
  int32_t GetPageIndex() const;
  CFX_SizeF GetPageSize() const;
  CXFA_Node* GetMasterPage() const;
  CXFA_Node* GetOldSubform() const { return m_pOldSubform; }
  void SetOldSubform(CXFA_Node* pSubform);

 private:
  CXFA_ViewLayoutItem(CXFA_Node* pNode, CXFA_FFPageView* pPageView);

  cppgc::Member<CXFA_FFPageView> const m_pFFPageView;
  cppgc::Member<CXFA_Node> m_pOldSubform;
};

#endif  // XFA_FXFA_LAYOUT_CXFA_VIEWLAYOUTITEM_H_
