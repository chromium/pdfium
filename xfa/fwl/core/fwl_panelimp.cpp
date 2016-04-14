// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/fwl_panelimp.h"

#include "xfa/fwl/core/cfwl_widgetimpproperties.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_targetimp.h"
#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/fwl_widgetmgrimp.h"
#include "xfa/fwl/core/ifwl_panel.h"

IFWL_Panel::IFWL_Panel() {}

CFWL_PanelImp::CFWL_PanelImp(const CFWL_WidgetImpProperties& properties,
                             IFWL_Widget* pOuter)
    : CFWL_WidgetImp(properties, pOuter) {}
CFWL_PanelImp::~CFWL_PanelImp() {}
FWL_ERR CFWL_PanelImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_Panel;
  return FWL_ERR_Succeeded;
}
uint32_t CFWL_PanelImp::GetClassID() const {
  return FWL_CLASSHASH_Panel;
}
FWL_ERR CFWL_PanelImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (!bAutoSize)
    rect = m_pProperties->m_rtWidget;

  return FWL_ERR_Succeeded;
}
