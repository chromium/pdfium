// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_CUSTOMPANEL_H_
#define XFA_FWL_CORE_IFWL_CUSTOMPANEL_H_

#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/ifwl_widget.h"

class IFWL_Proxy;

class IFWL_CustomPanel : public IFWL_Widget {
 public:
  static IFWL_CustomPanel* Create(CFWL_WidgetImpProperties& properties,
                                  IFWL_Widget* pOuter);

  IFWL_Content* GetContent();
  FWL_ERR SetContent(IFWL_Content* pContent);
  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_CustomPanel();
};

#endif  // XFA_FWL_CORE_IFWL_CUSTOMPANEL_H_
