// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/global_timer.h"

#include <map>

#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "fxjs/cjs_app.h"

namespace {

using TimerMap = std::map<int32_t, GlobalTimer*>;
TimerMap* g_global_timer_map = nullptr;

}  // namespace

// static
void GlobalTimer::InitializeGlobals() {
  CHECK(!g_global_timer_map);
  g_global_timer_map = new TimerMap();
}

// static
void GlobalTimer::DestroyGlobals() {
  delete g_global_timer_map;
  g_global_timer_map = nullptr;
}

GlobalTimer::GlobalTimer(CJS_App* pObj,
                         CJS_Runtime* pRuntime,
                         Type nType,
                         const WideString& script,
                         uint32_t dwElapse,
                         uint32_t dwTimeOut)
    : type_(nType),
      timer_id_(pRuntime->GetTimerHandler()->SetTimer(dwElapse, Trigger)),
      time_out_(dwTimeOut),
      jscript_(script),
      runtime_(pRuntime),
      embed_app_(pObj) {
  if (HasValidID()) {
    DCHECK(!pdfium::Contains((*g_global_timer_map), timer_id_));
    (*g_global_timer_map)[timer_id_] = this;
  }
}

GlobalTimer::~GlobalTimer() {
  if (!HasValidID()) {
    return;
  }

  if (runtime_ && runtime_->GetTimerHandler()) {
    runtime_->GetTimerHandler()->KillTimer(timer_id_);
  }

  DCHECK(pdfium::Contains((*g_global_timer_map), timer_id_));
  g_global_timer_map->erase(timer_id_);
}

// static
void GlobalTimer::Trigger(int32_t nTimerID) {
  auto it = g_global_timer_map->find(nTimerID);
  if (it == g_global_timer_map->end()) {
    return;
  }

  GlobalTimer* pTimer = it->second;
  if (pTimer->processing_) {
    return;
  }

  pTimer->processing_ = true;
  if (pTimer->embed_app_) {
    pTimer->embed_app_->TimerProc(pTimer);
  }

  // Timer proc may have destroyed timer, find it again.
  it = g_global_timer_map->find(nTimerID);
  if (it == g_global_timer_map->end()) {
    return;
  }

  pTimer = it->second;
  pTimer->processing_ = false;
  if (pTimer->IsOneShot()) {
    pTimer->embed_app_->CancelProc(pTimer);
  }
}

// static
void GlobalTimer::Cancel(int32_t nTimerID) {
  auto it = g_global_timer_map->find(nTimerID);
  if (it == g_global_timer_map->end()) {
    return;
  }

  GlobalTimer* pTimer = it->second;
  pTimer->embed_app_->CancelProc(pTimer);
}

bool GlobalTimer::HasValidID() const {
  return timer_id_ != CFX_Timer::HandlerIface::kInvalidTimerID;
}
