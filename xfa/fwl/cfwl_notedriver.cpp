// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_notedriver.h"

#include <algorithm>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/fx_extension.h"
#include "fxjs/gc/container_trace.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"

namespace pdfium {

namespace {

uint64_t g_next_listener_key = 1;

}  // namespace

CFWL_NoteDriver::CFWL_NoteDriver(CFWL_App* pApp) : app_(pApp) {}

CFWL_NoteDriver::~CFWL_NoteDriver() = default;

void CFWL_NoteDriver::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(app_);
  visitor->Trace(hover_);
  visitor->Trace(focus_);
  visitor->Trace(grab_);
  ContainerTrace(visitor, event_targets_);
}

void CFWL_NoteDriver::SendEvent(CFWL_Event* pNote) {
  for (const auto& pair : event_targets_) {
    if (pair.second->IsValid()) {
      pair.second->ProcessEvent(pNote);
    }
  }
}

void CFWL_NoteDriver::RegisterEventTarget(CFWL_Widget* pListener,
                                          CFWL_Widget* pEventSource) {
  uint64_t key = pListener->GetEventKey();
  if (key == 0) {
    key = g_next_listener_key++;
    pListener->SetEventKey(key);
  }
  if (!event_targets_[key]) {
    event_targets_[key] = cppgc::MakeGarbageCollected<Target>(
        app_->GetHeap()->GetAllocationHandle(), pListener);
  }
  event_targets_[key]->SetEventSource(pEventSource);
}

void CFWL_NoteDriver::UnregisterEventTarget(CFWL_Widget* pListener) {
  uint64_t key = pListener->GetEventKey();
  if (key == 0) {
    return;
  }

  auto it = event_targets_.find(key);
  if (it != event_targets_.end()) {
    it->second->Invalidate();
  }
}

void CFWL_NoteDriver::NotifyTargetHide(CFWL_Widget* pNoteTarget) {
  if (focus_ == pNoteTarget) {
    focus_ = nullptr;
  }
  if (hover_ == pNoteTarget) {
    hover_ = nullptr;
  }
  if (grab_ == pNoteTarget) {
    grab_ = nullptr;
  }
}

void CFWL_NoteDriver::NotifyTargetDestroy(CFWL_Widget* pNoteTarget) {
  if (focus_ == pNoteTarget) {
    focus_ = nullptr;
  }
  if (hover_ == pNoteTarget) {
    hover_ = nullptr;
  }
  if (grab_ == pNoteTarget) {
    grab_ = nullptr;
  }

  UnregisterEventTarget(pNoteTarget);
}

void CFWL_NoteDriver::ProcessMessage(CFWL_Message* pMessage) {
  CFWL_Widget* pMessageForm = pMessage->GetDstTarget();
  if (!pMessageForm) {
    return;
  }

  if (!DispatchMessage(pMessage, pMessageForm)) {
    return;
  }

  if (pMessage->GetType() == CFWL_Message::Type::kMouse) {
    MouseSecondary(pMessage);
  }
}

bool CFWL_NoteDriver::DispatchMessage(CFWL_Message* pMessage,
                                      CFWL_Widget* pMessageForm) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus: {
      if (!DoSetFocus(pMessage, pMessageForm)) {
        return false;
      }
      break;
    }
    case CFWL_Message::Type::kKillFocus: {
      if (!DoKillFocus(pMessage, pMessageForm)) {
        return false;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      if (!DoKey(pMessage, pMessageForm)) {
        return false;
      }
      break;
    }
    case CFWL_Message::Type::kMouse: {
      if (!DoMouse(pMessage, pMessageForm)) {
        return false;
      }
      break;
    }
    case CFWL_Message::Type::kMouseWheel: {
      if (!DoWheel(pMessage, pMessageForm)) {
        return false;
      }
      break;
    }
  }
  IFWL_WidgetDelegate* pDelegate = pMessage->GetDstTarget()->GetDelegate();
  if (pDelegate) {
    pDelegate->OnProcessMessage(pMessage);
  }

  return true;
}

bool CFWL_NoteDriver::DoSetFocus(CFWL_Message* pMessage,
                                 CFWL_Widget* pMessageForm) {
  focus_ = pMessage->GetDstTarget();
  return true;
}

bool CFWL_NoteDriver::DoKillFocus(CFWL_Message* pMessage,
                                  CFWL_Widget* pMessageForm) {
  if (focus_ == pMessage->GetDstTarget()) {
    focus_ = nullptr;
  }
  return true;
}

bool CFWL_NoteDriver::DoKey(CFWL_Message* pMessage, CFWL_Widget* pMessageForm) {
  CFWL_MessageKey* pMsg = static_cast<CFWL_MessageKey*>(pMessage);
#if !BUILDFLAG(IS_APPLE)
  if (pMsg->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown &&
      pMsg->key_code_or_char_ == XFA_FWL_VKEY_Tab) {
    return true;
  }
#endif

  if (focus_) {
    pMsg->SetDstTarget(focus_.Get());
    return true;
  }

  if (pMsg->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown &&
      pMsg->key_code_or_char_ == XFA_FWL_VKEY_Return) {
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
  if (pMsg->cmd_ == CFWL_MessageMouse::MouseCommand::kLeave ||
      pMsg->cmd_ == CFWL_MessageMouse::MouseCommand::kHover ||
      pMsg->cmd_ == CFWL_MessageMouse::MouseCommand::kEnter) {
    return !!pMsg->GetDstTarget();
  }
  if (pMsg->GetDstTarget() != pMessageForm) {
    pMsg->pos_ = pMsg->GetDstTarget()->TransformTo(pMessageForm, pMsg->pos_);
  }
  if (!DoMouseEx(pMsg, pMessageForm)) {
    pMsg->SetDstTarget(pMessageForm);
  }
  return true;
}

bool CFWL_NoteDriver::DoWheel(CFWL_Message* pMessage,
                              CFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetFWLApp()->GetWidgetMgr();
  CFWL_MessageMouseWheel* pMsg = static_cast<CFWL_MessageMouseWheel*>(pMessage);
  CFWL_Widget* pDst = pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->pos());
  if (!pDst) {
    return false;
  }

  pMsg->set_pos(pMessageForm->TransformTo(pDst, pMsg->pos()));
  pMsg->SetDstTarget(pDst);
  return true;
}

bool CFWL_NoteDriver::DoMouseEx(CFWL_Message* pMessage,
                                CFWL_Widget* pMessageForm) {
  CFWL_WidgetMgr* pWidgetMgr = pMessageForm->GetFWLApp()->GetWidgetMgr();
  CFWL_Widget* pTarget = nullptr;
  if (grab_) {
    pTarget = grab_.Get();
  }

  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (!pTarget) {
    pTarget = pWidgetMgr->GetWidgetAtPoint(pMessageForm, pMsg->pos_);
  }
  if (!pTarget) {
    return false;
  }
  if (pTarget && pMessageForm != pTarget) {
    pMsg->pos_ = pMessageForm->TransformTo(pTarget, pMsg->pos_);
  }

  pMsg->SetDstTarget(pTarget);
  return true;
}

void CFWL_NoteDriver::MouseSecondary(CFWL_Message* pMessage) {
  CFWL_Widget* pTarget = pMessage->GetDstTarget();
  if (pTarget == hover_) {
    return;
  }

  CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
  if (hover_) {
    CFWL_MessageMouse msLeave(hover_.Get(),
                              CFWL_MessageMouse::MouseCommand::kLeave,
                              Mask<XFA_FWL_KeyFlag>(),
                              pTarget->TransformTo(hover_.Get(), pMsg->pos_));
    DispatchMessage(&msLeave, nullptr);
  }
  if (pTarget->GetClassID() == FWL_Type::Form) {
    hover_ = nullptr;
    return;
  }
  hover_ = pTarget;

  CFWL_MessageMouse msHover(pTarget, CFWL_MessageMouse::MouseCommand::kHover,
                            Mask<XFA_FWL_KeyFlag>(), pMsg->pos_);
  DispatchMessage(&msHover, nullptr);
}

CFWL_NoteDriver::Target::Target(CFWL_Widget* pListener)
    : listener_(pListener) {}

CFWL_NoteDriver::Target::~Target() = default;

void CFWL_NoteDriver::Target::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(listener_);
  for (auto& widget : widgets_) {
    visitor->Trace(widget);
  }
}

void CFWL_NoteDriver::Target::SetEventSource(CFWL_Widget* pSource) {
  if (pSource) {
    widgets_.insert(pSource);
  }
}

bool CFWL_NoteDriver::Target::ProcessEvent(CFWL_Event* pEvent) {
  IFWL_WidgetDelegate* pDelegate = listener_->GetDelegate();
  if (!pDelegate) {
    return false;
  }
  if (!widgets_.empty() && widgets_.count(pEvent->GetSrcTarget()) == 0) {
    return false;
  }
  pDelegate->OnProcessEvent(pEvent);
  return true;
}

}  // namespace pdfium
