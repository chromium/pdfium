// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_IFWL_WIDGETMGRDELEGATE_H_
#define XFA_FWL_IFWL_WIDGETMGRDELEGATE_H_

class CFWL_Message;
class CFX_Graphics;
class CFX_Matrix;
class CFWL_Widget;

class CFWL_WidgetMgrDelegate {
 public:
  virtual void OnSetCapability(uint32_t dwCapability) = 0;
  virtual void OnProcessMessageToForm(CFWL_Message* pMessage) = 0;
  virtual void OnDrawWidget(CFWL_Widget* pWidget,
                            CFX_Graphics* pGraphics,
                            const CFX_Matrix* pMatrix) = 0;
};

#endif  // XFA_FWL_IFWL_WIDGETMGRDELEGATE_H_
