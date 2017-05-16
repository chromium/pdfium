// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_TIMERINFO_H_
#define XFA_FWL_CFWL_TIMERINFO_H_

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_system.h"

class IFWL_AdapterTimerMgr;

class CFWL_TimerInfo {
 public:
  explicit CFWL_TimerInfo(IFWL_AdapterTimerMgr* mgr);
  virtual ~CFWL_TimerInfo();

  void StopTimer();

 private:
  CFX_UnownedPtr<IFWL_AdapterTimerMgr> m_pMgr;
};

#endif  // XFA_FWL_CFWL_TIMERINFO_H_
