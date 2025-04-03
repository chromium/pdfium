// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_
#define XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/mask.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/layout/cxfa_layoutitem.h"

class CXFA_FFWidget;

class CXFA_ContentLayoutItem final : public CXFA_LayoutItem {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ContentLayoutItem() override;

  void Trace(cppgc::Visitor* visitor) const override;

  CXFA_FFWidget* GetFFWidget() { return ffwidget_; }

  CXFA_ContentLayoutItem* GetFirst();
  CXFA_ContentLayoutItem* GetLast();
  CXFA_ContentLayoutItem* GetPrev() const { return prev_; }
  CXFA_ContentLayoutItem* GetNext() const { return next_; }
  void InsertAfter(CXFA_ContentLayoutItem* pNext);

  CFX_RectF GetRelativeRect() const;
  CFX_RectF GetAbsoluteRect() const;
  size_t GetIndex() const;

  void SetStatusBits(Mask<XFA_WidgetStatus> val) { status_ |= val; }
  void ClearStatusBits(Mask<XFA_WidgetStatus> val) { status_ &= ~val; }

  // TRUE if all (not any) bits set in |val| are set in |status_|.
  bool TestStatusBits(Mask<XFA_WidgetStatus> val) const {
    return status_.TestAll(val);
  }

  CFX_PointF s_pos_;
  CFX_SizeF s_size_;

 private:
  CXFA_ContentLayoutItem(CXFA_Node* pNode, CXFA_FFWidget* pFFWidget);
  void RemoveSelf();

  mutable Mask<XFA_WidgetStatus> status_;
  cppgc::Member<CXFA_ContentLayoutItem> prev_;
  cppgc::Member<CXFA_ContentLayoutItem> next_;
  cppgc::Member<CXFA_FFWidget> const ffwidget_;
};

inline CXFA_FFWidget* GetFFWidget(CXFA_ContentLayoutItem* item) {
  return item ? item->GetFFWidget() : nullptr;
}

#endif  // XFA_FXFA_LAYOUT_CXFA_CONTENTLAYOUTITEM_H_
