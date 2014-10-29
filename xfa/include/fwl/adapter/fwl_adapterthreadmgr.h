// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_THREAD_H
#define _FWL_ADAPTER_THREAD_H
class IFWL_Thread;
class IFWL_AdapterThreadMgr;
class IFWL_AdapterMutex;
class IFWL_AdapterSemaphore;
class IFWL_AdapterThreadMgr
{
public:
    virtual FWL_ERR		Start(IFWL_Thread *pThread, FWL_HTHREAD &hThread, FX_BOOL bSuspended = FALSE) = 0;
    virtual FWL_ERR		Resume(FWL_HTHREAD hThread) = 0;
    virtual FWL_ERR		Suspend(FWL_HTHREAD hThread) = 0;
    virtual FWL_ERR		Kill(FWL_HTHREAD hThread, FX_INT32 iExitCode) = 0;
    virtual FWL_ERR		Stop(FWL_HTHREAD hThread, FX_INT32 iExitCode) = 0;
    virtual IFWL_Thread* GetCurrentThread() = 0;
};
class IFWL_AdapterSemaphore
{
public:
    static IFWL_AdapterSemaphore * Create();
    virtual FWL_ERR Destroy() = 0;
    virtual FWL_ERR Wait() const = 0;
    virtual FWL_ERR Post() = 0;
    virtual FWL_ERR Value(FX_DWORD &val) const = 0;
    virtual FWL_ERR Reset(FX_INT32 init) = 0;
};
#endif
