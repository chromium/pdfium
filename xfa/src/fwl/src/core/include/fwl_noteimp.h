// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_NOTE_IMP_H
#define _FWL_NOTE_IMP_H
class CFWL_Target;
class CFWL_WidgetImp;
class CFWL_NoteThread;
class CFWL_ToolTipImp;
class IFWL_ToolTipTarget;
class CFWL_CoreToopTipDP;
class CFWL_NoteLoop;
class CFWL_NoteDriver;
class CFWL_EventTarget;
class CFWL_ToolTipContainer;
class CFWL_NoteLoop : public CFX_Object
{
public:
    CFWL_NoteLoop(CFWL_WidgetImp *pForm = NULL);
    virtual FX_BOOL PreProcessMessage(CFWL_Message *pMessage);
    virtual FWL_ERR	Idle(FX_INT32 count);
    CFWL_WidgetImp*	GetForm();
    FX_BOOL	ContinueModal();
    FWL_ERR	EndModalLoop();
    FX_BOOL TranslateAccelerator(CFWL_Message *pMessage);
    FWL_ERR	SetMainForm(CFWL_WidgetImp *pForm);
protected:
    void GenerateCommondEvent(FX_DWORD dwCommand);

    CFWL_WidgetImp	*m_pForm;
    FX_BOOL			m_bContinueModal;
};
class CFWL_NoteDriver : public CFX_Object
{
public:
    CFWL_NoteDriver();
    ~CFWL_NoteDriver();
    virtual FX_BOOL		SendNote(CFWL_Note *pNote);
    virtual FX_BOOL		PostMessage(CFWL_Message *pMessage);
    virtual FWL_ERR		RegisterEventTarget(IFWL_Widget *pListener, IFWL_Widget *pEventSource = NULL, FX_DWORD dwFilter = FWL_EVENT_ALL_MASK);
    virtual FWL_ERR		UnregisterEventTarget(IFWL_Widget *pListener);
    virtual	void		ClearEventTargets(FX_BOOL bRemoveAll);
    virtual FX_INT32	GetQueueMaxSize() const;
    virtual FWL_ERR		SetQueueMaxSize(const FX_INT32 size);
    virtual IFWL_NoteThread* GetOwnerThread() const;
    virtual FWL_ERR		PushNoteLoop(IFWL_NoteLoop *pNoteLoop);
    virtual IFWL_NoteLoop* PopNoteLoop();
    virtual FX_BOOL		SetFocus(IFWL_Widget *pFocus, FX_BOOL bNotify = FALSE);
    virtual	FWL_ERR		Run();
    IFWL_Widget*		GetFocus();
    IFWL_Widget*		GetHover();
    void				SetHover(IFWL_Widget *pHover);
    void				SetGrab(IFWL_Widget *pGrab, FX_BOOL bSet);
    void				NotifyTargetHide(IFWL_Widget *pNoteTarget);
    void				NotifyTargetDestroy(IFWL_Widget *pNoteTarget);
    void				NotifyFullScreenMode(IFWL_Widget *pNoteTarget, FX_BOOL bFullScreen);
    FWL_ERR				RegisterForm(CFWL_WidgetImp *pForm);
    FWL_ERR				UnRegisterForm(CFWL_WidgetImp *pForm);
    FX_BOOL				QueueMessage(CFWL_Message *pMessage);
    FX_BOOL				UnqueueMessage(CFWL_NoteLoop *pNoteLoop);
    CFWL_NoteLoop*		GetTopLoop();
    FX_INT32            CountLoop();
    void                SetHook(FWLMessageHookCallback callback, FX_LPVOID info);
    FX_BOOL				ProcessMessage(CFWL_Message *pMessage);
protected:
    FX_BOOL				DispatchMessage(CFWL_Message *pMessage, IFWL_Widget *pMessageForm);
    FX_BOOL				DoActivate(CFWL_MsgActivate *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoDeactivate(CFWL_MsgDeactivate *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoSetFocus(CFWL_MsgSetFocus *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoKillFocus(CFWL_MsgKillFocus *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoKey(CFWL_MsgKey *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoMouse(CFWL_MsgMouse *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoWheel(CFWL_MsgMouseWheel *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoSize(CFWL_MsgSize *pMsg);
    FX_BOOL				DoWindowMove(CFWL_MsgWindowMove *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoDragFiles(CFWL_MsgDropFiles *pMsg, IFWL_Widget *pMessageForm);
    FX_BOOL				DoMouseEx(CFWL_MsgMouse *pMsg, IFWL_Widget *pMessageForm);
    void				MouseSecondary(CFWL_MsgMouse *pMsg);
    FX_BOOL				IsValidMessage(CFWL_Message *pMessage);
    IFWL_Widget*		GetMessageForm(IFWL_Widget *pDstTarget);
    void				ClearInvalidEventTargets(FX_BOOL bRemoveAll);
    CFX_PtrArray		m_forms;
    CFX_PtrArray		m_noteQueue;
    CFX_PtrArray		m_noteLoopQueue;
    CFX_MapPtrToPtr		m_eventTargets;
    FX_INT32			m_sendEventCalled;
    FX_INT32			m_maxSize;
    FX_BOOL				m_bFullScreen;
    IFWL_Widget			*m_pHover;
    IFWL_Widget			*m_pFocus;
    IFWL_Widget			*m_pGrab;
    CFWL_NoteLoop		*m_pNoteLoop;
    FWLMessageHookCallback m_hook;
    FX_LPVOID              m_hookInfo;
};
typedef CFX_MapPtrTemplate<FX_LPVOID, FX_DWORD> CFWL_EventSource;
class CFWL_EventTarget : public CFX_Object
{
public:
    CFWL_EventTarget(CFWL_NoteDriver *pNoteDriver, IFWL_Widget *pListener)
        : m_pNoteDriver(pNoteDriver)
        , m_pListener(pListener)
        , m_bInvalid(FALSE)
    {
    }
    ~CFWL_EventTarget();
    FX_INT32 SetEventSource(IFWL_Widget *pSource, FX_DWORD dwFilter = FWL_EVENT_ALL_MASK);
    FX_BOOL  ProcessEvent(CFWL_Event *pEvent);
    FX_BOOL  IsFilterEvent(CFWL_Event *pEvent, FX_DWORD dwFilter);
    FX_BOOL	 IsInvalid()
    {
        return m_bInvalid;
    }
    void	 FlagInvalid()
    {
        m_bInvalid = TRUE;
    }
protected:
    CFWL_EventSource m_eventSources;
    IFWL_Widget *m_pListener;
    CFWL_NoteDriver *m_pNoteDriver;
    FX_BOOL	m_bInvalid;
};
class CFWL_ToolTipContainer: public CFX_Object
{
public:
    virtual ~CFWL_ToolTipContainer();
    static CFWL_ToolTipContainer* getInstance();

    FX_ERR		AddToolTipTarget(IFWL_ToolTipTarget *pTarget);
    FX_ERR		RemoveToolTipTarget(IFWL_ToolTipTarget *pTarget);
    IFWL_ToolTipTarget* GetCurrentToolTipTarget();

    FX_BOOL HasToolTip(IFWL_Widget *pWidget);

    FX_BOOL ProcessEnter(CFWL_EvtMouse *pEvt, IFWL_Widget *pOwner);
    FX_BOOL ProcessLeave(CFWL_EvtMouse *pEvt);

    FX_ERR  SetToolTipInitialDelay(FX_INT32 iDelayTime);
    FX_ERR	SetToolTipAutoPopDelay(FX_INT32 iDelayTime);
protected:
    CFWL_ToolTipContainer();

    IFWL_ToolTipTarget		*pCurTarget;
    CFWL_ToolTipImp			*m_pToolTipImp;
    CFWL_CoreToopTipDP		*m_ToolTipDp;
    CFX_PtrArray			m_arrWidget;
};
#endif
