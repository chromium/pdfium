// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_timer.h"

#include <map>

#include "core/fxcrt/check.h"

namespace {

using TimerMap = std::map<int32_t, CFX_Timer*>;
TimerMap* g_pwl_timer_map = nullptr;

}  // namespace

// static
void CFX_Timer::InitializeGlobals() {
  CHECK(!g_pwl_timer_map);
  g_pwl_timer_map = new TimerMap();
}

// static
void CFX_Timer::DestroyGlobals() {
  delete g_pwl_timer_map;
  g_pwl_timer_map = nullptr;
}

CFX_Timer::CFX_Timer(HandlerIface* pHandlerIface,
                     CallbackIface* pCallbackIface,
                     int32_t nInterval)
    : handler_iface_(pHandlerIface), callback_iface_(pCallbackIface) {
  DCHECK(callback_iface_);
  if (handler_iface_) {
    timer_id_ = handler_iface_->SetTimer(nInterval, TimerProc);
    if (HasValidID()) {
      (*g_pwl_timer_map)[timer_id_] = this;
    }
  }
}

CFX_Timer::~CFX_Timer() {
  if (HasValidID()) {
    g_pwl_timer_map->erase(timer_id_);
    if (handler_iface_) {
      handler_iface_->KillTimer(timer_id_);
    }
  }
}

// static
void CFX_Timer::TimerProc(int32_t idEvent) {
  auto it = g_pwl_timer_map->find(idEvent);
  if (it != g_pwl_timer_map->end()) {
    it->second->callback_iface_->OnTimerFired();
  }
}
