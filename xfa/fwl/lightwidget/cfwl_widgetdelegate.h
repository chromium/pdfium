// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_LIGHTWIDGET_CFWL_WIDGETDELEGATE_H_
#define XFA_FWL_LIGHTWIDGET_CFWL_WIDGETDELEGATE_H_

#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widgetdelegate.h"

class CFWL_Event;
class CFWL_Message;
class CFX_Graphics;
class CFX_Matrix;

class CFWL_WidgetDelegate : public IFWL_WidgetDelegate {
 public:
  CFWL_WidgetDelegate();
  virtual ~CFWL_WidgetDelegate();

  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
};

#endif  // XFA_FWL_LIGHTWIDGET_CFWL_WIDGETDELEGATE_H_
