// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_PROXY_H_
#define XFA_FWL_CORE_IFWL_PROXY_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

class IFWL_Proxy {
 public:
  virtual ~IFWL_Proxy() {}
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) = 0;
  virtual FWL_ERR Update() = 0;
};

#endif  // XFA_FWL_CORE_IFWL_PROXY_H_
