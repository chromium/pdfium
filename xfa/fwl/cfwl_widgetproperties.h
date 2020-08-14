// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_WIDGETPROPERTIES_H_
#define XFA_FWL_CFWL_WIDGETPROPERTIES_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/fwl_widgetdef.h"

class CFWL_WidgetProperties {
 public:
  CFWL_WidgetProperties();
  CFWL_WidgetProperties(const CFWL_WidgetProperties& that);
  ~CFWL_WidgetProperties();

  uint32_t m_dwStyles = FWL_WGTSTYLE_Child;
  uint32_t m_dwStyleExes = 0;
  uint32_t m_dwStates = 0;
};

#endif  // XFA_FWL_CFWL_WIDGETPROPERTIES_H_
