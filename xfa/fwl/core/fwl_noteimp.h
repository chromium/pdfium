// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_FWL_NOTEIMP_H_
#define XFA_FWL_CORE_FWL_NOTEIMP_H_

#include <memory>
#include <unordered_map>

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFWL_CoreToolTipDP;
class CFWL_EventTarget;
class CFWL_MsgActivate;
class CFWL_MsgDeactivate;
class CFWL_MsgDropFiles;
class CFWL_MsgKey;
class CFWL_MsgKillFocus;
class CFWL_MsgMouse;
class CFWL_MsgMouseWheel;
class CFWL_MsgSetFocus;
class CFWL_MsgSize;
class CFWL_MsgWindowMove;
class CFWL_TargetImp;
class IFWL_ToolTip;
class IFWL_Widget;

class CFWL_NoteLoop {
 public:
  CFWL_NoteLoop(IFWL_Widget* pForm = nullptr);
  ~CFWL_NoteLoop() {}

  FWL_Error Idle(int32_t count);
  IFWL_Widget* GetForm();
  bool ContinueModal();
  FWL_Error EndModalLoop();
  FWL_Error SetMainForm(IFWL_Widget* pForm);

 protected:
  void GenerateCommondEvent(uint32_t dwCommand);

  IFWL_Widget* m_pForm;
  bool m_bContinueModal;
};

class CFWL_NoteDriver {
 public:
  CFWL_NoteDriver();
  ~CFWL_NoteDriver();

  void SendEvent(CFWL_Event* pNote);
  FWL_Error RegisterEventTarget(IFWL_Widget* pListener,
                                IFWL_Widget* pEventSource = nullptr,
                                uint32_t dwFilter = FWL_EVENT_ALL_MASK);
  FWL_Error UnregisterEventTarget(IFWL_Widget* pListener);
  void ClearEventTargets(bool bRemoveAll);
  FWL_Error PushNoteLoop(CFWL_NoteLoop* pNoteLoop);
  CFWL_NoteLoop* PopNoteLoop();
  IFWL_Widget* GetFocus();
  bool SetFocus(IFWL_Widget* pFocus, bool bNotify = false);
  void SetGrab(IFWL_Widget* pGrab, bool bSet);
  FWL_Error Run();

  IFWL_Widget* GetHover();
  void SetHover(IFWL_Widget* pHover);
  void NotifyTargetHide(IFWL_Widget* pNoteTarget);
  void NotifyTargetDestroy(IFWL_Widget* pNoteTarget);
  FWL_Error RegisterForm(IFWL_Widget* pForm);
  FWL_Error UnRegisterForm(IFWL_Widget* pForm);
  bool QueueMessage(CFWL_Message* pMessage);
  bool UnqueueMessage(CFWL_NoteLoop* pNoteLoop);
  CFWL_NoteLoop* GetTopLoop();
  int32_t CountLoop();
  bool ProcessMessage(CFWL_Message* pMessage);

 protected:
  bool DispatchMessage(CFWL_Message* pMessage, IFWL_Widget* pMessageForm);
  bool DoActivate(CFWL_MsgActivate* pMsg, IFWL_Widget* pMessageForm);
  bool DoDeactivate(CFWL_MsgDeactivate* pMsg, IFWL_Widget* pMessageForm);
  bool DoSetFocus(CFWL_MsgSetFocus* pMsg, IFWL_Widget* pMessageForm);
  bool DoKillFocus(CFWL_MsgKillFocus* pMsg, IFWL_Widget* pMessageForm);
  bool DoKey(CFWL_MsgKey* pMsg, IFWL_Widget* pMessageForm);
  bool DoMouse(CFWL_MsgMouse* pMsg, IFWL_Widget* pMessageForm);
  bool DoWheel(CFWL_MsgMouseWheel* pMsg, IFWL_Widget* pMessageForm);
  bool DoSize(CFWL_MsgSize* pMsg);
  bool DoWindowMove(CFWL_MsgWindowMove* pMsg, IFWL_Widget* pMessageForm);
  bool DoDragFiles(CFWL_MsgDropFiles* pMsg, IFWL_Widget* pMessageForm);
  bool DoMouseEx(CFWL_MsgMouse* pMsg, IFWL_Widget* pMessageForm);
  void MouseSecondary(CFWL_MsgMouse* pMsg);
  bool IsValidMessage(CFWL_Message* pMessage);
  IFWL_Widget* GetMessageForm(IFWL_Widget* pDstTarget);
  void ClearInvalidEventTargets(bool bRemoveAll);

  CFX_ArrayTemplate<IFWL_Widget*> m_forms;
  CFX_ArrayTemplate<CFWL_Message*> m_noteQueue;
  CFX_ArrayTemplate<CFWL_NoteLoop*> m_noteLoopQueue;
  std::unordered_map<uint32_t, CFWL_EventTarget*> m_eventTargets;
  IFWL_Widget* m_pHover;
  IFWL_Widget* m_pFocus;
  IFWL_Widget* m_pGrab;
  std::unique_ptr<CFWL_NoteLoop> m_pNoteLoop;
};

class CFWL_EventTarget {
 public:
  CFWL_EventTarget(CFWL_NoteDriver* pNoteDriver, IFWL_Widget* pListener);
  ~CFWL_EventTarget();

  int32_t SetEventSource(IFWL_Widget* pSource,
                         uint32_t dwFilter = FWL_EVENT_ALL_MASK);
  bool ProcessEvent(CFWL_Event* pEvent);
  bool IsFilterEvent(CFWL_Event* pEvent, uint32_t dwFilter);
  bool IsInvalid() { return m_bInvalid; }
  void FlagInvalid() { m_bInvalid = true; }

 protected:
  CFX_MapPtrTemplate<void*, uint32_t> m_eventSources;
  IFWL_Widget* m_pListener;
  CFWL_NoteDriver* m_pNoteDriver;
  bool m_bInvalid;
};

class CFWL_ToolTipContainer final {
 public:
  static CFWL_ToolTipContainer* getInstance();
  static void DeleteInstance();

 protected:
  CFWL_ToolTipContainer();
  ~CFWL_ToolTipContainer();

  std::unique_ptr<CFWL_CoreToolTipDP> m_pToolTipDp;

 private:
  static CFWL_ToolTipContainer* s_pInstance;
};

#endif  // XFA_FWL_CORE_FWL_NOTEIMP_H_
