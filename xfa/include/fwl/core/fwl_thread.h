// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_THREAD_H
#define _FWL_THREAD_H
class IFWL_NoteDriver;
class IFWL_Thread;
class IFWL_NoteThread;
typedef struct _FWL_HTHREAD {
    FX_LPVOID pData;
} *FWL_HTHREAD;
class IFWL_Thread
{
public:
    static IFWL_Thread * Create();
protected:
    virtual ~IFWL_Thread() {}
public:
    virtual void	Release() = 0;
    virtual FWL_ERR Run(FWL_HTHREAD hThread) = 0;
};
FWL_HTHREAD FWL_StartThread(IFWL_Thread *pThread, FX_BOOL bSuspended = FALSE);
FWL_ERR FWL_ResumeThread(FWL_HTHREAD hThread);
FWL_ERR FWL_SuspendThread(FWL_HTHREAD hThread);
FWL_ERR FWL_KillThread(FWL_HTHREAD hThread, FX_INT32 iExitCode);
FWL_ERR FWL_StopThread(FWL_HTHREAD hThread, FX_INT32 iExitCode);
FWL_ERR FWL_Sleep(FX_DWORD dwMilliseconds);
class IFWL_NoteThread : public IFWL_Thread
{
public:
    static IFWL_NoteThread* Create();
    virtual FWL_ERR Run(FWL_HTHREAD hThread) = 0;
    virtual IFWL_NoteDriver* GetNoteDriver() = 0;
};
typedef struct _FWL_HMUTEX {
    FX_LPVOID pData;
} *FWL_HMUTEX;
FWL_HMUTEX	FWL_CreateMutex();
FWL_ERR		FWL_DestroyMutex(FWL_HMUTEX hMutex);
FWL_ERR		FWL_LockMutex(FWL_HMUTEX hMutex);
FWL_ERR		FWL_TryLockMutex(FWL_HMUTEX hMutex);
FWL_ERR		FWL_UnlockMutex(FWL_HMUTEX hMutex);
FWL_ERR		FWL_IsLockedMutex(FWL_HMUTEX hMutex, FX_BOOL &bLocked);
typedef struct _FWL_HSEMAPHORE {
    FX_LPVOID pData;
} *FWL_HSEMAPHORE;
FWL_HSEMAPHORE	FWL_CreateSemaphore();
FWL_ERR		FWL_DestroySemaphore(FWL_HSEMAPHORE hSemaphore);
FWL_ERR		FWL_WaitSemaphore(FWL_HSEMAPHORE hSemaphore);
FWL_ERR		FWL_PostSemaphore(FWL_HSEMAPHORE hSemaphore, FX_INT32 down = 1);
FWL_ERR		FWL_GetSemaphoreValue(FWL_HSEMAPHORE hSemaphore, FX_INT32 &value);
FWL_ERR		FWL_ResetSemaphore(FWL_HSEMAPHORE hSemaphore, FX_INT32 init);
#endif
