// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_TIMER_H_
#define FPDFSDK_PWL_CPWL_TIMER_H_

#include "core/fxcrt/timerhandler_iface.h"
#include "core/fxcrt/unowned_ptr.h"

class CPWL_TimerHandler;

class CPWL_Timer {
 public:
  class CallbackIface {
   public:
    virtual ~CallbackIface() = default;
    virtual void OnTimerFired() = 0;
  };

  CPWL_Timer(TimerHandlerIface* pTimerHandler,
             CallbackIface* pCallbackIface,
             int32_t nInterval);
  ~CPWL_Timer();

 private:
  static void TimerProc(int32_t idEvent);

  bool HasValidID() const {
    return m_nTimerID != TimerHandlerIface::kInvalidTimerID;
  }

  const int32_t m_nTimerID;
  UnownedPtr<TimerHandlerIface> const m_pTimerHandler;
  UnownedPtr<CallbackIface> const m_pCallbackIface;
};

#endif  // FPDFSDK_PWL_CPWL_TIMER_H_
