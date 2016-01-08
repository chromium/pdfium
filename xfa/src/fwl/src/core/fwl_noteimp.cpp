// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetmgrimp.h"
#include "xfa/src/fwl/src/core/include/fwl_panelimp.h"
#include "xfa/src/fwl/src/core/include/fwl_formimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_appimp.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_tooltipctrlimp.h"
CFWL_NoteLoop::CFWL_NoteLoop(CFWL_WidgetImp* pForm)
    : m_pForm(pForm), m_bContinueModal(TRUE) {}
FX_BOOL CFWL_NoteLoop::PreProcessMessage(CFWL_Message* pMessage) {
  if (!m_pForm) {
    return FALSE;
  }
  return TranslateAccelerator(pMessage);
}
FWL_ERR CFWL_NoteLoop::Idle(int32_t count) {
#if (_FX_OS_ == _FX_WIN32_DESKTOP_)
  if (count <= 0)
#endif
  {
    CFWL_EvtIdle ev;
    IFWL_App* pApp = FWL_GetApp();
    if (!pApp)
      return FWL_ERR_Indefinite;
    IFWL_NoteDriver* pDriver = pApp->GetNoteDriver();
    if (!pDriver)
      return FWL_ERR_Indefinite;
    pDriver->SendNote(&ev);
  }
  return FWL_ERR_Indefinite;
}
CFWL_WidgetImp* CFWL_NoteLoop::GetForm() {
  return m_pForm;
}
FX_BOOL CFWL_NoteLoop::ContinueModal() {
  return m_bContinueModal;
}
FWL_ERR CFWL_NoteLoop::EndModalLoop() {
  m_bContinueModal = FALSE;
#if (_FX_OS_ == _FX_MACOSX_)
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  IFWL_AdapterWidgetMgr* adapterWidgetMgr = pWidgetMgr->GetAdapterWidgetMgr();
  adapterWidgetMgr->EndLoop();
#endif
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_NoteLoop::TranslateAccelerator(CFWL_Message* pMessage) {
  if (pMessage->GetClassID() != FWL_MSGHASH_Key) {
    return FALSE;
  }
  CFWL_MsgKey* pMsgKey = static_cast<CFWL_MsgKey*>(pMessage);
  if (pMsgKey->m_dwCmd != FWL_MSGKEYCMD_KeyDown) {
    return FALSE;
  }
  CFX_MapAccelerators& accel =
      static_cast<CFWL_FormImp*>(m_pForm)->GetAccelerator();
  FX_POSITION pos = accel.GetStartPosition();
  if (!pos) {
    return FALSE;
  }
  FX_DWORD vrKey, rValue;
  while (pos) {
    accel.GetNextAssoc(pos, vrKey, rValue);
    FX_DWORD dwFlags = (vrKey & 0xFF00) >> 8;
    FX_DWORD m_dwKeyCode = vrKey & 0x00FF;
    if (pMsgKey->m_dwFlags == dwFlags && pMsgKey->m_dwKeyCode == m_dwKeyCode) {
      GenerateCommondEvent(rValue);
      return TRUE;
    }
  }
  return FALSE;
}
FWL_ERR CFWL_NoteLoop::SetMainForm(CFWL_WidgetImp* pForm) {
  m_pForm = pForm;
  return FWL_ERR_Succeeded;
}
void CFWL_NoteLoop::GenerateCommondEvent(FX_DWORD dwCommand) {
  CFWL_EvtMenuCommand ev;
  ev.m_iCommand = dwCommand;
  IFWL_NoteThread* pThread = m_pForm->GetOwnerThread();
  if (!pThread)
    return;
  IFWL_NoteDriver* pDriver = pThread->GetNoteDriver();
  if (!pDriver)
    return;
  pDriver->SendNote(&ev);
}
CFWL_NoteDriver::CFWL_NoteDriver()
    : m_sendEventCalled(0),
      m_maxSize(500),
      m_bFullScreen(FALSE),
      m_pHover(nullptr),
      m_pFocus(nullptr),
      m_pGrab(nullptr),
      m_hook(nullptr) {
  m_pNoteLoop = new CFWL_NoteLoop;
  PushNoteLoop(m_pNoteLoop);
}
CFWL_NoteDriver::~CFWL_NoteDriver() {
  delete m_pNoteLoop;
  ClearInvalidEventTargets(TRUE);
}
FX_BOOL CFWL_NoteDriver::SendNote(CFWL_Note* pNote) {
  if (pNote->IsEvent()) {
    int32_t iCount = m_eventTargets.GetCount();
    if (iCount < 1) {
      return TRUE;
    }
    if (FWL_EVTHASH_Mouse == static_cast<CFWL_Event*>(pNote)->GetClassID()) {
      CFWL_EvtMouse* pMouse = static_cast<CFWL_EvtMouse*>(pNote);
      if (FWL_MSGMOUSECMD_MouseHover == pMouse->m_dwCmd) {
        if (m_pNoteLoop->GetForm() &&
            CFWL_ToolTipContainer::getInstance()->ProcessEnter(
                pMouse, m_pNoteLoop->GetForm()->GetInterface())) {
        }
      } else if (FWL_MSGMOUSECMD_MouseLeave == pMouse->m_dwCmd) {
        if (CFWL_ToolTipContainer::getInstance()->ProcessLeave(pMouse)) {
        }
      } else if ((FWL_MSGMOUSECMD_LButtonDown <= pMouse->m_dwCmd) &&
                 (FWL_MSGMOUSECMD_MButtonDblClk >= pMouse->m_dwCmd)) {
        if (CFWL_ToolTipContainer::getInstance()->ProcessLeave(pMouse)) {
        }
      }
    }
    m_sendEventCalled++;
    FX_POSITION pos = m_eventTargets.GetStartPosition();
    while (pos) {
      void* key = NULL;
      CFWL_EventTarget* pEventTarget;
      m_eventTargets.GetNextAssoc(pos, key, (void*&)pEventTarget);
      if (pEventTarget && !pEventTarget->IsInvalid()) {
        pEventTarget->ProcessEvent(static_cast<CFWL_Event*>(pNote));
      }
    }
    m_sendEventCalled--;
  } else {
    if (!pNote->m_pDstTarget)
      return FALSE;
    IFWL_WidgetDelegate* pDelegate = pNote->m_pDstTarget->SetDelegate(NULL);
    if (pDelegate) {
      pDelegate->OnProcessMessage(static_cast<CFWL_Message*>(pNote));
    }
  }
  return TRUE;
}
extern void FWL_PostMessageToMainRoop(CFWL_Message* pMessage);
FX_BOOL CFWL_NoteDriver::PostMessage(CFWL_Message* pMessage) {
  FWL_PostMessageToMainRoop(pMessage);
  return TRUE;
}
#define FWL_NoteDriver_EventKey 1100
FWL_ERR CFWL_NoteDriver::RegisterEventTarget(IFWL_Widget* pListener,
                                             IFWL_Widget* pEventSource,
                                             FX_DWORD dwFilter) {
  FX_DWORD dwkey = (FX_DWORD)(uintptr_t)pListener->GetPrivateData(
      (void*)(uintptr_t)FWL_NoteDriver_EventKey);
  if (dwkey == 0) {
    void* random = FX_Random_MT_Start(0);
    dwkey = rand();
    FX_Random_MT_Close(random);
    pListener->SetPrivateData((void*)(uintptr_t)FWL_NoteDriver_EventKey,
                              (void*)(uintptr_t)dwkey, NULL);
  }
  CFWL_EventTarget* value = NULL;
  if (!m_eventTargets.Lookup((void*)(uintptr_t)dwkey, (void*&)value)) {
    value = new CFWL_EventTarget(this, pListener);
    m_eventTargets.SetAt((void*)(uintptr_t)dwkey, value);
  }
  value->SetEventSource(pEventSource, dwFilter);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_NoteDriver::UnregisterEventTarget(IFWL_Widget* pListener) {
  FX_DWORD dwkey = (FX_DWORD)(uintptr_t)pListener->GetPrivateData(
      (void*)(uintptr_t)FWL_NoteDriver_EventKey);
  if (dwkey == 0) {
    return FWL_ERR_Indefinite;
  }
  CFWL_EventTarget* value = NULL;
  if (m_eventTargets.Lookup((void*)(uintptr_t)dwkey, (void*&)value)) {
    value->FlagInvalid();
  }
  return FWL_ERR_Succeeded;
}
void CFWL_NoteDriver::ClearEventTargets(FX_BOOL bRemoveAll) {
  ClearInvalidEventTargets(bRemoveAll);
}
int32_t CFWL_NoteDriver::GetQueueMaxSize() const {
  return m_maxSize;
}
FWL_ERR CFWL_NoteDriver::SetQueueMaxSize(const int32_t size) {
  m_maxSize = size;
  return FWL_ERR_Succeeded;
}
IFWL_NoteThread* CFWL_NoteDriver::GetOwnerThread() const {
  return FWL_GetApp();
}
FWL_ERR CFWL_NoteDriver::PushNoteLoop(IFWL_NoteLoop* pNoteLoop) {
  m_noteLoopQueue.Add(pNoteLoop);
  return FWL_ERR_Succeeded;
}
IFWL_NoteLoop* CFWL_NoteDriver::PopNoteLoop() {
  int32_t pos = m_noteLoopQueue.GetSize();
  if (pos <= 0)
    return NULL;
  IFWL_NoteLoop* p =
      static_cast<IFWL_NoteLoop*>(m_noteLoopQueue.GetAt(pos - 1));
  m_noteLoopQueue.RemoveAt(pos - 1);
  return p;
}
FX_BOOL CFWL_NoteDriver::SetFocus(IFWL_Widget* pFocus, FX_BOOL bNotify) {
  if (m_pFocus == pFocus) {
    return TRUE;
  }
  IFWL_Widget* pPrev = m_pFocus;
  m_pFocus = pFocus;
  if (pPrev) {
    CFWL_MsgKillFocus ms;
    ms.m_pDstTarget = pPrev;
    ms.m_pSrcTarget = pPrev;
    if (bNotify) {
      ms.m_dwExtend = 1;
    }
    IFWL_WidgetDelegate* pDelegate = pPrev->SetDelegate(NULL);
    if (pDelegate) {
      pDelegate->OnProcessMessage(&ms);
    }
  }
  if (pFocus) {
    IFWL_Widget* pWidget =
        FWL_GetWidgetMgr()->GetWidget(pFocus, FWL_WGTRELATION_SystemForm);
    CFWL_FormImp* pForm =
        pWidget ? static_cast<CFWL_FormImp*>(pWidget->GetImpl()) : nullptr;
    if (pForm) {
      CFWL_WidgetImp* pNewFocus =
          static_cast<CFWL_WidgetImp*>(pFocus->GetImpl());
      pForm->SetSubFocus(pNewFocus);
    }
    CFWL_MsgSetFocus ms;
    ms.m_pDstTarget = pFocus;
    if (bNotify) {
      ms.m_dwExtend = 1;
    }
    IFWL_WidgetDelegate* pDelegate = pFocus->SetDelegate(NULL);
    if (pDelegate) {
      pDelegate->OnProcessMessage(&ms);
    }
  }
  return TRUE;
}
FWL_ERR CFWL_NoteDriver::Run() {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return FWL_ERR_Indefinite;
#if (_FX_OS_ == _FX_MACOSX_)
  IFWL_AdapterWidgetMgr* adapterWidgetMgr = pWidgetMgr->GetAdapterWidgetMgr();
  CFWL_NoteLoop* pTopLoop = GetTopLoop();
  if (pTopLoop) {
    CFWL_WidgetImp* formImp = pTopLoop->GetForm();
    if (formImp) {
      IFWL_Widget* pForm = formImp->GetInterface();
      adapterWidgetMgr->RunLoop(pForm);
    }
  }
#elif(_FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_)
  FX_BOOL bIdle = TRUE;
  int32_t iIdleCount = 0;
  CFWL_NoteLoop* pTopLoop = NULL;
  for (;;) {
    pTopLoop = GetTopLoop();
    if (!pTopLoop || !pTopLoop->ContinueModal()) {
      break;
    }
    if (UnqueueMessage(pTopLoop)) {
      continue;
    }
    while (bIdle && !(pWidgetMgr->CheckMessage_Native())) {
      if (FWL_ERR_Indefinite == pTopLoop->Idle(iIdleCount++)) {
        bIdle = FALSE;
      }
    }
    do {
      if (FWL_ERR_Indefinite == pWidgetMgr->DispatchMessage_Native()) {
        break;
      }
      if (pWidgetMgr->IsIdleMessage_Native()) {
        bIdle = TRUE;
        iIdleCount = 0;
      }
    } while (pWidgetMgr->CheckMessage_Native());
  }
#elif(_FX_OS_ == _FX_LINUX_DESKTOP_)
  CFWL_NoteLoop* pTopLoop = NULL;
  for (;;) {
    pTopLoop = GetTopLoop();
    if (!pTopLoop || !pTopLoop->ContinueModal()) {
      break;
    }
    if (UnqueueMessage(pTopLoop)) {
      continue;
    }
    if (pWidgetMgr->CheckMessage_Native()) {
      pWidgetMgr->DispatchMessage_Native();
    }
  }
#endif
  return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_NoteDriver::GetFocus() {
  return m_pFocus;
}
IFWL_Widget* CFWL_NoteDriver::GetHover() {
  return m_pHover;
}
void CFWL_NoteDriver::SetHover(IFWL_Widget* pHover) {
  m_pHover = pHover;
}
void CFWL_NoteDriver::SetGrab(IFWL_Widget* pGrab, FX_BOOL bSet) {
  m_pGrab = bSet ? pGrab : NULL;
}
void CFWL_NoteDriver::NotifyTargetHide(IFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget) {
    m_pFocus = NULL;
  }
  if (m_pHover == pNoteTarget) {
    m_pHover = NULL;
  }
  if (m_pGrab == pNoteTarget) {
    m_pGrab = NULL;
  }
}
void CFWL_NoteDriver::NotifyTargetDestroy(IFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget) {
    m_pFocus = NULL;
  }
  if (m_pHover == pNoteTarget) {
    m_pHover = NULL;
  }
  if (m_pGrab == pNoteTarget) {
    m_pGrab = NULL;
  }
  UnregisterEventTarget(pNoteTarget);
  int32_t count = m_forms.GetSize();
  for (int32_t nIndex = 0; nIndex < count; nIndex++) {
    CFWL_FormImp* pForm = static_cast<CFWL_FormImp*>(m_forms[nIndex]);
    if (!pForm) {
      continue;
    }
    CFWL_WidgetImp* pSubFocus = pForm->GetSubFocus();
    if (!pSubFocus)
      return;
    if (pSubFocus && pSubFocus->GetInterface() == pNoteTarget) {
      pForm->SetSubFocus(NULL);
    }
  }
}
void CFWL_NoteDriver::NotifyFullScreenMode(IFWL_Widget* pNoteTarget,
                                           FX_BOOL bFullScreen) {
  m_bFullScreen = bFullScreen;
}
FWL_ERR CFWL_NoteDriver::RegisterForm(CFWL_WidgetImp* pForm) {
  if (!pForm)
    return FWL_ERR_Indefinite;
  if (m_forms.Find(pForm) >= 0) {
    return FWL_ERR_Indefinite;
  }
  m_forms.Add(pForm);
  if (m_forms.GetSize() == 1) {
    CFWL_NoteLoop* pLoop =
        static_cast<CFWL_NoteLoop*>(m_noteLoopQueue.GetAt(0));
    if (!pLoop)
      return FWL_ERR_Indefinite;
    pLoop->SetMainForm(pForm);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_NoteDriver::UnRegisterForm(CFWL_WidgetImp* pForm) {
  if (!pForm)
    return FWL_ERR_Indefinite;
  int32_t nIndex = m_forms.Find(pForm);
  if (nIndex < 0) {
    return FWL_ERR_Indefinite;
  }
  m_forms.RemoveAt(nIndex);
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_NoteDriver::QueueMessage(CFWL_Message* pMessage) {
  pMessage->Retain();
  m_noteQueue.Add(pMessage);
  return TRUE;
}
FX_BOOL CFWL_NoteDriver::UnqueueMessage(CFWL_NoteLoop* pNoteLoop) {
  if (m_noteQueue.GetSize() < 1) {
    return FALSE;
  }
  CFWL_Message* pMessage = static_cast<CFWL_Message*>(m_noteQueue[0]);
  m_noteQueue.RemoveAt(0);
  if (!IsValidMessage(pMessage)) {
    pMessage->Release();
    return TRUE;
  }
  FX_BOOL bHookMessage = FALSE;
  if (m_hook) {
    bHookMessage = (*m_hook)(pMessage, m_hookInfo);
  }
  if (!bHookMessage && !pNoteLoop->PreProcessMessage(pMessage)) {
    ProcessMessage(pMessage);
  }
  pMessage->Release();
  return TRUE;
}
CFWL_NoteLoop* CFWL_NoteDriver::GetTopLoop() {
  int32_t size = m_noteLoopQueue.GetSize();
  if (size <= 0)
    return NULL;
  return static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[size - 1]);
}
int32_t CFWL_NoteDriver::CountLoop() {
  return m_noteLoopQueue.GetSize();
}
void CFWL_NoteDriver::SetHook(FWLMessageHookCallback callback, void* info) {
  m_hook = callback;
  m_hookInfo = info;
}
FX_BOOL CFWL_NoteDriver::ProcessMessage(CFWL_Message* pMessage) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  IFWL_Widget* pMessageForm = pWidgetMgr->IsFormDisabled()
                                  ? pMessage->m_pDstTarget
                                  : GetMessageForm(pMessage->m_pDstTarget);
  if (!pMessageForm)
    return FALSE;
  if (DispatchMessage(pMessage, pMessageForm)) {
    if (pMessage->GetClassID() == FWL_MSGHASH_Mouse) {
      MouseSecondary(static_cast<CFWL_MsgMouse*>(pMessage));
    }
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CFWL_NoteDriver::DispatchMessage(CFWL_Message* pMessage,
                                         IFWL_Widget* pMessageForm) {
  FX_BOOL bRet = FALSE;
  switch (pMessage->GetClassID()) {
    case FWL_MSGHASH_Activate: {
      bRet = DoActivate(static_cast<CFWL_MsgActivate*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_Deactivate: {
      bRet = DoDeactivate(static_cast<CFWL_MsgDeactivate*>(pMessage),
                          pMessageForm);
      break;
    }
    case FWL_MSGHASH_SetFocus: {
      bRet = DoSetFocus(static_cast<CFWL_MsgSetFocus*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_KillFocus: {
      bRet =
          DoKillFocus(static_cast<CFWL_MsgKillFocus*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_Key: {
      bRet = DoKey(static_cast<CFWL_MsgKey*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_Mouse: {
      bRet = DoMouse(static_cast<CFWL_MsgMouse*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_MouseWheel: {
      bRet = DoWheel(static_cast<CFWL_MsgMouseWheel*>(pMessage), pMessageForm);
      break;
    }
    case FWL_MSGHASH_Size: {
      bRet = DoSize(static_cast<CFWL_MsgSize*>(pMessage));
      break;
    }
    case FWL_MSGHASH_Cursor: {
      bRet = TRUE;
      break;
    }
    case FWL_MSGHASH_WindowMove: {
      bRet = DoWindowMove(static_cast<CFWL_MsgWindowMove*>(pMessage),
                          pMessageForm);
      break;
    }
    case FWL_MSGHASH_DropFiles: {
      bRet =
          DoDragFiles(static_cast<CFWL_MsgDropFiles*>(pMessage), pMessageForm);
      break;
    }
    default: {
      bRet = TRUE;
      break;
    }
  }
  if (bRet) {
    IFWL_WidgetDelegate* pDelegate = pMessage->m_pDstTarget->SetDelegate(NULL);
    if (pDelegate) {
      pDelegate->OnProcessMessage(pMessage);
    }
  }
  return bRet;
}
FX_BOOL CFWL_NoteDriver::DoActivate(CFWL_MsgActivate* pMsg,
                                    IFWL_Widget* pMessageForm) {
  if (m_bFullScreen) {
    return FALSE;
  }
  pMsg->m_pDstTarget = pMessageForm;
  return (pMsg->m_pDstTarget)->GetStates() & FWL_WGTSTATE_Deactivated;
}
FX_BOOL CFWL_NoteDriver::DoDeactivate(CFWL_MsgDeactivate* pMsg,
                                      IFWL_Widget* pMessageForm) {
  if (m_bFullScreen) {
    return FALSE;
  }
  int32_t iTrackLoop = m_noteLoopQueue.GetSize();
  if (iTrackLoop <= 0)
    return FALSE;
  if (iTrackLoop == 1) {
    if (pMessageForm->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"))) {
      return FALSE;
    }
    if (pMsg->m_pSrcTarget &&
        pMsg->m_pSrcTarget->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"))) {
      return FALSE;
    }
    if (pMsg->m_pSrcTarget && pMsg->m_pSrcTarget->GetClassID() == 1111984755) {
      return FALSE;
    }
    return TRUE;
  }
  IFWL_Widget* pDst = pMsg->m_pDstTarget;
  if (!pDst)
    return FALSE;
#if (_FX_OS_ == _FX_MACOSX_)
  if (pDst == pMessageForm && pDst->IsInstance(L"FWL_FORMPROXY")) {
    return TRUE;
  }
#endif
  return pDst != pMessageForm &&
         !pDst->IsInstance(FX_WSTRC(L"FWL_FORMPROXY")) &&
         !pMessageForm->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"));
}
FX_BOOL CFWL_NoteDriver::DoSetFocus(CFWL_MsgSetFocus* pMsg,
                                    IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (pWidgetMgr->IsFormDisabled()) {
    m_pFocus = pMsg->m_pDstTarget;
    return TRUE;
  } else {
    IFWL_Widget* pWidget = pMsg->m_pDstTarget;
    CFWL_FormImp* pForm =
        pWidget ? static_cast<CFWL_FormImp*>(pWidget->GetImpl()) : nullptr;
    if (pForm) {
      CFWL_WidgetImp* pSubFocus = pForm->GetSubFocus();
      if (pSubFocus && ((pSubFocus->GetStates() & FWL_WGTSTATE_Focused) == 0)) {
        pMsg->m_pDstTarget = pSubFocus->GetInterface();
        if (m_pFocus != pMsg->m_pDstTarget) {
          m_pFocus = pMsg->m_pDstTarget;
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}
FX_BOOL CFWL_NoteDriver::DoKillFocus(CFWL_MsgKillFocus* pMsg,
                                     IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (pWidgetMgr->IsFormDisabled()) {
    if (m_pFocus == pMsg->m_pDstTarget) {
      m_pFocus = NULL;
    }
    return TRUE;
  }
  IFWL_Widget* pWidget = pMsg->m_pDstTarget;
  CFWL_FormImp* pForm =
      pWidget ? static_cast<CFWL_FormImp*>(pWidget->GetImpl()) : nullptr;
  if (pForm) {
    CFWL_WidgetImp* pSubFocus = pForm->GetSubFocus();
    if (pSubFocus && (pSubFocus->GetStates() & FWL_WGTSTATE_Focused)) {
      pMsg->m_pDstTarget = pSubFocus->GetInterface();
      if (m_pFocus == pMsg->m_pDstTarget) {
        m_pFocus = NULL;
        return TRUE;
      }
    }
  }
  return FALSE;
}
FX_BOOL CFWL_NoteDriver::DoKey(CFWL_MsgKey* pMsg, IFWL_Widget* pMessageForm) {
#if (_FX_OS_ != _FX_MACOSX_)
  if (pMsg->m_dwCmd == FWL_MSGKEYCMD_KeyDown &&
      pMsg->m_dwKeyCode == FWL_VKEY_Tab) {
    CFWL_WidgetMgr* pWidgetMgr =
        static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
    IFWL_Widget* pForm = GetMessageForm(pMsg->m_pDstTarget);
    IFWL_Widget* pFocus = m_pFocus;
    if (m_pFocus) {
      if (pWidgetMgr->GetWidget(m_pFocus, FWL_WGTRELATION_SystemForm) !=
          pForm) {
        pFocus = NULL;
      }
    }
    FX_BOOL bFind = FALSE;
    IFWL_Widget* pNextTabStop = pWidgetMgr->nextTab(pForm, pFocus, bFind);
    if (!pNextTabStop) {
      bFind = FALSE;
      pNextTabStop = pWidgetMgr->nextTab(pForm, NULL, bFind);
    }
    if (pNextTabStop == pFocus) {
      return TRUE;
    }
    if (pNextTabStop) {
      SetFocus(pNextTabStop);
    }
    return TRUE;
  }
#endif
  if (!m_pFocus) {
    if (pMsg->m_dwCmd == FWL_MSGKEYCMD_KeyDown &&
        pMsg->m_dwKeyCode == FWL_VKEY_Return) {
      CFWL_WidgetMgr* pWidgetMgr =
          static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
      IFWL_Widget* defButton = pWidgetMgr->GetDefaultButton(pMessageForm);
      if (defButton) {
        pMsg->m_pDstTarget = defButton;
        return TRUE;
      }
    }
    return FALSE;
  }
  pMsg->m_pDstTarget = m_pFocus;
  return TRUE;
}
FX_BOOL CFWL_NoteDriver::DoMouse(CFWL_MsgMouse* pMsg,
                                 IFWL_Widget* pMessageForm) {
  if (pMsg->m_dwCmd == FWL_MSGMOUSECMD_MouseLeave ||
      pMsg->m_dwCmd == FWL_MSGMOUSECMD_MouseHover ||
      pMsg->m_dwCmd == FWL_MSGMOUSECMD_MouseEnter) {
    return pMsg->m_pDstTarget != NULL;
  }
  if (pMsg->m_pDstTarget != pMessageForm) {
    pMsg->m_pDstTarget->TransformTo(pMessageForm, pMsg->m_fx, pMsg->m_fy);
  }
  if (!DoMouseEx(pMsg, pMessageForm)) {
    pMsg->m_pDstTarget = pMessageForm;
  }
  return TRUE;
}
FX_BOOL CFWL_NoteDriver::DoWheel(CFWL_MsgMouseWheel* pMsg,
                                 IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return FALSE;
  IFWL_Widget* pDst =
      pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->m_fx, pMsg->m_fy);
  if (!pDst)
    return FALSE;
  while (pDst && pDst->GetClassID() == FWL_CLASSHASH_Grid) {
    pDst = pDst->GetParent();
  }
  pMessageForm->TransformTo(pDst, pMsg->m_fx, pMsg->m_fy);
  pMsg->m_pDstTarget = pDst;
  return TRUE;
}
FX_BOOL CFWL_NoteDriver::DoSize(CFWL_MsgSize* pMsg) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return FALSE;
  pWidgetMgr->NotifySizeChanged(pMsg->m_pDstTarget, (FX_FLOAT)pMsg->m_iWidth,
                                (FX_FLOAT)pMsg->m_iHeight);
  return TRUE;
}
FX_BOOL CFWL_NoteDriver::DoWindowMove(CFWL_MsgWindowMove* pMsg,
                                      IFWL_Widget* pMessageForm) {
  return pMsg->m_pDstTarget == pMessageForm;
}
FX_BOOL CFWL_NoteDriver::DoDragFiles(CFWL_MsgDropFiles* pMsg,
                                     IFWL_Widget* pMessageForm) {
  return pMsg->m_pDstTarget == pMessageForm;
}
FX_BOOL CFWL_NoteDriver::DoMouseEx(CFWL_MsgMouse* pMsg,
                                   IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
  if (!pWidgetMgr)
    return FALSE;
  IFWL_Widget* pTarget = NULL;
  if (m_pGrab)
    pTarget = m_pGrab;
  if (!pTarget) {
    pTarget =
        pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->m_fx, pMsg->m_fy);
    while (pTarget && pTarget->GetClassID() == FWL_CLASSHASH_Grid) {
      pTarget = pTarget->GetParent();
    }
  }
  if (pTarget) {
    if (pMessageForm != pTarget) {
      pMessageForm->TransformTo(pTarget, pMsg->m_fx, pMsg->m_fy);
    }
  }
  if (!pTarget)
    return FALSE;
  pMsg->m_pDstTarget = pTarget;
  return TRUE;
}
void CFWL_NoteDriver::MouseSecondary(CFWL_MsgMouse* pMsg) {
  IFWL_Widget* pTarget = pMsg->m_pDstTarget;
  if (pTarget == m_pHover) {
    return;
  }
  if (m_pHover) {
    CFWL_MsgMouse msLeave;
    msLeave.m_pDstTarget = m_pHover;
    msLeave.m_fx = pMsg->m_fx;
    msLeave.m_fy = pMsg->m_fy;
    pTarget->TransformTo(m_pHover, msLeave.m_fx, msLeave.m_fy);
    msLeave.m_dwFlags = 0;
    msLeave.m_dwCmd = FWL_MSGMOUSECMD_MouseLeave;
    DispatchMessage(&msLeave, NULL);
  }
  if (pTarget->GetClassID() == FWL_CLASSHASH_Form) {
    m_pHover = NULL;
    return;
  }
  m_pHover = pTarget;
  CFWL_MsgMouse msHover;
  msHover.m_pDstTarget = pTarget;
  msHover.m_fx = pMsg->m_fx;
  msHover.m_fy = pMsg->m_fy;
  msHover.m_dwFlags = 0;
  msHover.m_dwCmd = FWL_MSGMOUSECMD_MouseHover;
  DispatchMessage(&msHover, NULL);
}
FX_BOOL CFWL_NoteDriver::IsValidMessage(CFWL_Message* pMessage) {
  if (pMessage->GetClassID() == FWL_MSGHASH_Post) {
    return TRUE;
  }
  int32_t iCount = m_noteLoopQueue.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_NoteLoop* pNoteLoop = static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[i]);
    CFWL_WidgetImp* pForm = pNoteLoop->GetForm();
    if (pForm && (pForm->GetInterface() == pMessage->m_pDstTarget)) {
      return TRUE;
    }
  }
  iCount = m_forms.GetSize();
  for (int32_t j = 0; j < iCount; j++) {
    CFWL_FormImp* pForm = static_cast<CFWL_FormImp*>(m_forms[j]);
    if (pForm->GetInterface() == pMessage->m_pDstTarget) {
      return TRUE;
    }
  }
  return FALSE;
}
IFWL_Widget* CFWL_NoteDriver::GetMessageForm(IFWL_Widget* pDstTarget) {
  int32_t iTrackLoop = m_noteLoopQueue.GetSize();
  if (iTrackLoop <= 0)
    return NULL;
  IFWL_Widget* pMessageForm = NULL;
  if (iTrackLoop > 1) {
    CFWL_NoteLoop* pNootLoop =
        static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[iTrackLoop - 1]);
    pMessageForm = pNootLoop->GetForm()->GetInterface();
  } else {
    pMessageForm = (m_forms.Find(pDstTarget) < 0) ? NULL : pDstTarget;
  }
  if (!pMessageForm && pDstTarget) {
    CFWL_WidgetMgr* pWidgetMgr =
        static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr());
    if (!pWidgetMgr)
      return NULL;
    pMessageForm =
        pWidgetMgr->GetWidget(pDstTarget, FWL_WGTRELATION_SystemForm);
  }
  return pMessageForm;
}
void CFWL_NoteDriver::ClearInvalidEventTargets(FX_BOOL bRemoveAll) {
  FX_POSITION pos = m_eventTargets.GetStartPosition();
  while (pos) {
    void* key = NULL;
    CFWL_EventTarget* pEventTarget = NULL;
    m_eventTargets.GetNextAssoc(pos, key, (void*&)pEventTarget);
    if (pEventTarget && (bRemoveAll || pEventTarget->IsInvalid())) {
      m_eventTargets.RemoveKey(key);
      delete pEventTarget;
    }
  }
}
class CFWL_CoreToopTipDP : public IFWL_ToolTipDP {
 public:
  FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
  int32_t GetInitialDelay(IFWL_Widget* pWidget);
  int32_t GetAutoPopDelay(IFWL_Widget* pWidget);
  CFX_DIBitmap* GetToolTipIcon(IFWL_Widget* pWidget);
  CFX_SizeF GetToolTipIconSize(IFWL_Widget* pWidget);
  CFX_RectF GetAnchor();
  CFWL_CoreToopTipDP();

  CFX_WideString m_wsCaption;
  int32_t m_nInitDelayTime;
  int32_t m_nAutoPopDelayTime;
  CFX_RectF m_fAnchor;
};
CFWL_CoreToopTipDP::CFWL_CoreToopTipDP() {
  m_nInitDelayTime = 500;
  m_nAutoPopDelayTime = 50000;
  m_fAnchor.Set(0.0, 0.0, 0.0, 0.0);
}
FWL_ERR CFWL_CoreToopTipDP::GetCaption(IFWL_Widget* pWidget,
                                       CFX_WideString& wsCaption) {
  wsCaption = m_wsCaption;
  return FWL_ERR_Succeeded;
}
int32_t CFWL_CoreToopTipDP::GetInitialDelay(IFWL_Widget* pWidget) {
  return m_nInitDelayTime;
}
int32_t CFWL_CoreToopTipDP::GetAutoPopDelay(IFWL_Widget* pWidget) {
  return m_nAutoPopDelayTime;
}
CFX_DIBitmap* CFWL_CoreToopTipDP::GetToolTipIcon(IFWL_Widget* pWidget) {
  return NULL;
}
CFX_SizeF CFWL_CoreToopTipDP::GetToolTipIconSize(IFWL_Widget* pWidget) {
  CFX_SizeF sz;
  sz.Set(0, 0);
  return sz;
}
CFX_RectF CFWL_CoreToopTipDP::GetAnchor() {
  return m_fAnchor;
}
CFWL_EventTarget::~CFWL_EventTarget() {
  m_eventSources.RemoveAll();
}
int32_t CFWL_EventTarget::SetEventSource(IFWL_Widget* pSource,
                                         FX_DWORD dwFilter) {
  if (pSource) {
    m_eventSources.SetAt(pSource, dwFilter);
    return m_eventSources.GetCount();
  }
  return 1;
}
FX_BOOL CFWL_EventTarget::ProcessEvent(CFWL_Event* pEvent) {
  IFWL_WidgetDelegate* pDelegate = m_pListener->SetDelegate(NULL);
  if (!pDelegate)
    return FALSE;
  if (m_eventSources.GetCount() == 0) {
    pDelegate->OnProcessEvent(pEvent);
    return TRUE;
  }
  FX_POSITION pos = m_eventSources.GetStartPosition();
  while (pos) {
    IFWL_Widget* pSource = NULL;
    FX_DWORD dwFilter = 0;
    m_eventSources.GetNextAssoc(pos, (void*&)pSource, dwFilter);
    if (pSource == pEvent->m_pSrcTarget ||
        pEvent->GetClassID() == FWL_EVTHASH_Idle) {
      if (IsFilterEvent(pEvent, dwFilter)) {
        pDelegate->OnProcessEvent(pEvent);
        return TRUE;
      }
    }
  }
  return FALSE;
}
FX_BOOL CFWL_EventTarget::IsFilterEvent(CFWL_Event* pEvent, FX_DWORD dwFilter) {
  if (dwFilter == FWL_EVENT_ALL_MASK) {
    return TRUE;
  }
  FX_BOOL bRet = FALSE;
  switch (pEvent->GetClassID()) {
    case FWL_EVTHASH_Mouse: {
      bRet = dwFilter & FWL_EVENT_MOUSE_MASK;
      break;
    }
    case FWL_EVTHASH_MouseWheel: {
      bRet = dwFilter & FWL_EVENT_MOUSEWHEEL_MASK;
      break;
    }
    case FWL_EVTHASH_Key: {
      bRet = dwFilter & FWL_EVENT_KEY_MASK;
      break;
    }
    case FWL_EVTHASH_SetFocus:
    case FWL_EVTHASH_KillFocus: {
      bRet = dwFilter & FWL_EVENT_FOCUSCHANGED_MASK;
      break;
    }
    case FWL_EVTHASH_Draw: {
      bRet = dwFilter & FWL_EVENT_DRAW_MASK;
      break;
    }
    case FWL_EVTHASH_Close: {
      bRet = dwFilter & FWL_EVENT_CLOSE_MASK;
      break;
    }
    case FWL_EVTHASH_SizeChanged: {
      bRet = dwFilter & FWL_EVENT_SIZECHANGED_MASK;
      break;
    }
    case FWL_EVTHASH_Idle: {
      bRet = dwFilter & FWL_EVENT_IDLE_MASK;
      break;
    }
    default: {
      bRet = dwFilter & FWL_EVENT_CONTROL_MASK;
      break;
    }
  }
  return bRet;
}

CFWL_ToolTipContainer* CFWL_ToolTipContainer::s_pInstance = NULL;

CFWL_ToolTipContainer::CFWL_ToolTipContainer()
    : pCurTarget(NULL), m_pToolTipImp(NULL) {
  m_ToolTipDp = new CFWL_CoreToopTipDP;
  m_ToolTipDp->m_nInitDelayTime = 0;
  m_ToolTipDp->m_nAutoPopDelayTime = 2000;
}
CFWL_ToolTipContainer::~CFWL_ToolTipContainer() {
  if (m_pToolTipImp) {
    IFWL_ToolTip* pToolTip =
        static_cast<IFWL_ToolTip*>(m_pToolTipImp->GetInterface());
    pToolTip->Finalize();
    delete pToolTip;
  }
  delete m_ToolTipDp;
}
// static
CFWL_ToolTipContainer* CFWL_ToolTipContainer::getInstance() {
  if (!s_pInstance) {
    s_pInstance = new CFWL_ToolTipContainer;
  }
  return s_pInstance;
}
// static
void CFWL_ToolTipContainer::DeleteInstance() {
  if (s_pInstance) {
    delete s_pInstance;
    s_pInstance = NULL;
  }
}
FX_ERR CFWL_ToolTipContainer::AddToolTipTarget(IFWL_ToolTipTarget* pTarget) {
  if (m_arrWidget.Find((void*)pTarget) < 0) {
    m_arrWidget.Add((void*)pTarget);
    return FWL_ERR_Succeeded;
  }
  return FWL_ERR_Indefinite;
}
FX_ERR CFWL_ToolTipContainer::RemoveToolTipTarget(IFWL_ToolTipTarget* pTarget) {
  int index = m_arrWidget.Find((void*)pTarget);
  if (index >= 0) {
    m_arrWidget.RemoveAt(index);
    return FWL_ERR_Succeeded;
  }
  return FWL_ERR_Indefinite;
}
FX_BOOL CFWL_ToolTipContainer::HasToolTip(IFWL_Widget* pWedget) {
  int32_t iCount = m_arrWidget.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    IFWL_ToolTipTarget* p = static_cast<IFWL_ToolTipTarget*>(m_arrWidget[i]);
    if (p->GetWidget() == pWedget) {
      pCurTarget = p;
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL CFWL_ToolTipContainer::ProcessEnter(CFWL_EvtMouse* pEvt,
                                            IFWL_Widget* pOwner) {
  if (HasToolTip(pEvt->m_pDstTarget)) {
    if (NULL == m_pToolTipImp) {
      CFWL_WidgetImpProperties prop;
      prop.m_pDataProvider = m_ToolTipDp;
      prop.m_pOwner = pOwner;
      CFX_RectF rtTooltip;
      rtTooltip.Set(150, 150, 100, 50);
      prop.m_rtWidget = rtTooltip;
      IFWL_ToolTip* pToolTip = IFWL_ToolTip::Create(prop, nullptr);
      pToolTip->Initialize();
      m_pToolTipImp = static_cast<CFWL_ToolTipImp*>(pToolTip->GetImpl());
      m_pToolTipImp->ModifyStylesEx(FWL_STYLEEXT_TTP_Multiline, 0);
      m_pToolTipImp->SetStates(FWL_WGTSTATE_Invisible, TRUE);
    }
    if (pCurTarget->IsShowed()) {
      CFX_WideString wsCaption;
      pCurTarget->GetCaption(wsCaption);
      if (!wsCaption.IsEmpty()) {
        m_ToolTipDp->m_wsCaption = wsCaption;
      }
      CFX_RectF rt;
      rt.Reset();
      CFX_SizeF sz;
      sz.Reset();
      pCurTarget->GetToolTipSize(sz);
      if (sz.x > 0 && sz.y > 0) {
        rt.width = sz.x;
        rt.height = sz.y;
      } else {
        CFX_RectF r;
        m_pToolTipImp->GetWidgetRect(r, TRUE);
        rt.width = r.width;
        rt.height = r.height;
      }
      CFX_PointF pt;
      pt.Set(pEvt->m_fx, pEvt->m_fy);
      if (pCurTarget->GetToolTipPos(pt) == FWL_ERR_Succeeded) {
        rt.left = pt.x;
        rt.top = pt.y;
        m_pToolTipImp->ModifyStylesEx(FWL_STYLEEXT_TTP_NoAnchor, 0);
      } else {
        CFX_RectF rtAnchor;
        pCurTarget->GetWidget()->GetClientRect(rtAnchor);
        pCurTarget->GetWidget()->TransformTo(NULL, rtAnchor.left, rtAnchor.top);
        m_pToolTipImp->SetAnchor(rtAnchor);
        m_pToolTipImp->ModifyStylesEx(0, FWL_STYLEEXT_TTP_NoAnchor);
      }
      m_pToolTipImp->SetWidgetRect(rt);
      m_pToolTipImp->Update();
      m_pToolTipImp->Show();
    }
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CFWL_ToolTipContainer::ProcessLeave(CFWL_EvtMouse* pEvt) {
  if (HasToolTip(pEvt->m_pDstTarget) && NULL != m_pToolTipImp) {
    m_pToolTipImp->Hide();
    pCurTarget = NULL;
    return TRUE;
  }
  return FALSE;
}
IFWL_ToolTipTarget* CFWL_ToolTipContainer::GetCurrentToolTipTarget() {
  return pCurTarget;
}
FX_ERR CFWL_ToolTipContainer::SetToolTipInitialDelay(int32_t nDelayTime) {
  m_ToolTipDp->m_nInitDelayTime = nDelayTime;
  return FWL_ERR_Succeeded;
}
FX_ERR CFWL_ToolTipContainer::SetToolTipAutoPopDelay(int32_t nDelayTime) {
  m_ToolTipDp->m_nAutoPopDelayTime = nDelayTime;
  return FWL_ERR_Succeeded;
}
FWL_ERR FWL_AddToolTipTarget(IFWL_ToolTipTarget* pTarget) {
  return CFWL_ToolTipContainer::getInstance()->AddToolTipTarget(pTarget);
}
FWL_ERR FWL_RemoveToolTipTarget(IFWL_ToolTipTarget* pTarget) {
  return CFWL_ToolTipContainer::getInstance()->RemoveToolTipTarget(pTarget);
}
FWL_ERR FWL_SetToolTipInitialDelay(int32_t nDelayTime) {
  return CFWL_ToolTipContainer::getInstance()->SetToolTipInitialDelay(
      nDelayTime);
}
FWL_ERR FWL_SetToolTipAutoPopDelay(int32_t nDelayTime) {
  return CFWL_ToolTipContainer::getInstance()->SetToolTipAutoPopDelay(
      nDelayTime);
}
IFWL_Widget* FWL_GetCurrentThreadModalWidget(IFWL_NoteThread* pNoteThread) {
  if (!pNoteThread)
    return NULL;
  CFWL_NoteDriver* noteDriver =
      static_cast<CFWL_NoteDriver*>(pNoteThread->GetNoteDriver());
  if (!noteDriver)
    return NULL;
  if (noteDriver->CountLoop() == 1) {
    return NULL;
  }
  CFWL_NoteLoop* topLoop = noteDriver->GetTopLoop();
  if (!topLoop)
    return NULL;
  CFWL_WidgetImp* widget = topLoop->GetForm();
  if (!widget)
    return NULL;
  return widget->GetInterface();
}
FWL_ERR FWL_SetHook(IFWL_NoteDriver* driver,
                    FWLMessageHookCallback callback,
                    void* info) {
  CFWL_NoteDriver* noteDriver = static_cast<CFWL_NoteDriver*>(driver);
  noteDriver->SetHook(callback, info);
  return FWL_ERR_Succeeded;
}
