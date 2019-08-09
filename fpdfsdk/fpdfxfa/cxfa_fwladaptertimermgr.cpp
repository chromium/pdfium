// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cxfa_fwladaptertimermgr.h"

#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

std::vector<std::unique_ptr<CFWL_TimerInfo>>* g_TimerArray = nullptr;

class CFWL_FWLAdapterTimerInfo final : public CFWL_TimerInfo {
 public:
  CFWL_FWLAdapterTimerInfo(IFWL_AdapterTimerMgr* mgr,
                           int32_t event,
                           CFWL_Timer* timer)
      : CFWL_TimerInfo(mgr), idEvent(event), pTimer(timer) {}

  int32_t idEvent;
  CFWL_Timer* pTimer;
};

void TimerProc(int32_t idEvent) {
  if (!g_TimerArray)
    return;

  for (const auto& info : *g_TimerArray) {
    auto* pInfo = static_cast<CFWL_FWLAdapterTimerInfo*>(info.get());
    if (pInfo->idEvent == idEvent) {
      pInfo->pTimer->OnTimerFired();
      break;
    }
  }
}

}  // namespace

CXFA_FWLAdapterTimerMgr::CXFA_FWLAdapterTimerMgr(
    TimerHandlerIface* pTimerHandler)
    : m_pTimerHandler(pTimerHandler) {}

CXFA_FWLAdapterTimerMgr::~CXFA_FWLAdapterTimerMgr() = default;

CFWL_TimerInfo* CXFA_FWLAdapterTimerMgr::Start(CFWL_Timer* pTimer,
                                               uint32_t dwElapse) {
  if (!g_TimerArray)
    g_TimerArray = new std::vector<std::unique_ptr<CFWL_TimerInfo>>;

  if (!m_pTimerHandler)
    return nullptr;

  int32_t id_event = m_pTimerHandler->SetTimer(dwElapse, TimerProc);
  g_TimerArray->push_back(
      pdfium::MakeUnique<CFWL_FWLAdapterTimerInfo>(this, id_event, pTimer));
  return g_TimerArray->back().get();
}

void CXFA_FWLAdapterTimerMgr::Stop(CFWL_TimerInfo* pTimerInfo) {
  if (!pTimerInfo || !m_pTimerHandler)
    return;

  CFWL_FWLAdapterTimerInfo* pInfo =
      static_cast<CFWL_FWLAdapterTimerInfo*>(pTimerInfo);
  m_pTimerHandler->KillTimer(pInfo->idEvent);
  if (!g_TimerArray)
    return;

  pdfium::FakeUniquePtr<CFWL_TimerInfo> fake(pInfo);
  auto it = std::find(g_TimerArray->begin(), g_TimerArray->end(), fake);
  if (it != g_TimerArray->end())
    g_TimerArray->erase(it);
}
