// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/global_timer.h"

#include <map>

#include "core/fxcrt/timerhandler_iface.h"
#include "fxjs/cjs_app.h"
#include "third_party/base/no_destructor.h"

namespace {

using TimerMap = std::map<int32_t, GlobalTimer*>;
TimerMap& GetGlobalTimerMap() {
  static pdfium::base::NoDestructor<TimerMap> timer_map;
  return *timer_map;
}

}  // namespace

GlobalTimer::GlobalTimer(CJS_App* pObj,
                         CJS_Runtime* pRuntime,
                         Type nType,
                         const WideString& script,
                         uint32_t dwElapse,
                         uint32_t dwTimeOut)
    : m_nType(nType),
      m_nTimerID(pRuntime->GetTimerHandler()->SetTimer(dwElapse, Trigger)),
      m_dwTimeOut(dwTimeOut),
      m_swJScript(script),
      m_pRuntime(pRuntime),
      m_pEmbedApp(pObj) {
  if (HasValidID())
    GetGlobalTimerMap()[m_nTimerID] = this;
}

GlobalTimer::~GlobalTimer() {
  if (!HasValidID())
    return;

  if (m_pRuntime && m_pRuntime->GetTimerHandler())
    m_pRuntime->GetTimerHandler()->KillTimer(m_nTimerID);

  GetGlobalTimerMap().erase(m_nTimerID);
}

// static
void GlobalTimer::Trigger(int32_t nTimerID) {
  auto it = GetGlobalTimerMap().find(nTimerID);
  if (it == GetGlobalTimerMap().end())
    return;

  GlobalTimer* pTimer = it->second;
  if (pTimer->m_bProcessing)
    return;

  pTimer->m_bProcessing = true;
  if (pTimer->m_pEmbedApp)
    pTimer->m_pEmbedApp->TimerProc(pTimer);

  // Timer proc may have destroyed timer, find it again.
  it = GetGlobalTimerMap().find(nTimerID);
  if (it == GetGlobalTimerMap().end())
    return;

  pTimer = it->second;
  pTimer->m_bProcessing = false;
  if (pTimer->IsOneShot())
    pTimer->m_pEmbedApp->CancelProc(pTimer);
}

// static
void GlobalTimer::Cancel(int32_t nTimerID) {
  auto it = GetGlobalTimerMap().find(nTimerID);
  if (it == GetGlobalTimerMap().end())
    return;

  GlobalTimer* pTimer = it->second;
  pTimer->m_pEmbedApp->CancelProc(pTimer);
}

bool GlobalTimer::HasValidID() const {
  return m_nTimerID != TimerHandlerIface::kInvalidTimerID;
}
