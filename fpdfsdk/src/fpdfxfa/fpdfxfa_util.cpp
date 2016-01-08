// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_mgr.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"

CFX_PtrArray CXFA_FWLAdapterTimerMgr::ms_timerArray;

FWL_ERR CXFA_FWLAdapterTimerMgr::Start(IFWL_Timer* pTimer,
                                       FX_DWORD dwElapse,
                                       FWL_HTIMER& hTimer,
                                       FX_BOOL bImmediately /* = TRUE */) {
  if (m_pEnv) {
    uint32_t uIDEvent = m_pEnv->FFI_SetTimer(dwElapse, TimerProc);
    CFWL_TimerInfo* pInfo = new CFWL_TimerInfo;
    pInfo->uIDEvent = uIDEvent;
    pInfo->pTimer = pTimer;
    ms_timerArray.Add(pInfo);

    hTimer = (FWL_HTIMER)pInfo;
    return FWL_ERR_Succeeded;
  }

  return FWL_ERR_Indefinite;
}

FWL_ERR CXFA_FWLAdapterTimerMgr::Stop(FWL_HTIMER hTimer) {
  if (!hTimer)
    return FWL_ERR_Indefinite;

  if (m_pEnv) {
    CFWL_TimerInfo* pInfo = (CFWL_TimerInfo*)hTimer;

    m_pEnv->FFI_KillTimer(pInfo->uIDEvent);

    int32_t index = ms_timerArray.Find(pInfo);
    if (index >= 0) {
      ms_timerArray.RemoveAt(index);
      delete pInfo;
    }
    return FWL_ERR_Succeeded;
  }

  return FWL_ERR_Indefinite;
}

void CXFA_FWLAdapterTimerMgr::TimerProc(int32_t idEvent) {
  CFWL_TimerInfo* pInfo = NULL;
  int32_t iCount = CXFA_FWLAdapterTimerMgr::ms_timerArray.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_TimerInfo* pTemp =
        (CFWL_TimerInfo*)CXFA_FWLAdapterTimerMgr::ms_timerArray.GetAt(i);
    if (pTemp->uIDEvent == idEvent) {
      pInfo = pTemp;
      break;
    }
  }
  if (pInfo) {
    pInfo->pTimer->Run((FWL_HTIMER)pInfo);
  }
}
