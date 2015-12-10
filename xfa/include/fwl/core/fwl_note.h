// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_NOTE_H
#define _FWL_NOTE_H
class IFWL_Target;
class IFWL_Widget;
class IFWL_NoteThread;
class IFWL_ThemeProvider;
class CFWL_Note;
class CFWL_Message;
class CFWL_MsgActivate;
class CFWL_MsgDeactivate;
class CFWL_MsgMouse;
class CFWL_MsgMouseWheel;
class CFWL_MsgKey;
class CFWL_MsgSetFocus;
class CFWL_MsgKillFocus;
class CFWL_MsgCursor;
class CFWL_MsgSize;
class CFWL_MsgWindowMove;
class CFWL_MsgDropFiles;
class CFWL_MsgTaskClicked;
class CFWL_MsgClose;
class CFWL_MsgWindowWillMove;
class CFWL_Event;
class CFWL_EvtMouse;
class CFWL_EvtMouseWheel;
class CFWL_EvtKey;
class CFWL_EvtSetFocus;
class CFWL_EvtKillFocus;
class CFWL_EvtDraw;
class CFWL_EvtClick;
class CFWL_EvtScroll;
class CFWL_EvtClose;
class CFWL_EvtContextMenu;
class CFWL_EvtMenuCommand;
class CFWL_EvtSizeChanged;
class CFWL_EvtIdle;
class IFWL_NoteDriver;
class IFWL_NoteLoop;
#define FWL_MSG_Activate L"FWL_MESSAGE_Activate"
#define FWL_MSG_Deactivate L"FWL_MESSAGE_Deactivate"
#define FWL_MSG_SetFocus L"FWL_MESSAGE_SetFocus"
#define FWL_MSG_KillFocus L"FWL_MESSAGE_KillFocus"
#define FWL_MSG_Mouse L"FWL_MESSAGE_Mouse"
#define FWL_MSG_MouseWheel L"FWL_MESSAGE_MouseWheel"
#define FWL_MSG_Key L"FWL_MESSAGE_Key"
#define FWL_MSG_Cursor L"FWL_MESSAGE_Cursor"
#define FWL_MSG_Size L"FWL_MESSAGE_Size"
#define FWL_MSG_WindowMove L"FWL_MESSAGE_WindowMove"
#define FWL_MSG_DropFiles L"FWL_MESSAGE_DropFiles"
#define FWL_MSG_TaskClicked L"FWL_MESSAGE_TaskClicked"
#define FWL_MSG_Close L"FWL_MESSAGE_Close"
#define FWL_MSG_Post L"FWL_MESSAGE_Post"
#define FWL_MSG_WindowWillMove L"FWL_MESSAGE_WindowWillMove"
#define FWL_MSGHASH_Activate 2410369469
#define FWL_MSGHASH_Deactivate 1184214790
#define FWL_MSGHASH_SetFocus 4174512504
#define FWL_MSGHASH_KillFocus 1557903832
#define FWL_MSGHASH_Mouse 706128309
#define FWL_MSGHASH_MouseWheel 893703466
#define FWL_MSGHASH_Key 3751372405
#define FWL_MSGHASH_Cursor 3182626218
#define FWL_MSGHASH_Size 160077735
#define FWL_MSGHASH_WindowMove 1032269377
#define FWL_MSGHASH_DropFiles 2004165236
#define FWL_MSGHASH_TaskClicked 3128231086
#define FWL_MSGHASH_Close 2977563906
#define FWL_MSGHASH_Post 1969633074
#define FWL_MSGHASH_WindowWillMove 2229175763
#define FWL_EVT_Mouse L"FWL_EVENT_Mouse"
#define FWL_EVT_MouseWheel L"FWL_EVENT_MouseWheel"
#define FWL_EVT_Key L"FWL_EVENT_Key"
#define FWL_EVT_SetFocus L"FWL_EVENT_SetFocus"
#define FWL_EVT_KillFocus L"FWL_EVENT_KillFocus"
#define FWL_EVT_Click L"FWL_EVENT_Click"
#define FWL_EVT_Draw L"FWL_EVENT_Draw"
#define FWL_EVT_Scroll L"FWL_EVENT_Scroll"
#define FWL_EVT_Close L"FWL_EVENT_Close"
#define FWL_EVT_ContextMenu L"FWL_EVENT_ContextMenu"
#define FWL_EVT_MenuCommand L"FWL_EVENT_MenuCommand"
#define FWL_EVT_SizeChanged L"FWL_EVENT_SizeChanged"
#define FWL_EVTHASH_Mouse 1765258002
#define FWL_EVTHASH_MouseWheel 3907114407
#define FWL_EVTHASH_Key 2408354450
#define FWL_EVTHASH_SetFocus 3909721269
#define FWL_EVTHASH_KillFocus 1779363253
#define FWL_EVTHASH_Draw 2430713303
#define FWL_EVTHASH_Click 4026328783
#define FWL_EVTHASH_Scroll 2965158968
#define FWL_EVTHASH_Close 4036693599
#define FWL_EVTHASH_ContextMenu 2717307715
#define FWL_EVTHASH_MenuCommand 497763741
#define FWL_EVTHASH_SizeChanged 3083958510
#define FWL_EVTHASH_Idle 839546759
#define FWL_MSGMOUSECMD_LButtonDown 1
#define FWL_MSGMOUSECMD_LButtonUp 2
#define FWL_MSGMOUSECMD_LButtonDblClk 3
#define FWL_MSGMOUSECMD_RButtonDown 4
#define FWL_MSGMOUSECMD_RButtonUp 5
#define FWL_MSGMOUSECMD_RButtonDblClk 6
#define FWL_MSGMOUSECMD_MButtonDown 7
#define FWL_MSGMOUSECMD_MButtonUp 8
#define FWL_MSGMOUSECMD_MButtonDblClk 9
#define FWL_MSGMOUSECMD_MouseMove 10
#define FWL_MSGMOUSECMD_MouseEnter 11
#define FWL_MSGMOUSECMD_MouseLeave 12
#define FWL_MSGMOUSECMD_MouseHover 13
#define FWL_MSGKEYCMD_KeyDown 1
#define FWL_MSGKEYCMD_KeyUp 2
#define FWL_MSGKEYCMD_Char 3
#define FWL_KEYFLAG_Ctrl (1 << 0)
#define FWL_KEYFLAG_Alt (1 << 1)
#define FWL_KEYFLAG_Shift (1 << 2)
#define FWL_KEYFLAG_Command (1 << 3)
#define FWL_KEYFLAG_LButton (1 << 4)
#define FWL_KEYFLAG_RButton (1 << 5)
#define FWL_KEYFLAG_MButton (1 << 6)

// Separate hierarchy not related to IFWL_* hierarchy. These should not
// get cast to IFWL_* types.
class CFWL_Note {
 public:
  virtual FX_DWORD Release() {
    m_dwRefCount--;
    FX_DWORD dwRefCount = m_dwRefCount;
    if (!m_dwRefCount) {
      delete this;
    }
    return dwRefCount;
  }
  virtual CFWL_Note* Retain() {
    m_dwRefCount++;
    return this;
  }
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const {
    wsClass = L"CFWL_Note";
    return FWL_ERR_Succeeded;
  }
  virtual FX_DWORD GetClassID() const { return 0; }
  virtual FX_BOOL IsInstance(const CFX_WideStringC& wsClass) const {
    return TRUE;
  }
  virtual CFWL_Note* Clone() { return NULL; }
  FX_BOOL IsEvent() const { return m_bIsEvent; }
  IFWL_Widget* m_pSrcTarget;
  IFWL_Widget* m_pDstTarget;

 protected:
  CFWL_Note(FX_BOOL bIsEvent)
      : m_pSrcTarget(NULL),
        m_pDstTarget(NULL),
        m_dwRefCount(1),
        m_bIsEvent(bIsEvent),
        m_dwExtend(0) {}
  virtual ~CFWL_Note() {}
  virtual FX_BOOL Initialize() { return TRUE; }
  virtual int32_t Finalize() { return 0; }
  FX_DWORD m_dwRefCount;
  FX_BOOL m_bIsEvent;

 public:
  FX_DWORD m_dwExtend;
};
class CFWL_Message : public CFWL_Note {
 public:
  CFWL_Message() : CFWL_Note(FALSE) {}
  virtual ~CFWL_Message() {}
};
#define BEGIN_FWL_MESSAGE_DEF(classname, msghashcode)             \
  class classname : public CFWL_Message {                         \
   public:                                                        \
    classname() : CFWL_Message() {}                               \
    virtual CFWL_Note* Clone() { return new classname(*this); }   \
    virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const { \
      wsClass = L## #classname;                                   \
      return FWL_ERR_Succeeded;                                   \
    }                                                             \
    virtual FX_DWORD GetClassID() const { return msghashcode; }
#define END_FWL_MESSAGE_DEF \
  }                         \
  ;
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgActivate, FWL_MSGHASH_Activate)
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgDeactivate, FWL_MSGHASH_Deactivate)
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgMouse, FWL_MSGHASH_Mouse)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_DWORD m_dwFlags;
FX_DWORD m_dwCmd;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgMouseWheel, FWL_MSGHASH_MouseWheel)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_FLOAT m_fDeltaX;
FX_FLOAT m_fDeltaY;
FX_DWORD m_dwFlags;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgSetFocus, FWL_MSGHASH_SetFocus)
IFWL_Widget* m_pKillFocus;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgKillFocus, FWL_MSGHASH_KillFocus)
IFWL_Widget* m_pSetFocus;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgKey, FWL_MSGHASH_Key)
FX_DWORD m_dwKeyCode;
FX_DWORD m_dwFlags;
FX_DWORD m_dwCmd;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgCursor, FWL_MSGHASH_Cursor)
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgSize, FWL_MSGHASH_Size)
int32_t m_iWidth;
int32_t m_iHeight;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgWindowMove, FWL_MSGHASH_WindowMove)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgDropFiles, FWL_MSGHASH_DropFiles)
CFWL_MsgDropFiles(const CFWL_MsgDropFiles& copy) {
  m_pDstTarget = copy.m_pDstTarget;
  m_pSrcTarget = copy.m_pSrcTarget;
  m_fx = copy.m_fx;
  m_fy = copy.m_fy;
  m_files.Append(copy.m_files);
}
FX_FLOAT m_fx;
FX_FLOAT m_fy;
CFX_WideStringArray m_files;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgTaskClicked, FWL_MSGHASH_TaskClicked)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgClose, FWL_MSGHASH_Close)
END_FWL_MESSAGE_DEF
BEGIN_FWL_MESSAGE_DEF(CFWL_MsgWindowWillMove, FWL_MSGHASH_WindowWillMove)
END_FWL_MESSAGE_DEF
class CFWL_Event : public CFWL_Note {
 public:
  CFWL_Event() : CFWL_Note(TRUE) {}
  virtual ~CFWL_Event() {}
};
#define BEGIN_FWL_EVENT_DEF(classname, eventhashcode)             \
  class classname : public CFWL_Event {                           \
   public:                                                        \
    classname() : CFWL_Event() {}                                 \
    virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const { \
      wsClass = L## #classname;                                   \
      return FWL_ERR_Succeeded;                                   \
    }                                                             \
    virtual FX_DWORD GetClassID() const { return eventhashcode; }
#define END_FWL_EVENT_DEF \
  }                       \
  ;
BEGIN_FWL_EVENT_DEF(CFWL_EvtMouse, FWL_EVTHASH_Mouse)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_DWORD m_dwFlags;
FX_DWORD m_dwCmd;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtMouseWheel, FWL_EVTHASH_MouseWheel)
FX_FLOAT m_fx;
FX_FLOAT m_fy;
FX_FLOAT m_fDeltaX;
FX_FLOAT m_fDeltaY;
FX_DWORD m_dwFlags;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtKey, FWL_EVTHASH_Key)
FX_DWORD m_dwKeyCode;
FX_DWORD m_dwFlags;
FX_DWORD m_dwCmd;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtSetFocus, FWL_EVTHASH_SetFocus)
IFWL_Widget* m_pSetFocus;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtKillFocus, FWL_EVTHASH_KillFocus)
IFWL_Widget* m_pKillFocus;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtDraw, FWL_EVTHASH_Draw)
CFX_Graphics* m_pGraphics;
IFWL_Widget* m_pWidget;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtClick, FWL_EVTHASH_Click)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtScroll, FWL_EVTHASH_Scroll)
FX_DWORD m_iScrollCode;
FX_FLOAT m_fPos;
FX_BOOL* m_pRet;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtClose, FWL_EVTHASH_Close)
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtContextMenu, FWL_EVTHASH_ContextMenu)
FX_FLOAT m_fPosX;
FX_FLOAT m_fPosY;
IFWL_Widget* m_pOwner;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtMenuCommand, FWL_EVTHASH_MenuCommand)
int32_t m_iCommand;
void* m_pData;
END_FWL_EVENT_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtSizeChanged, FWL_EVTHASH_SizeChanged)
IFWL_Widget* m_pWidget;
CFX_RectF m_rtOld;
CFX_RectF m_rtNew;
END_FWL_MESSAGE_DEF
BEGIN_FWL_EVENT_DEF(CFWL_EvtIdle, FWL_EVTHASH_Idle)
END_FWL_EVENT_DEF
typedef enum {
  FWL_EVENT_MOUSE_MASK = 1 << 0,
  FWL_EVENT_MOUSEWHEEL_MASK = 1 << 1,
  FWL_EVENT_KEY_MASK = 1 << 2,
  FWL_EVENT_FOCUSCHANGED_MASK = 1 << 3,
  FWL_EVENT_DRAW_MASK = 1 << 4,
  FWL_EVENT_CLOSE_MASK = 1 << 5,
  FWL_EVENT_SIZECHANGED_MASK = 1 << 6,
  FWL_EVENT_IDLE_MASK = 1 << 7,
  FWL_EVENT_CONTROL_MASK = 1 << 8,
  FWL_EVENT_ALL_MASK = 0xFF
} FWLEventMask;
class IFWL_NoteDriver {
 public:
  virtual ~IFWL_NoteDriver() {}
  virtual FX_BOOL SendNote(CFWL_Note* pNote) = 0;
  virtual FX_BOOL PostMessage(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR RegisterEventTarget(
      IFWL_Widget* pListener,
      IFWL_Widget* pEventSource = NULL,
      FX_DWORD dwFilter = FWL_EVENT_ALL_MASK) = 0;
  virtual FWL_ERR UnregisterEventTarget(IFWL_Widget* pListener) = 0;
  virtual void ClearEventTargets(FX_BOOL bRemoveAll) = 0;
  virtual int32_t GetQueueMaxSize() const = 0;
  virtual FWL_ERR SetQueueMaxSize(const int32_t size) = 0;
  virtual IFWL_NoteThread* GetOwnerThread() const = 0;
  virtual FWL_ERR PushNoteLoop(IFWL_NoteLoop* pNoteLoop) = 0;
  virtual IFWL_NoteLoop* PopNoteLoop() = 0;
  virtual IFWL_Widget* GetFocus() = 0;
  virtual FX_BOOL SetFocus(IFWL_Widget* pFocus, FX_BOOL bNotify = FALSE) = 0;
  virtual void SetGrab(IFWL_Widget* pGrab, FX_BOOL bSet) = 0;
  virtual FWL_ERR Run() = 0;
};
IFWL_Widget* FWL_GetCurrentThreadModalWidget(IFWL_NoteThread* pNoteThread);
class IFWL_NoteLoop {
 public:
  virtual ~IFWL_NoteLoop() {}
  virtual FX_BOOL PreProcessMessage(CFWL_Message* pMessage) = 0;
  virtual FWL_ERR Idle(int32_t count) = 0;
};
class IFWL_ToolTipTarget {
 public:
  virtual ~IFWL_ToolTipTarget() {}
  virtual IFWL_Widget* GetWidget() = 0;
  virtual FX_BOOL IsShowed() = 0;
  virtual FWL_ERR DrawToolTip(CFX_Graphics* pGraphics,
                              const CFX_Matrix* pMatrix,
                              IFWL_Widget* pToolTip) = 0;
  virtual FX_BOOL UseDefaultTheme() = 0;
  virtual FWL_ERR GetCaption(CFX_WideString& wsCaption) = 0;
  virtual FWL_ERR GetToolTipSize(CFX_SizeF& sz) = 0;
  virtual FWL_ERR GetToolTipPos(CFX_PointF& pt) { return FWL_ERR_Indefinite; }
};
FWL_ERR FWL_AddToolTipTarget(IFWL_ToolTipTarget* pTarget);
FWL_ERR FWL_RemoveToolTipTarget(IFWL_ToolTipTarget* pTarget);
FWL_ERR FWL_SetToolTipInitialDelay(int32_t iDelayTime);
FWL_ERR FWL_SetToolTipAutoPopDelay(int32_t iDelayTime);
typedef FX_BOOL (*FWLMessageHookCallback)(CFWL_Message* msg, void* info);
FWL_ERR FWL_SetHook(IFWL_NoteDriver* driver,
                    FWLMessageHookCallback callback,
                    void* info);
#endif
