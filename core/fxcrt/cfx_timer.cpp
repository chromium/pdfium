// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_timer.h"

#include <map>

#include "third_party/base/check.h"
#include "third_party/base/no_destructor.h"

namespace {

using TimerMap = std::map<int32_t, CFX_Timer*>;
TimerMap& GetPWLTimerMap() {
  static pdfium::base::NoDestructor<TimerMap> timer_map;
  return *timer_map;
}

}  // namespace

CFX_Timer::CFX_Timer(HandlerIface* pHandlerIface,
                     CallbackIface* pCallbackIface,
                     int32_t nInterval)
    : m_pHandlerIface(pHandlerIface), m_pCallbackIface(pCallbackIface) {
  DCHECK(m_pCallbackIface);
  if (m_pHandlerIface) {
    m_nTimerID = m_pHandlerIface->SetTimer(nInterval, TimerProc);
    if (HasValidID())
      GetPWLTimerMap()[m_nTimerID] = this;
  }
}

CFX_Timer::~CFX_Timer() {
  if (HasValidID()) {
    GetPWLTimerMap().erase(m_nTimerID);
    if (m_pHandlerIface)
      m_pHandlerIface->KillTimer(m_nTimerID);
  }
}

// static
void CFX_Timer::TimerProc(int32_t idEvent) {
  auto it = GetPWLTimerMap().find(idEvent);
  if (it != GetPWLTimerMap().end())
    it->second->m_pCallbackIface->OnTimerFired();
}
