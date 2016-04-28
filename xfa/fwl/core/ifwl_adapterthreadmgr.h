// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_ADAPTERTHREADMGR_H_
#define XFA_FWL_CORE_IFWL_ADAPTERTHREADMGR_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fwl/core/fwl_error.h"

class IFWL_App;

class IFWL_AdapterThreadMgr {
 public:
  virtual ~IFWL_AdapterThreadMgr() {}

  virtual IFWL_App* GetCurrentThread() = 0;
};

#endif  // XFA_FWL_CORE_IFWL_ADAPTERTHREADMGR_H_
