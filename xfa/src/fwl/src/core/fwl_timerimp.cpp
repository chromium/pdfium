// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_appimp.h"
FWL_HTIMER FWL_StartTimer(IFWL_Timer *pTimer, FX_DWORD dwElapse, FX_BOOL bImmediately )
{
    IFWL_AdapterNative *pAdapterNative = FWL_GetAdapterNative();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapterNative, NULL);
    IFWL_AdapterTimerMgr *pAdapterTimerMgr = pAdapterNative->GetTimerMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapterTimerMgr, NULL);
    FWL_HTIMER hTimer = NULL;
    pAdapterTimerMgr->Start(pTimer, dwElapse, hTimer, bImmediately);
    return hTimer;
}
FX_INT32 FWL_StopTimer(FWL_HTIMER hTimer)
{
    IFWL_AdapterNative *pAdapterNative = FWL_GetAdapterNative();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapterNative, FWL_ERR_Indefinite);
    IFWL_AdapterTimerMgr *pAdapterTimerMgr = pAdapterNative->GetTimerMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapterTimerMgr, FWL_ERR_Indefinite);
    return pAdapterTimerMgr->Stop(hTimer);
}
