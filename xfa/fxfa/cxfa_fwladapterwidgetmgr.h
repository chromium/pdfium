// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_
#define XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

class CFWL_Widget;

class CXFA_FWLAdapterWidgetMgr : public CFWL_WidgetMgr::AdapterIface {
 public:
  CXFA_FWLAdapterWidgetMgr();
  ~CXFA_FWLAdapterWidgetMgr() override;

  void RepaintWidget(CFWL_Widget* pWidget) override;
  bool GetPopupPos(CFWL_Widget* pWidget,
                   float fMinHeight,
                   float fMaxHeight,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect) override;
};

#endif  // XFA_FXFA_CXFA_FWLADAPTERWIDGETMGR_H_
