// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FWL_ADAPTER_FWL_ADAPTERTHREADMGR_H_
#define XFA_INCLUDE_FWL_ADAPTER_FWL_ADAPTERTHREADMGR_H_

#include "core/include/fxcrt/fx_system.h"
#include "xfa/include/fwl/core/fwl_error.h"
#include "xfa/include/fwl/core/fwl_thread.h"

class IFWL_AdapterThreadMgr {
 public:
  virtual ~IFWL_AdapterThreadMgr() {}
  virtual FWL_ERR Start(IFWL_Thread* pThread,
                        FWL_HTHREAD& hThread,
                        FX_BOOL bSuspended = FALSE) = 0;
  virtual FWL_ERR Resume(FWL_HTHREAD hThread) = 0;
  virtual FWL_ERR Suspend(FWL_HTHREAD hThread) = 0;
  virtual FWL_ERR Kill(FWL_HTHREAD hThread, int32_t iExitCode) = 0;
  virtual FWL_ERR Stop(FWL_HTHREAD hThread, int32_t iExitCode) = 0;
  virtual IFWL_Thread* GetCurrentThread() = 0;
};
#endif  // XFA_INCLUDE_FWL_ADAPTER_FWL_ADAPTERTHREADMGR_H_
