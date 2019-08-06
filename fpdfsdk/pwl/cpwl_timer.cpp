// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_timer.h"

#include <map>

namespace {

std::map<int32_t, CPWL_Timer*>& GetPWLTimeMap() {
  // Leak the object at shutdown.
  static auto* timeMap = new std::map<int32_t, CPWL_Timer*>;
  return *timeMap;
}

}  // namespace

CPWL_Timer::CPWL_Timer(IPWL_SystemHandler* pSystemHandler,
                       CallbackIface* pCallbackIface,
                       int32_t nInterval)
    : m_nTimerID(pSystemHandler->SetTimer(nInterval, TimerProc)),
      m_pSystemHandler(pSystemHandler),
      m_pCallbackIface(pCallbackIface) {
  ASSERT(m_pCallbackIface);
  if (HasValidID())
    GetPWLTimeMap()[m_nTimerID] = this;
}

CPWL_Timer::~CPWL_Timer() {
  if (HasValidID()) {
    m_pSystemHandler->KillTimer(m_nTimerID);
    GetPWLTimeMap().erase(m_nTimerID);
  }
}

// static
void CPWL_Timer::TimerProc(int32_t idEvent) {
  auto it = GetPWLTimeMap().find(idEvent);
  if (it != GetPWLTimeMap().end())
    it->second->m_pCallbackIface->OnTimerFired();
}
