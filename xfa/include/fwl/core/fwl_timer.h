// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_TIMER_H
#define _FWL_TIMER_H
class IFWL_Timer;
typedef struct _FWL_HTIMER {
    FX_LPVOID pData;
} *FWL_HTIMER;
class IFWL_Timer
{
public:
    virtual FX_INT32 Run(FWL_HTIMER hTimer) = 0;
};
FWL_HTIMER FWL_StartTimer(IFWL_Timer *pTimer, FX_DWORD dwElapse, FX_BOOL bImmediately = TRUE);
FX_INT32 FWL_StopTimer(FWL_HTIMER hTimer);
#endif
