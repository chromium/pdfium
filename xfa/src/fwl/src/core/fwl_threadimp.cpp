// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_appimp.h"
IFWL_Thread* IFWL_Thread::Create() {
  return (IFWL_Thread*)new CFWL_Thread;
}
CFWL_Thread::CFWL_Thread() {}
CFWL_Thread::~CFWL_Thread() {}
FWL_ERR CFWL_Thread::Run(FWL_HTHREAD hThread) {
  return FWL_ERR_Succeeded;
}
IFWL_NoteThread* IFWL_NoteThread::Create() {
  return (IFWL_NoteThread*)new CFWL_NoteThread;
}
CFWL_NoteThread::CFWL_NoteThread() : m_hThread(NULL) {
  m_pNoteDriver = new CFWL_NoteDriver;
}
CFWL_NoteThread::~CFWL_NoteThread() {
  if (m_hThread) {
    FWL_StopThread(m_hThread, 0);
  }
  if (m_pNoteDriver) {
    delete m_pNoteDriver;
    m_pNoteDriver = NULL;
  }
}
FWL_ERR CFWL_NoteThread::Run(FWL_HTHREAD hThread) {
  if (!m_pNoteDriver)
    return FWL_ERR_Indefinite;
  FWL_ERR result = m_pNoteDriver->Run();
  return result;
}
IFWL_NoteDriver* CFWL_NoteThread::GetNoteDriver() {
  return (IFWL_NoteDriver*)m_pNoteDriver;
}
extern IFWL_AdapterNative* FWL_GetAdapterNative();
FWL_HTHREAD FWL_StartThread(IFWL_Thread* pThread, FX_BOOL bSuspended) {
  IFWL_AdapterNative* pNative = FWL_GetAdapterNative();
  if (!pNative)
    return NULL;
  IFWL_AdapterThreadMgr* pThreadMgr = pNative->GetThreadMgr();
  if (!pThreadMgr)
    return NULL;
  FWL_HTHREAD hThread = NULL;
  pThreadMgr->Start(pThread, hThread, bSuspended);
  return hThread;
}
FWL_ERR FWL_ResumeThread(FWL_HTHREAD hThread) {
  IFWL_AdapterNative* Native = FWL_GetAdapterNative();
  if (!Native)
    return FWL_ERR_Indefinite;
  IFWL_AdapterThreadMgr* ThreadMgr = Native->GetThreadMgr();
  if (!ThreadMgr)
    return FWL_ERR_Indefinite;
  return ThreadMgr->Resume(hThread);
}
FWL_ERR FWL_SuspendThread(FWL_HTHREAD hThread) {
  IFWL_AdapterNative* Native = FWL_GetAdapterNative();
  if (!Native)
    return FWL_ERR_Indefinite;
  IFWL_AdapterThreadMgr* ThreadMgr = Native->GetThreadMgr();
  if (!ThreadMgr)
    return FWL_ERR_Indefinite;
  return ThreadMgr->Suspend(hThread);
}
FWL_ERR FWL_KillThread(FWL_HTHREAD hThread, int32_t iExitCode) {
  IFWL_AdapterNative* Native = FWL_GetAdapterNative();
  if (!Native)
    return FWL_ERR_Indefinite;
  IFWL_AdapterThreadMgr* ThreadMgr = Native->GetThreadMgr();
  if (!ThreadMgr)
    return FWL_ERR_Indefinite;
  return ThreadMgr->Kill(hThread, iExitCode);
}
FWL_ERR FWL_StopThread(FWL_HTHREAD hThread, int32_t iExitCode) {
  IFWL_AdapterNative* Native = FWL_GetAdapterNative();
  if (!Native)
    return FWL_ERR_Indefinite;
  IFWL_AdapterThreadMgr* ThreadMgr = Native->GetThreadMgr();
  if (!ThreadMgr)
    return FWL_ERR_Indefinite;
  return ThreadMgr->Stop(hThread, iExitCode);
}
FWL_HMUTEX FWL_CreateMutex() {
  return NULL;
}
FWL_ERR FWL_DestroyMutex(FWL_HMUTEX hMutex) {
  return FWL_ERR_Succeeded;
}
FWL_ERR FWL_LockMutex(FWL_HMUTEX hMutex) {
  return FWL_ERR_Succeeded;
}
FWL_ERR FWL_TryLockMutex(FWL_HMUTEX hMutex) {
  return FWL_ERR_Succeeded;
}
FWL_ERR FWL_UnlockMutex(FWL_HMUTEX hMutex) {
  return FWL_ERR_Succeeded;
}
FWL_ERR FWL_IsLockedMutex(FWL_HMUTEX hMutex, FX_BOOL& bLocked) {
  return FWL_ERR_Succeeded;
}
FWL_HSEMAPHORE FWL_CreateSemaphore() {
  return (FWL_HSEMAPHORE)IFWL_AdapterSemaphore::Create();
}
FWL_ERR FWL_DestroySemaphore(FWL_HSEMAPHORE hSemaphore) {
  return ((IFWL_AdapterSemaphore*)hSemaphore)->Destroy();
}
FWL_ERR FWL_WaitSemaphore(FWL_HSEMAPHORE hSemaphore) {
  return ((IFWL_AdapterSemaphore*)hSemaphore)->Wait();
}
FWL_ERR FWL_PostSemaphore(FWL_HSEMAPHORE hSemaphore, int32_t down) {
  return ((IFWL_AdapterSemaphore*)hSemaphore)->Post();
}
FWL_ERR FWL_GetSemaphoreValue(FWL_HSEMAPHORE hSemaphore, int32_t& value) {
  return ((IFWL_AdapterSemaphore*)hSemaphore)->Value((FX_DWORD&)value);
}
FWL_ERR FWL_ResetSemaphore(FWL_HSEMAPHORE hSemaphore, int32_t init) {
  return ((IFWL_AdapterSemaphore*)hSemaphore)->Reset(init);
}
