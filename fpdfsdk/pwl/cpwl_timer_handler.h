// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_TIMER_HANDLER_H_
#define FPDFSDK_PWL_CPWL_TIMER_HANDLER_H_

#include <memory>

class CPWL_Timer;
class IPWL_SystemHandler;

class CPWL_TimerHandler {
 public:
  CPWL_TimerHandler();
  virtual ~CPWL_TimerHandler();

  virtual void TimerProc();
  virtual IPWL_SystemHandler* GetSystemHandler() const = 0;

  void BeginTimer(int32_t nElapse);
  void EndTimer();

 private:
  std::unique_ptr<CPWL_Timer> m_pTimer;
};

#endif  // FPDFSDK_PWL_CPWL_TIMER_HANDLER_H_
