// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_TIMER_H_
#define FPDFSDK_PDFWINDOW_CPWL_TIMER_H_

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_basic.h"

class CFX_SystemHandler;
class CPWL_TimerHandler;

class CPWL_Timer {
 public:
  CPWL_Timer(CPWL_TimerHandler* pAttached, CFX_SystemHandler* pSystemHandler);
  virtual ~CPWL_Timer();

  static void TimerProc(int32_t idEvent);

  int32_t SetPWLTimer(int32_t nElapse);
  void KillPWLTimer();

 private:
  int32_t m_nTimerID;
  CFX_UnownedPtr<CPWL_TimerHandler> m_pAttached;
  CFX_UnownedPtr<CFX_SystemHandler> m_pSystemHandler;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_TIMER_H_
