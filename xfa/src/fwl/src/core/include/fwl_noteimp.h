// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_NOTE_IMP_H
#define _FWL_NOTE_IMP_H

#include "xfa/include/fwl/core/fwl_note.h"

class CFWL_TargetImp;
class CFWL_WidgetImp;
class CFWL_NoteThreadImp;
class CFWL_ToolTipImp;
class CFWL_CoreToopTipDP;
class CFWL_NoteDriver;
class CFWL_EventTarget;
class CFWL_ToolTipContainer;

class CFWL_NoteLoop : public IFWL_NoteLoop {
 public:
  CFWL_NoteLoop(CFWL_WidgetImp* pForm = NULL);

  // IFWL_NoteLoop:
  ~CFWL_NoteLoop() override {}
  FX_BOOL PreProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR Idle(int32_t count) override;

  CFWL_WidgetImp* GetForm();
  FX_BOOL ContinueModal();
  FWL_ERR EndModalLoop();
  FX_BOOL TranslateAccelerator(CFWL_Message* pMessage);
  FWL_ERR SetMainForm(CFWL_WidgetImp* pForm);

 protected:
  void GenerateCommondEvent(FX_DWORD dwCommand);

  CFWL_WidgetImp* m_pForm;
  FX_BOOL m_bContinueModal;
};
class CFWL_NoteDriver : public IFWL_NoteDriver {
 public:
  CFWL_NoteDriver();
  ~CFWL_NoteDriver() override;

  // IFWL_NoteDriver:
  FX_BOOL SendNote(CFWL_Note* pNote) override;
  FX_BOOL PostMessage(CFWL_Message* pMessage) override;
  FWL_ERR RegisterEventTarget(IFWL_Widget* pListener,
                              IFWL_Widget* pEventSource = NULL,
                              FX_DWORD dwFilter = FWL_EVENT_ALL_MASK) override;
  FWL_ERR UnregisterEventTarget(IFWL_Widget* pListener) override;
  void ClearEventTargets(FX_BOOL bRemoveAll) override;
  int32_t GetQueueMaxSize() const override;
  FWL_ERR SetQueueMaxSize(const int32_t size) override;
  IFWL_NoteThread* GetOwnerThread() const override;
  FWL_ERR PushNoteLoop(IFWL_NoteLoop* pNoteLoop) override;
  IFWL_NoteLoop* PopNoteLoop() override;
  IFWL_Widget* GetFocus() override;
  FX_BOOL SetFocus(IFWL_Widget* pFocus, FX_BOOL bNotify = FALSE) override;
  void SetGrab(IFWL_Widget* pGrab, FX_BOOL bSet) override;
  FWL_ERR Run() override;

  IFWL_Widget* GetHover();
  void SetHover(IFWL_Widget* pHover);
  void NotifyTargetHide(IFWL_Widget* pNoteTarget);
  void NotifyTargetDestroy(IFWL_Widget* pNoteTarget);
  void NotifyFullScreenMode(IFWL_Widget* pNoteTarget, FX_BOOL bFullScreen);
  FWL_ERR RegisterForm(CFWL_WidgetImp* pForm);
  FWL_ERR UnRegisterForm(CFWL_WidgetImp* pForm);
  FX_BOOL QueueMessage(CFWL_Message* pMessage);
  FX_BOOL UnqueueMessage(CFWL_NoteLoop* pNoteLoop);
  CFWL_NoteLoop* GetTopLoop();
  int32_t CountLoop();
  void SetHook(FWLMessageHookCallback callback, void* info);
  FX_BOOL ProcessMessage(CFWL_Message* pMessage);

 protected:
  FX_BOOL DispatchMessage(CFWL_Message* pMessage, IFWL_Widget* pMessageForm);
  FX_BOOL DoActivate(CFWL_MsgActivate* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoDeactivate(CFWL_MsgDeactivate* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoSetFocus(CFWL_MsgSetFocus* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoKillFocus(CFWL_MsgKillFocus* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoKey(CFWL_MsgKey* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoMouse(CFWL_MsgMouse* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoWheel(CFWL_MsgMouseWheel* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoSize(CFWL_MsgSize* pMsg);
  FX_BOOL DoWindowMove(CFWL_MsgWindowMove* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoDragFiles(CFWL_MsgDropFiles* pMsg, IFWL_Widget* pMessageForm);
  FX_BOOL DoMouseEx(CFWL_MsgMouse* pMsg, IFWL_Widget* pMessageForm);
  void MouseSecondary(CFWL_MsgMouse* pMsg);
  FX_BOOL IsValidMessage(CFWL_Message* pMessage);
  IFWL_Widget* GetMessageForm(IFWL_Widget* pDstTarget);
  void ClearInvalidEventTargets(FX_BOOL bRemoveAll);
  CFX_PtrArray m_forms;
  CFX_PtrArray m_noteQueue;
  CFX_PtrArray m_noteLoopQueue;
  CFX_MapPtrToPtr m_eventTargets;
  int32_t m_sendEventCalled;
  int32_t m_maxSize;
  FX_BOOL m_bFullScreen;
  IFWL_Widget* m_pHover;
  IFWL_Widget* m_pFocus;
  IFWL_Widget* m_pGrab;
  CFWL_NoteLoop* m_pNoteLoop;
  FWLMessageHookCallback m_hook;
  void* m_hookInfo;
};
typedef CFX_MapPtrTemplate<void*, FX_DWORD> CFWL_EventSource;
class CFWL_EventTarget {
 public:
  CFWL_EventTarget(CFWL_NoteDriver* pNoteDriver, IFWL_Widget* pListener)
      : m_pListener(pListener), m_pNoteDriver(pNoteDriver), m_bInvalid(FALSE) {}
  ~CFWL_EventTarget();
  int32_t SetEventSource(IFWL_Widget* pSource,
                         FX_DWORD dwFilter = FWL_EVENT_ALL_MASK);
  FX_BOOL ProcessEvent(CFWL_Event* pEvent);
  FX_BOOL IsFilterEvent(CFWL_Event* pEvent, FX_DWORD dwFilter);
  FX_BOOL IsInvalid() { return m_bInvalid; }
  void FlagInvalid() { m_bInvalid = TRUE; }

 protected:
  CFWL_EventSource m_eventSources;
  IFWL_Widget* m_pListener;
  CFWL_NoteDriver* m_pNoteDriver;
  FX_BOOL m_bInvalid;
};
class CFWL_ToolTipContainer {
 public:
  static CFWL_ToolTipContainer* getInstance();
  static void DeleteInstance();

  FX_ERR AddToolTipTarget(IFWL_ToolTipTarget* pTarget);
  FX_ERR RemoveToolTipTarget(IFWL_ToolTipTarget* pTarget);
  IFWL_ToolTipTarget* GetCurrentToolTipTarget();

  FX_BOOL HasToolTip(IFWL_Widget* pWidget);

  FX_BOOL ProcessEnter(CFWL_EvtMouse* pEvt, IFWL_Widget* pOwner);
  FX_BOOL ProcessLeave(CFWL_EvtMouse* pEvt);

  FX_ERR SetToolTipInitialDelay(int32_t iDelayTime);
  FX_ERR SetToolTipAutoPopDelay(int32_t iDelayTime);

 protected:
  CFWL_ToolTipContainer();
  virtual ~CFWL_ToolTipContainer();

  IFWL_ToolTipTarget* pCurTarget;
  CFWL_ToolTipImp* m_pToolTipImp;
  CFWL_CoreToopTipDP* m_ToolTipDp;
  CFX_PtrArray m_arrWidget;

 private:
  static CFWL_ToolTipContainer* s_pInstance;
};
#endif
