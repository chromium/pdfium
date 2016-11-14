// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/fwl_noteimp.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_tooltip.h"

CFWL_NoteLoop::CFWL_NoteLoop() : m_bContinueModal(true) {}

CFWL_NoteDriver::CFWL_NoteDriver()
    : m_pHover(nullptr),
      m_pFocus(nullptr),
      m_pGrab(nullptr),
      m_pNoteLoop(pdfium::MakeUnique<CFWL_NoteLoop>()) {
  PushNoteLoop(m_pNoteLoop.get());
}
CFWL_NoteDriver::~CFWL_NoteDriver() {
  ClearEventTargets(true);
}

void CFWL_NoteDriver::SendEvent(CFWL_Event* pNote) {
  if (m_eventTargets.empty())
    return;

  for (const auto& pair : m_eventTargets) {
    CFWL_EventTarget* pEventTarget = pair.second;
    if (pEventTarget && !pEventTarget->IsInvalid())
      pEventTarget->ProcessEvent(pNote);
  }
}

void CFWL_NoteDriver::RegisterEventTarget(IFWL_Widget* pListener,
                                          IFWL_Widget* pEventSource,
                                          uint32_t dwFilter) {
  uint32_t key = pListener->GetEventKey();
  if (key == 0) {
    do {
      key = rand();
    } while (key == 0 || pdfium::ContainsKey(m_eventTargets, key));
    pListener->SetEventKey(key);
  }
  if (!m_eventTargets[key])
    m_eventTargets[key] = new CFWL_EventTarget(pListener);

  m_eventTargets[key]->SetEventSource(pEventSource, dwFilter);
}

void CFWL_NoteDriver::UnregisterEventTarget(IFWL_Widget* pListener) {
  uint32_t key = pListener->GetEventKey();
  if (key == 0)
    return;

  auto it = m_eventTargets.find(key);
  if (it != m_eventTargets.end())
    it->second->FlagInvalid();
}

void CFWL_NoteDriver::PushNoteLoop(CFWL_NoteLoop* pNoteLoop) {
  m_noteLoopQueue.Add(pNoteLoop);
}

CFWL_NoteLoop* CFWL_NoteDriver::PopNoteLoop() {
  int32_t pos = m_noteLoopQueue.GetSize();
  if (pos <= 0)
    return nullptr;

  CFWL_NoteLoop* p = m_noteLoopQueue.GetAt(pos - 1);
  m_noteLoopQueue.RemoveAt(pos - 1);
  return p;
}
bool CFWL_NoteDriver::SetFocus(IFWL_Widget* pFocus, bool bNotify) {
  if (m_pFocus == pFocus) {
    return true;
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
    if (IFWL_WidgetDelegate* pDelegate = pPrev->GetDelegate())
      pDelegate->OnProcessMessage(&ms);
  }
  if (pFocus) {
    IFWL_Widget* pWidget =
        pFocus->GetOwnerApp()->GetWidgetMgr()->GetSystemFormWidget(pFocus);
    IFWL_Form* pForm = static_cast<IFWL_Form*>(pWidget);
    if (pForm)
      pForm->SetSubFocus(pFocus);

    CFWL_MsgSetFocus ms;
    ms.m_pDstTarget = pFocus;
    if (bNotify) {
      ms.m_dwExtend = 1;
    }

    if (IFWL_WidgetDelegate* pDelegate = pFocus->GetDelegate())
      pDelegate->OnProcessMessage(&ms);
  }
  return true;
}

void CFWL_NoteDriver::Run() {
#if (_FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_WIN32_DESKTOP_ || \
     _FX_OS_ == _FX_WIN64_)
  CFWL_NoteLoop* pTopLoop = nullptr;
  for (;;) {
    pTopLoop = GetTopLoop();
    if (!pTopLoop || !pTopLoop->ContinueModal())
      break;
    if (UnqueueMessage(pTopLoop))
      continue;
  }
#endif
}

void CFWL_NoteDriver::NotifyTargetHide(IFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget) {
    m_pFocus = nullptr;
  }
  if (m_pHover == pNoteTarget) {
    m_pHover = nullptr;
  }
  if (m_pGrab == pNoteTarget) {
    m_pGrab = nullptr;
  }
}
void CFWL_NoteDriver::NotifyTargetDestroy(IFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget) {
    m_pFocus = nullptr;
  }
  if (m_pHover == pNoteTarget) {
    m_pHover = nullptr;
  }
  if (m_pGrab == pNoteTarget) {
    m_pGrab = nullptr;
  }
  UnregisterEventTarget(pNoteTarget);
  int32_t count = m_forms.GetSize();
  for (int32_t nIndex = 0; nIndex < count; nIndex++) {
    IFWL_Form* pForm = static_cast<IFWL_Form*>(m_forms[nIndex]);
    if (!pForm) {
      continue;
    }
    IFWL_Widget* pSubFocus = pForm->GetSubFocus();
    if (!pSubFocus)
      return;
    if (pSubFocus == pNoteTarget) {
      pForm->SetSubFocus(nullptr);
    }
  }
}

void CFWL_NoteDriver::RegisterForm(IFWL_Widget* pForm) {
  if (!pForm || m_forms.Find(pForm) >= 0)
    return;

  m_forms.Add(pForm);
  if (m_forms.GetSize() == 1) {
    CFWL_NoteLoop* pLoop =
        static_cast<CFWL_NoteLoop*>(m_noteLoopQueue.GetAt(0));
    if (!pLoop)
      return;
    pLoop->SetMainForm(pForm);
  }
}

void CFWL_NoteDriver::UnRegisterForm(IFWL_Widget* pForm) {
  if (!pForm)
    return;
  int32_t nIndex = m_forms.Find(pForm);
  if (nIndex < 0)
    return;
  m_forms.RemoveAt(nIndex);
}
bool CFWL_NoteDriver::QueueMessage(CFWL_Message* pMessage) {
  pMessage->Retain();
  m_noteQueue.Add(pMessage);
  return true;
}
bool CFWL_NoteDriver::UnqueueMessage(CFWL_NoteLoop* pNoteLoop) {
  if (m_noteQueue.GetSize() < 1) {
    return false;
  }
  CFWL_Message* pMessage = m_noteQueue[0];
  m_noteQueue.RemoveAt(0);
  if (!IsValidMessage(pMessage)) {
    pMessage->Release();
    return true;
  }
  ProcessMessage(pMessage);

  pMessage->Release();
  return true;
}
CFWL_NoteLoop* CFWL_NoteDriver::GetTopLoop() const {
  int32_t size = m_noteLoopQueue.GetSize();
  if (size <= 0)
    return nullptr;
  return static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[size - 1]);
}

bool CFWL_NoteDriver::ProcessMessage(CFWL_Message* pMessage) {
  CFWL_WidgetMgr* pWidgetMgr =
      pMessage->m_pDstTarget->GetOwnerApp()->GetWidgetMgr();
  IFWL_Widget* pMessageForm = pWidgetMgr->IsFormDisabled()
                                  ? pMessage->m_pDstTarget
                                  : GetMessageForm(pMessage->m_pDstTarget);
  if (!pMessageForm)
    return false;
  if (DispatchMessage(pMessage, pMessageForm)) {
    if (pMessage->GetClassID() == CFWL_MessageType::Mouse)
      MouseSecondary(static_cast<CFWL_MsgMouse*>(pMessage));
    return true;
  }
  return false;
}

bool CFWL_NoteDriver::DispatchMessage(CFWL_Message* pMessage,
                                      IFWL_Widget* pMessageForm) {
  bool bRet = false;
  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::Activate: {
      bRet = DoActivate(static_cast<CFWL_MsgActivate*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::Deactivate: {
      bRet = DoDeactivate(static_cast<CFWL_MsgDeactivate*>(pMessage),
                          pMessageForm);
      break;
    }
    case CFWL_MessageType::SetFocus: {
      bRet = DoSetFocus(static_cast<CFWL_MsgSetFocus*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::KillFocus: {
      bRet =
          DoKillFocus(static_cast<CFWL_MsgKillFocus*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::Key: {
      bRet = DoKey(static_cast<CFWL_MsgKey*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::Mouse: {
      bRet = DoMouse(static_cast<CFWL_MsgMouse*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::MouseWheel: {
      bRet = DoWheel(static_cast<CFWL_MsgMouseWheel*>(pMessage), pMessageForm);
      break;
    }
    case CFWL_MessageType::Size: {
      bRet = DoSize(static_cast<CFWL_MsgSize*>(pMessage));
      break;
    }
    case CFWL_MessageType::Cursor: {
      bRet = true;
      break;
    }
    case CFWL_MessageType::WindowMove: {
      bRet = DoWindowMove(static_cast<CFWL_MsgWindowMove*>(pMessage),
                          pMessageForm);
      break;
    }
    case CFWL_MessageType::DropFiles: {
      bRet =
          DoDragFiles(static_cast<CFWL_MsgDropFiles*>(pMessage), pMessageForm);
      break;
    }
    default: {
      bRet = true;
      break;
    }
  }
  if (bRet) {
    if (IFWL_WidgetDelegate* pDelegate =
            pMessage->m_pDstTarget->GetDelegate()) {
      pDelegate->OnProcessMessage(pMessage);
    }
  }
  return bRet;
}

bool CFWL_NoteDriver::DoActivate(CFWL_MsgActivate* pMsg,
                                 IFWL_Widget* pMessageForm) {
  pMsg->m_pDstTarget = pMessageForm;
  return !!(pMsg->m_pDstTarget->GetStates() & FWL_WGTSTATE_Deactivated);
}

bool CFWL_NoteDriver::DoDeactivate(CFWL_MsgDeactivate* pMsg,
                                   IFWL_Widget* pMessageForm) {
  int32_t iTrackLoop = m_noteLoopQueue.GetSize();
  if (iTrackLoop <= 0)
    return false;
  if (iTrackLoop == 1) {
    if (pMessageForm->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"))) {
      return false;
    }
    if (pMsg->m_pSrcTarget &&
        pMsg->m_pSrcTarget->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"))) {
      return false;
    }
    if (pMsg->m_pSrcTarget &&
        pMsg->m_pSrcTarget->GetClassID() == FWL_Type::ToolTip) {
      return false;
    }
    return true;
  }
  IFWL_Widget* pDst = pMsg->m_pDstTarget;
  if (!pDst)
    return false;
#if (_FX_OS_ == _FX_MACOSX_)
  if (pDst == pMessageForm && pDst->IsInstance(L"FWL_FORMPROXY")) {
    return true;
  }
#endif
  return pDst != pMessageForm &&
         !pDst->IsInstance(FX_WSTRC(L"FWL_FORMPROXY")) &&
         !pMessageForm->IsInstance(FX_WSTRC(L"FWL_FORMPROXY"));
}
bool CFWL_NoteDriver::DoSetFocus(CFWL_MsgSetFocus* pMsg,
                                 IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
  if (pWidgetMgr->IsFormDisabled()) {
    m_pFocus = pMsg->m_pDstTarget;
    return true;
  }
  IFWL_Widget* pWidget = pMsg->m_pDstTarget;
  if (pWidget) {
    IFWL_Form* pForm = static_cast<IFWL_Form*>(pWidget);
    IFWL_Widget* pSubFocus = pForm->GetSubFocus();
    if (pSubFocus && ((pSubFocus->GetStates() & FWL_WGTSTATE_Focused) == 0)) {
      pMsg->m_pDstTarget = pSubFocus;
      if (m_pFocus != pMsg->m_pDstTarget) {
        m_pFocus = pMsg->m_pDstTarget;
        return true;
      }
    }
  }
  return false;
}
bool CFWL_NoteDriver::DoKillFocus(CFWL_MsgKillFocus* pMsg,
                                  IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
  if (pWidgetMgr->IsFormDisabled()) {
    if (m_pFocus == pMsg->m_pDstTarget) {
      m_pFocus = nullptr;
    }
    return true;
  }
  IFWL_Form* pForm = static_cast<IFWL_Form*>(pMsg->m_pDstTarget);
  if (pForm) {
    IFWL_Widget* pSubFocus = pForm->GetSubFocus();
    if (pSubFocus && (pSubFocus->GetStates() & FWL_WGTSTATE_Focused)) {
      pMsg->m_pDstTarget = pSubFocus;
      if (m_pFocus == pMsg->m_pDstTarget) {
        m_pFocus = nullptr;
        return true;
      }
    }
  }
  return false;
}
bool CFWL_NoteDriver::DoKey(CFWL_MsgKey* pMsg, IFWL_Widget* pMessageForm) {
#if (_FX_OS_ != _FX_MACOSX_)
  if (pMsg->m_dwCmd == FWL_KeyCommand::KeyDown &&
      pMsg->m_dwKeyCode == FWL_VKEY_Tab) {
    CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
    IFWL_Widget* pForm = GetMessageForm(pMsg->m_pDstTarget);
    IFWL_Widget* pFocus = m_pFocus;
    if (m_pFocus) {
      if (pWidgetMgr->GetSystemFormWidget(m_pFocus) != pForm)
        pFocus = nullptr;
    }
    bool bFind = false;
    IFWL_Widget* pNextTabStop = pWidgetMgr->nextTab(pForm, pFocus, bFind);
    if (!pNextTabStop) {
      bFind = false;
      pNextTabStop = pWidgetMgr->nextTab(pForm, nullptr, bFind);
    }
    if (pNextTabStop == pFocus) {
      return true;
    }
    if (pNextTabStop) {
      SetFocus(pNextTabStop);
    }
    return true;
  }
#endif
  if (!m_pFocus) {
    if (pMsg->m_dwCmd == FWL_KeyCommand::KeyDown &&
        pMsg->m_dwKeyCode == FWL_VKEY_Return) {
      CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
      IFWL_Widget* defButton = pWidgetMgr->GetDefaultButton(pMessageForm);
      if (defButton) {
        pMsg->m_pDstTarget = defButton;
        return true;
      }
    }
    return false;
  }
  pMsg->m_pDstTarget = m_pFocus;
  return true;
}
bool CFWL_NoteDriver::DoMouse(CFWL_MsgMouse* pMsg, IFWL_Widget* pMessageForm) {
  if (pMsg->m_dwCmd == FWL_MouseCommand::Leave ||
      pMsg->m_dwCmd == FWL_MouseCommand::Hover ||
      pMsg->m_dwCmd == FWL_MouseCommand::Enter) {
    return !!pMsg->m_pDstTarget;
  }
  if (pMsg->m_pDstTarget != pMessageForm) {
    pMsg->m_pDstTarget->TransformTo(pMessageForm, pMsg->m_fx, pMsg->m_fy);
  }
  if (!DoMouseEx(pMsg, pMessageForm)) {
    pMsg->m_pDstTarget = pMessageForm;
  }
  return true;
}
bool CFWL_NoteDriver::DoWheel(CFWL_MsgMouseWheel* pMsg,
                              IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
  if (!pWidgetMgr)
    return false;

  IFWL_Widget* pDst =
      pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->m_fx, pMsg->m_fy);
  if (!pDst)
    return false;

  pMessageForm->TransformTo(pDst, pMsg->m_fx, pMsg->m_fy);
  pMsg->m_pDstTarget = pDst;
  return true;
}
bool CFWL_NoteDriver::DoSize(CFWL_MsgSize* pMsg) {
  CFWL_WidgetMgr* pWidgetMgr =
      pMsg->m_pDstTarget->GetOwnerApp()->GetWidgetMgr();
  if (!pWidgetMgr)
    return false;
  pWidgetMgr->NotifySizeChanged(pMsg->m_pDstTarget, (FX_FLOAT)pMsg->m_iWidth,
                                (FX_FLOAT)pMsg->m_iHeight);
  return true;
}
bool CFWL_NoteDriver::DoWindowMove(CFWL_MsgWindowMove* pMsg,
                                   IFWL_Widget* pMessageForm) {
  return pMsg->m_pDstTarget == pMessageForm;
}
bool CFWL_NoteDriver::DoDragFiles(CFWL_MsgDropFiles* pMsg,
                                  IFWL_Widget* pMessageForm) {
  return pMsg->m_pDstTarget == pMessageForm;
}
bool CFWL_NoteDriver::DoMouseEx(CFWL_MsgMouse* pMsg,
                                IFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetOwnerApp()->GetWidgetMgr();
  if (!pWidgetMgr)
    return false;
  IFWL_Widget* pTarget = nullptr;
  if (m_pGrab)
    pTarget = m_pGrab;
  if (!pTarget) {
    pTarget =
        pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->m_fx, pMsg->m_fy);
  }
  if (pTarget) {
    if (pMessageForm != pTarget) {
      pMessageForm->TransformTo(pTarget, pMsg->m_fx, pMsg->m_fy);
    }
  }
  if (!pTarget)
    return false;
  pMsg->m_pDstTarget = pTarget;
  return true;
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
    msLeave.m_dwCmd = FWL_MouseCommand::Leave;
    DispatchMessage(&msLeave, nullptr);
  }
  if (pTarget->GetClassID() == FWL_Type::Form) {
    m_pHover = nullptr;
    return;
  }
  m_pHover = pTarget;
  CFWL_MsgMouse msHover;
  msHover.m_pDstTarget = pTarget;
  msHover.m_fx = pMsg->m_fx;
  msHover.m_fy = pMsg->m_fy;
  msHover.m_dwFlags = 0;
  msHover.m_dwCmd = FWL_MouseCommand::Hover;
  DispatchMessage(&msHover, nullptr);
}
bool CFWL_NoteDriver::IsValidMessage(CFWL_Message* pMessage) {
  if (pMessage->GetClassID() == CFWL_MessageType::Post)
    return true;

  int32_t iCount = m_noteLoopQueue.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    CFWL_NoteLoop* pNoteLoop = static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[i]);
    IFWL_Widget* pForm = pNoteLoop->GetForm();
    if (pForm && (pForm == pMessage->m_pDstTarget))
      return true;
  }
  iCount = m_forms.GetSize();
  for (int32_t j = 0; j < iCount; j++) {
    IFWL_Form* pForm = static_cast<IFWL_Form*>(m_forms[j]);
    if (pForm == pMessage->m_pDstTarget)
      return true;
  }
  return false;
}

IFWL_Widget* CFWL_NoteDriver::GetMessageForm(IFWL_Widget* pDstTarget) {
  int32_t iTrackLoop = m_noteLoopQueue.GetSize();
  if (iTrackLoop <= 0)
    return nullptr;
  IFWL_Widget* pMessageForm = nullptr;
  if (iTrackLoop > 1) {
    CFWL_NoteLoop* pNootLoop =
        static_cast<CFWL_NoteLoop*>(m_noteLoopQueue[iTrackLoop - 1]);
    pMessageForm = pNootLoop->GetForm();
  } else if (m_forms.Find(pDstTarget) < 0) {
    pMessageForm = pDstTarget;
  }
  if (!pMessageForm && pDstTarget) {
    CFWL_WidgetMgr* pWidgetMgr = pDstTarget->GetOwnerApp()->GetWidgetMgr();
    if (!pWidgetMgr)
      return nullptr;
    pMessageForm = pWidgetMgr->GetSystemFormWidget(pDstTarget);
  }
  return pMessageForm;
}

void CFWL_NoteDriver::ClearEventTargets(bool bRemoveAll) {
  auto it = m_eventTargets.begin();
  while (it != m_eventTargets.end()) {
    auto old = it++;
    if (old->second && (bRemoveAll || old->second->IsInvalid())) {
      delete old->second;
      m_eventTargets.erase(old);
    }
  }
}

CFWL_EventTarget::CFWL_EventTarget(IFWL_Widget* pListener)
    : m_pListener(pListener), m_bInvalid(false) {}
CFWL_EventTarget::~CFWL_EventTarget() {
  m_eventSources.RemoveAll();
}

int32_t CFWL_EventTarget::SetEventSource(IFWL_Widget* pSource,
                                         uint32_t dwFilter) {
  if (pSource) {
    m_eventSources.SetAt(pSource, dwFilter);
    return m_eventSources.GetCount();
  }
  return 1;
}

bool CFWL_EventTarget::ProcessEvent(CFWL_Event* pEvent) {
  IFWL_WidgetDelegate* pDelegate = m_pListener->GetDelegate();
  if (!pDelegate)
    return false;
  if (m_eventSources.GetCount() == 0) {
    pDelegate->OnProcessEvent(pEvent);
    return true;
  }
  FX_POSITION pos = m_eventSources.GetStartPosition();
  while (pos) {
    IFWL_Widget* pSource = nullptr;
    uint32_t dwFilter = 0;
    m_eventSources.GetNextAssoc(pos, (void*&)pSource, dwFilter);
    if (pSource == pEvent->m_pSrcTarget ||
        pEvent->GetClassID() == CFWL_EventType::Idle) {
      if (IsFilterEvent(pEvent, dwFilter)) {
        pDelegate->OnProcessEvent(pEvent);
        return true;
      }
    }
  }
  return false;
}

bool CFWL_EventTarget::IsFilterEvent(CFWL_Event* pEvent,
                                     uint32_t dwFilter) const {
  if (dwFilter == FWL_EVENT_ALL_MASK)
    return true;

  switch (pEvent->GetClassID()) {
    case CFWL_EventType::Mouse:
      return !!(dwFilter & FWL_EVENT_MOUSE_MASK);
    case CFWL_EventType::MouseWheel:
      return !!(dwFilter & FWL_EVENT_MOUSEWHEEL_MASK);
    case CFWL_EventType::Key:
      return !!(dwFilter & FWL_EVENT_KEY_MASK);
    case CFWL_EventType::SetFocus:
    case CFWL_EventType::KillFocus:
      return !!(dwFilter & FWL_EVENT_FOCUSCHANGED_MASK);
    case CFWL_EventType::Draw:
      return !!(dwFilter & FWL_EVENT_DRAW_MASK);
    case CFWL_EventType::Close:
      return !!(dwFilter & FWL_EVENT_CLOSE_MASK);
    case CFWL_EventType::SizeChanged:
      return !!(dwFilter & FWL_EVENT_SIZECHANGED_MASK);
    case CFWL_EventType::Idle:
      return !!(dwFilter & FWL_EVENT_IDLE_MASK);
    default:
      return !!(dwFilter & FWL_EVENT_CONTROL_MASK);
  }
}
