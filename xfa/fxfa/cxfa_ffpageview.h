// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFPAGEVIEW_H_
#define XFA_FXFA_CXFA_FFPAGEVIEW_H_

#include <vector>

#include "core/fxcrt/mask.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"
#include "xfa/fxfa/layout/cxfa_traversestrategy_layoutitem.h"
#include "xfa/fxfa/layout/cxfa_viewlayoutitem.h"

class CXFA_FFWidget;
class CXFA_FFDocView;

class CXFA_FFPageView final : public cppgc::GarbageCollected<CXFA_FFPageView> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFPageView();

  void Trace(cppgc::Visitor* visitor) const;

  CXFA_ViewLayoutItem* GetLayoutItem() const { return layout_item_; }
  void SetLayoutItem(CXFA_ViewLayoutItem* pItem) { layout_item_ = pItem; }

  CXFA_FFDocView* GetDocView() const;
  CFX_RectF GetPageViewRect() const;
  CFX_Matrix GetDisplayMatrix(const FX_RECT& rtDisp, int32_t iRotate) const;

  // This always returns a non-null iterator from the gc heap.
  CXFA_FFWidget::IteratorIface* CreateGCedTraverseWidgetIterator(
      Mask<XFA_WidgetStatus> dwWidgetFilter);

 private:
  CXFA_FFPageView(CXFA_FFDocView* pDocView, CXFA_Node* pPageArea);

  cppgc::Member<CXFA_Node> const page_area_;
  cppgc::Member<CXFA_FFDocView> const doc_view_;
  cppgc::Member<CXFA_ViewLayoutItem> layout_item_;
};

class CXFA_FFPageWidgetIterator final : public CXFA_FFWidget::IteratorIface {
  CPPGC_STACK_ALLOCATED();

 public:
  CXFA_FFPageWidgetIterator(CXFA_FFPageView* pPageView,
                            Mask<XFA_WidgetStatus> dwFilter);
  ~CXFA_FFPageWidgetIterator() override;

  // CXFA_FFWidget::IteratorIface:
  CXFA_FFWidget* MoveToFirst() override;
  CXFA_FFWidget* MoveToLast() override;
  CXFA_FFWidget* MoveToNext() override;
  CXFA_FFWidget* MoveToPrevious() override;
  CXFA_FFWidget* GetCurrentWidget() override;
  bool SetCurrentWidget(CXFA_FFWidget* hWidget) override;

 private:
  CXFA_LayoutItemIterator s_iterator_;
  const Mask<XFA_WidgetStatus> filter_;
  const bool ignore_relevant_;
};

class CXFA_FFTabOrderPageWidgetIterator final
    : public cppgc::GarbageCollected<CXFA_FFTabOrderPageWidgetIterator>,
      public CXFA_FFWidget::IteratorIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFTabOrderPageWidgetIterator() override;

  void Trace(cppgc::Visitor* visitor) const;

  // CXFA_FFWidget::IteratorIface:
  CXFA_FFWidget* MoveToFirst() override;
  CXFA_FFWidget* MoveToLast() override;
  CXFA_FFWidget* MoveToNext() override;
  CXFA_FFWidget* MoveToPrevious() override;
  CXFA_FFWidget* GetCurrentWidget() override;
  bool SetCurrentWidget(CXFA_FFWidget* hWidget) override;

 private:
  CXFA_FFTabOrderPageWidgetIterator(CXFA_FFPageView* pPageView,
                                    Mask<XFA_WidgetStatus> dwFilter);

  CXFA_FFWidget* GetTraverseWidget(CXFA_FFWidget* pWidget);
  CXFA_FFWidget* FindWidgetByName(const WideString& wsWidgetName,
                                  CXFA_FFWidget* pRefWidget);
  void CreateTabOrderWidgetArray();
  std::vector<CXFA_ContentLayoutItem*> CreateSpaceOrderLayoutItems();

  cppgc::Member<CXFA_ViewLayoutItem> const page_view_layout_;
  std::vector<cppgc::Member<CXFA_ContentLayoutItem>> tab_order_widget_array_;
  const Mask<XFA_WidgetStatus> filter_;
  int32_t cur_widget_ = -1;
  const bool ignore_relevant_;
};

#endif  // XFA_FXFA_CXFA_FFPAGEVIEW_H_
