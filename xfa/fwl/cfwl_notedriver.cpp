// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_notedriver.h"

#include <algorithm>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_extension.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"

namespace {

uint64_t g_next_listener_key = 1;

}  // namespace

CFWL_NoteDriver::CFWL_NoteDriver() = default;

CFWL_NoteDriver::~CFWL_NoteDriver() = default;

void CFWL_NoteDriver::Trace(cppgc::Visitor* visitor) const {
  for (const auto& item : m_eventTargets)
    item.second->Trace(visitor);

  visitor->Trace(m_pHover);
  visitor->Trace(m_pFocus);
  visitor->Trace(m_pGrab);
}

void CFWL_NoteDriver::SendEvent(CFWL_Event* pNote) {
  for (const auto& pair : m_eventTargets) {
    if (pair.second->IsValid())
      pair.second->ProcessEvent(pNote);
  }
}

void CFWL_NoteDriver::RegisterEventTarget(CFWL_Widget* pListener,
                                          CFWL_Widget* pEventSource) {
  uint64_t key = pListener->GetEventKey();
  if (key == 0) {
    key = g_next_listener_key++;
    pListener->SetEventKey(key);
  }
  if (!m_eventTargets[key])
    m_eventTargets[key] = std::make_unique<Target>(pListener);

  m_eventTargets[key]->SetEventSource(pEventSource);
}

void CFWL_NoteDriver::UnregisterEventTarget(CFWL_Widget* pListener) {
  uint64_t key = pListener->GetEventKey();
  if (key == 0)
    return;

  auto it = m_eventTargets.find(key);
  if (it != m_eventTargets.end())
    it->second->Invalidate();
}

void CFWL_NoteDriver::NotifyTargetHide(CFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget)
    m_pFocus = nullptr;
  if (m_pHover == pNoteTarget)
    m_pHover = nullptr;
  if (m_pGrab == pNoteTarget)
    m_pGrab = nullptr;
}

void CFWL_NoteDriver::NotifyTargetDestroy(CFWL_Widget* pNoteTarget) {
  if (m_pFocus == pNoteTarget)
    m_pFocus = nullptr;
  if (m_pHover == pNoteTarget)
    m_pHover = nullptr;
  if (m_pGrab == pNoteTarget)
    m_pGrab = nullptr;

  UnregisterEventTarget(pNoteTarget);
}

void CFWL_NoteDriver::ProcessMessage(CFWL_Message* pMessage) {
  CFWL_Widget* pMessageForm = pMessage->GetDstTarget();
  if (!pMessageForm)
    return;

  if (!DispatchMessage(pMessage, pMessageForm))
    return;

  if (pMessage->GetType() == CFWL_Message::Type::kMouse)
    MouseSecondary(pMessage);
}

bool CFWL_NoteDriver::DispatchMessage(CFWL_Message* pMessage,
                                      CFWL_Widget* pMessageForm) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus: {
      if (!DoSetFocus(pMessage, pMessageForm))
        return false;
      break;
    }
    case CFWL_Message::Type::kKillFocus: {
      if (!DoKillFocus(pMessage, pMessageForm))
        return false;
      break;
    }
    case CFWL_Message::Type::kKey: {
      if (!DoKey(pMessage, pMessageForm))
        return false;
      break;
    }
    case CFWL_Message::Type::kMouse: {
      if (!DoMouse(pMessage, pMessageForm))
        return false;
      break;
    }
    case CFWL_Message::Type::kMouseWheel: {
      if (!DoWheel(pMessage, pMessageForm))
        return false;
      break;
    }
    default:
      break;
  }
  IFWL_WidgetDelegate* pDelegate = pMessage->GetDstTarget()->GetDelegate();
  if (pDelegate)
    pDelegate->OnProcessMessage(pMessage);

  return true;
}

bool CFWL_NoteDriver::DoSetFocus(CFWL_Message* pMessage,
                                 CFWL_Widget* pMessageForm) {
  m_pFocus = pMessage->GetDstTarget();
  return true;
}

bool CFWL_NoteDriver::DoKillFocus(CFWL_Message* pMessage,
                                  CFWL_Widget* pMessageForm) {
  if (m_pFocus == pMessage->GetDstTarget())
    m_pFocus = nullptr;
  return true;
}

bool CFWL_NoteDriver::DoKey(CFWL_Message* pMessage, CFWL_Widget* pMessageForm) {
  CFWL_MessageKey* pMsg = static_cast<CFWL_MessageKey*>(pMessage);
#if !BUILDFLAG(IS_APPLE)
  if (pMsg->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown &&
      pMsg->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Tab) {
    return true;
  }
#endif

  if (m_pFocus) {
    pMsg->SetDstTarget(m_pFocus.Get());
    return true;
  }

  if (pMsg->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown &&
      pMsg->m_dwKeyCodeOrChar == XFA_FWL_VKEY_Return) {
    CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetFWLApp()->GetWidgetMgr();
    CFWL_Widget* pDefButton = pWidgetMgr->GetDefaultButton(pMessageForm);
    if (pDefButton) {
      pMsg->SetDstTarget(pDefButton);
      return true;
    }
  }
  return false;
}

bool CFWL_NoteDriver::DoMouse(CFWL_Message* pMessage,
                              CFWL_Widget* pMessageForm) {
  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (pMsg->m_dwCmd == CFWL_MessageMouse::MouseCommand::kLeave ||
      pMsg->m_dwCmd == CFWL_MessageMouse::MouseCommand::kHover ||
      pMsg->m_dwCmd == CFWL_MessageMouse::MouseCommand::kEnter) {
    return !!pMsg->GetDstTarget();
  }
  if (pMsg->GetDstTarget() != pMessageForm)
    pMsg->m_pos = pMsg->GetDstTarget()->TransformTo(pMessageForm, pMsg->m_pos);
  if (!DoMouseEx(pMsg, pMessageForm))
    pMsg->SetDstTarget(pMessageForm);
  return true;
}

bool CFWL_NoteDriver::DoWheel(CFWL_Message* pMessage,
                              CFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetFWLApp()->GetWidgetMgr();
  CFWL_MessageMouseWheel* pMsg = static_cast<CFWL_MessageMouseWheel*>(pMessage);
  CFWL_Widget* pDst = pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->pos());
  if (!pDst)
    return false;

  pMsg->set_pos(pMessageForm->TransformTo(pDst, pMsg->pos()));
  pMsg->SetDstTarget(pDst);
  return true;
}

bool CFWL_NoteDriver::DoMouseEx(CFWL_Message* pMessage,
                                CFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetFWLApp()->GetWidgetMgr();
  CFWL_Widget* pTarget = nullptr;
  if (m_pGrab)
    pTarget = m_pGrab.Get();

  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (!pTarget)
    pTarget = pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->m_pos);
  if (!pTarget)
    return false;
  if (pTarget && pMessageForm != pTarget)
    pMsg->m_pos = pMessageForm->TransformTo(pTarget, pMsg->m_pos);

  pMsg->SetDstTarget(pTarget);
  return true;
}

void CFWL_NoteDriver::MouseSecondary(CFWL_Message* pMessage) {
  CFWL_Widget* pTarget = pMessage->GetDstTarget();
  if (pTarget == m_pHover)
    return;

  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (m_pHover) {
    CFWL_MessageMouse msLeave(
        m_pHover.Get(), CFWL_MessageMouse::MouseCommand::kLeave,
        Mask<XFA_FWL_KeyFlag>(),
        pTarget->TransformTo(m_pHover.Get(), pMsg->m_pos));
    DispatchMessage(&msLeave, nullptr);
  }
  if (pTarget->GetClassID() == FWL_Type::Form) {
    m_pHover = nullptr;
    return;
  }
  m_pHover = pTarget;

  CFWL_MessageMouse msHover(pTarget, CFWL_MessageMouse::MouseCommand::kHover,
                            Mask<XFA_FWL_KeyFlag>(), pMsg->m_pos);
  DispatchMessage(&msHover, nullptr);
}

CFWL_NoteDriver::Target::Target(CFWL_Widget* pListener)
    : m_pListener(pListener) {}

CFWL_NoteDriver::Target::~Target() = default;

void CFWL_NoteDriver::Target::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pListener);
  for (auto& widget : m_widgets)
    visitor->Trace(widget);
}

void CFWL_NoteDriver::Target::SetEventSource(CFWL_Widget* pSource) {
  if (pSource)
    m_widgets.insert(pSource);
}

bool CFWL_NoteDriver::Target::ProcessEvent(CFWL_Event* pEvent) {
  IFWL_WidgetDelegate* pDelegate = m_pListener->GetDelegate();
  if (!pDelegate)
    return false;
  if (!m_widgets.empty() && m_widgets.count(pEvent->GetSrcTarget()) == 0)
    return false;
  pDelegate->OnProcessEvent(pEvent);
  return true;
}
