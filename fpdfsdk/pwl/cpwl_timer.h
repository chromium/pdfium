// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_TIMER_H_
#define FPDFSDK_PWL_CPWL_TIMER_H_

#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class CPWL_TimerHandler;

class CPWL_Timer {
 public:
  class CallbackIface {
   public:
    virtual ~CallbackIface() = default;
    virtual void OnTimerFired() = 0;
  };

  CPWL_Timer(IPWL_SystemHandler* pSystemHandler,
             CallbackIface* pCallbackIface,
             int32_t nInterval);
  ~CPWL_Timer();

 private:
  static void TimerProc(int32_t idEvent);

  bool HasValidID() const {
    return m_nTimerID != IPWL_SystemHandler::kInvalidTimerID;
  }

  const int32_t m_nTimerID;
  UnownedPtr<IPWL_SystemHandler> const m_pSystemHandler;
  UnownedPtr<CallbackIface> const m_pCallbackIface;
};

#endif  // FPDFSDK_PWL_CPWL_TIMER_H_
