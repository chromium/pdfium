// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_TIMERMGR_H
#define _FWL_ADAPTER_TIMERMGR_H
class IFWL_Timer;
class IFWL_AdapterTimerMgr;
class IFWL_AdapterTimerMgr
{
public:
    virtual FWL_ERR		Start(IFWL_Timer *pTimer, FX_DWORD dwElapse, FWL_HTIMER &hTimer, FX_BOOL bImmediately = TRUE) = 0;
    virtual FWL_ERR		Stop(FWL_HTIMER hTimer) = 0;
};
#endif
