// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_CUSTOM_H_
#define XFA_FWL_CORE_IFWL_CUSTOM_H_

#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widget.h"

class IFWL_Proxy;

class IFWL_Custom : public IFWL_Widget {
 public:
  static IFWL_Custom* Create(const CFWL_WidgetImpProperties& properties,
                             IFWL_Widget* pOuter);

  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_Custom();
};

#endif  // XFA_FWL_CORE_IFWL_CUSTOM_H_
