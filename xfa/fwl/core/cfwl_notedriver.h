// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_NOTEDRIVER_H_
#define XFA_FWL_CORE_CFWL_NOTEDRIVER_H_

#include <deque>
#include <memory>
#include <unordered_map>

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFWL_EventTarget;
class CFWL_NoteLoop;
class CFWL_TargetImp;
class IFWL_Widget;

class CFWL_NoteDriver {
 public:
  CFWL_NoteDriver();
  ~CFWL_NoteDriver();

  void SendEvent(CFWL_Event* pNote);

  void RegisterEventTarget(IFWL_Widget* pListener, IFWL_Widget* pEventSource);
  void UnregisterEventTarget(IFWL_Widget* pListener);
  void ClearEventTargets(bool bRemoveAll);

  CFWL_NoteLoop* GetTopLoop() const;
  void PushNoteLoop(CFWL_NoteLoop* pNoteLoop);
  CFWL_NoteLoop* PopNoteLoop();

  IFWL_Widget* GetFocus() const { return m_pFocus; }
  bool SetFocus(IFWL_Widget* pFocus, bool bNotify = false);
  void SetGrab(IFWL_Widget* pGrab, bool bSet) {
    m_pGrab = bSet ? pGrab : nullptr;
  }

  void Run();

  void NotifyTargetHide(IFWL_Widget* pNoteTarget);
  void NotifyTargetDestroy(IFWL_Widget* pNoteTarget);

  void RegisterForm(IFWL_Widget* pForm);
  void UnRegisterForm(IFWL_Widget* pForm);

  void QueueMessage(std::unique_ptr<CFWL_Message> pMessage);
  void UnqueueMessage(CFWL_NoteLoop* pNoteLoop);
  void ProcessMessage(CFWL_Message* pMessage);

 private:
  bool DispatchMessage(CFWL_Message* pMessage, IFWL_Widget* pMessageForm);
  bool DoSetFocus(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  bool DoKillFocus(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  bool DoKey(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  bool DoMouse(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  bool DoWheel(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  bool DoMouseEx(CFWL_Message* pMsg, IFWL_Widget* pMessageForm);
  void MouseSecondary(CFWL_Message* pMsg);
  bool IsValidMessage(CFWL_Message* pMessage);
  IFWL_Widget* GetMessageForm(IFWL_Widget* pDstTarget);

  CFX_ArrayTemplate<IFWL_Widget*> m_forms;
  std::deque<std::unique_ptr<CFWL_Message>> m_noteQueue;
  CFX_ArrayTemplate<CFWL_NoteLoop*> m_noteLoopQueue;
  std::unordered_map<uint32_t, CFWL_EventTarget*> m_eventTargets;
  IFWL_Widget* m_pHover;
  IFWL_Widget* m_pFocus;
  IFWL_Widget* m_pGrab;
  std::unique_ptr<CFWL_NoteLoop> m_pNoteLoop;
};

#endif  // XFA_FWL_CORE_CFWL_NOTEDRIVER_H_
