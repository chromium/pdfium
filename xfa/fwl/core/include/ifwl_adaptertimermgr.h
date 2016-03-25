// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_INCLUDE_IFWL_ADAPTERTIMERMGR_H_
#define XFA_FWL_CORE_INCLUDE_IFWL_ADAPTERTIMERMGR_H_

#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_timer.h"

class IFWL_Timer;

class IFWL_AdapterTimerMgr {
 public:
  virtual ~IFWL_AdapterTimerMgr() {}
  virtual FWL_ERR Start(IFWL_Timer* pTimer,
                        uint32_t dwElapse,
                        FWL_HTIMER& hTimer,
                        FX_BOOL bImmediately = TRUE) = 0;
  virtual FWL_ERR Stop(FWL_HTIMER hTimer) = 0;
};

#endif  // XFA_FWL_CORE_INCLUDE_IFWL_ADAPTERTIMERMGR_H_
