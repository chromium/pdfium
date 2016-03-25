// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_WIDGETMGRDELEGATE_H_
#define XFA_FWL_CORE_IFWL_WIDGETMGRDELEGATE_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

#define FWL_WGTMGR_DisableThread 0x00000001
#define FWL_WGTMGR_DisableForm 0x00000002

class CFWL_Message;
class CFX_Graphics;
class CFX_Matrix;
class IFWL_Widget;

class IFWL_WidgetMgrDelegate {
 public:
  virtual ~IFWL_WidgetMgrDelegate() {}

  virtual FWL_ERR OnSetCapability(
      uint32_t dwCapability = FWL_WGTMGR_DisableThread) = 0;
  virtual int32_t OnProcessMessageToForm(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR OnDrawWidget(IFWL_Widget* pWidget,
                               CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = nullptr) = 0;
};

#endif  // XFA_FWL_CORE_IFWL_WIDGETMGRDELEGATE_H_
