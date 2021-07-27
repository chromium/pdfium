// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_
#define XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_

#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

class CFWL_Widget;
class CFX_RectF;

class CXFA_FWLAdapterWidgetMgr
    : public cppgc::GarbageCollected<CXFA_FWLAdapterWidgetMgr>,
      public CFWL_WidgetMgr::AdapterIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FWLAdapterWidgetMgr() override;

  // CFWL_WidgetMgr::AdapterIface:
  void Trace(cppgc::Visitor* visitor) const override;
  void RepaintWidget(CFWL_Widget* pWidget) override;
  bool GetPopupPos(CFWL_Widget* pWidget,
                   float fMinHeight,
                   float fMaxHeight,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect) override;

 private:
  CXFA_FWLAdapterWidgetMgr();
};

#endif  // XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_
