// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_WIDGET_IMP_H
#define _FWL_WIDGET_IMP_H
class CFWL_NoteTarget;
class CFWL_NoteThread;
class CFWL_WidgetImpProperties;
class CFWL_WidgetMgr;
class IFWL_Widget;
class IFWL_ThemeProvider;
class IFWL_DataProvider;
class IFWL_WidgetDelegate;
class CFWL_WidgetImp;
class CFWL_WidgetImp : public CFWL_Target
{
public:
    virtual FWL_ERR		Initialize();
    virtual FWL_ERR		Finalize();

    virtual FWL_ERR		GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual FWL_ERR		GetGlobalRect(CFX_RectF &rect);
    virtual FWL_ERR		SetWidgetRect(const CFX_RectF &rect);
    virtual	FWL_ERR		GetClientRect(CFX_RectF &rect);
    virtual	IFWL_Widget* GetParent();
    virtual FWL_ERR		 SetParent(IFWL_Widget *pParent);
    virtual	IFWL_Widget* GetOwner();
    virtual FWL_ERR		 SetOwner(IFWL_Widget *pOwner);
    virtual IFWL_Widget* GetOuter();
    virtual FX_DWORD	GetStyles();
    virtual FWL_ERR		ModifyStyles(FX_DWORD dwStylesAdded, FX_DWORD dwStylesRemoved);
    virtual FX_DWORD	GetStylesEx();
    virtual FWL_ERR		ModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved);
    virtual FX_DWORD	GetStates();
    virtual FWL_ERR		SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
    virtual FWL_ERR		SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback);
    virtual FX_LPVOID	GetPrivateData(FX_LPVOID module_id);
    virtual	FWL_ERR		Update();
    virtual FWL_ERR		LockUpdate();
    virtual FWL_ERR		UnlockUpdate();
    virtual FX_DWORD	HitTest(FX_FLOAT fx, FX_FLOAT fy);
    virtual	FWL_ERR		TransformTo(IFWL_Widget *pWidget, FX_FLOAT &fx, FX_FLOAT &fy);
    virtual FWL_ERR		TransformTo(IFWL_Widget *pWidget, CFX_RectF &rt);
    virtual FWL_ERR		GetMatrix(CFX_Matrix &matrix, FX_BOOL bGlobal = FALSE);
    virtual FWL_ERR		SetMatrix(const CFX_Matrix &matrix);
    virtual FWL_ERR		DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
    virtual IFWL_ThemeProvider*	GetThemeProvider();
    virtual FWL_ERR				SetThemeProvider(IFWL_ThemeProvider *pThemeProvider);
    virtual FWL_ERR				SetDataProvider(IFWL_DataProvider *pDataProvider);
    virtual IFWL_WidgetDelegate*SetDelegate(IFWL_WidgetDelegate *pDelegate);
    virtual	IFWL_NoteThread*	GetOwnerThread() const;
    FWL_ERR				SetOwnerThread(CFWL_NoteThread *pOwnerThread);
    FWL_ERR				GetProperties(CFWL_WidgetImpProperties &properties);
    FWL_ERR				SetProperties(const CFWL_WidgetImpProperties &properties);
    IFWL_Widget*		GetInterface();
    void				SetInterface(IFWL_Widget *pInterface);
    CFX_SizeF			GetOffsetFromParent(IFWL_Widget *pParent);
protected:
    CFWL_WidgetImp(IFWL_Widget *pOuter = NULL);
    CFWL_WidgetImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    virtual ~CFWL_WidgetImp();
    FX_BOOL	IsEnabled();
    FX_BOOL	IsVisible();
    FX_BOOL IsActive();
    FX_BOOL IsOverLapper();
    FX_BOOL IsPopup();
    FX_BOOL IsChild();
    FX_BOOL	IsLocked();
    FX_BOOL IsOffscreen();
    FX_BOOL	HasBorder();
    FX_BOOL	HasEdge();
    void		GetEdgeRect(CFX_RectF &rtEdge);
    FX_FLOAT	GetBorderSize(FX_BOOL bCX = TRUE);
    FX_FLOAT	GetEdgeWidth();
    void		GetRelativeRect(CFX_RectF &rect);
    FX_LPVOID	GetThemeCapacity(FX_DWORD dwCapacity);
    IFWL_ThemeProvider* GetAvailableTheme();
    CFWL_WidgetImp*	GetRootOuter();
    CFWL_WidgetImp*	GetSameAncestor(CFWL_WidgetImp *pWidget);
    CFX_SizeF	GetOffsetFromAncestor(CFWL_WidgetImp *pAncestor);
    FX_BOOL		TransformToOuter(FX_FLOAT &fx, FX_FLOAT &fy);
    FX_BOOL		TransformFromOuter(FX_FLOAT &fx, FX_FLOAT &fy);
    CFX_SizeF	CalcTextSize(const CFX_WideString &wsText, IFWL_ThemeProvider *pTheme, FX_BOOL bMultiLine = FALSE, FX_INT32 iLineWidth = -1);
    void		CalcTextRect(const CFX_WideString &wsText, IFWL_ThemeProvider *pTheme, FX_DWORD dwTTOStyles, FX_INT32 iTTOAlign, CFX_RectF &rect);
    void		SetFocus(FX_BOOL bFocus);
    void		SetGrab(FX_BOOL bSet);
    FX_BOOL		GetPopupPos(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
    FX_BOOL		GetPopupPosMenu(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
    FX_BOOL		GetPopupPosComboBox(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
    FX_BOOL		GetPopupPosGeneral(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
    FX_BOOL		GetScreenSize(FX_FLOAT &fx, FX_FLOAT &fy);
    void		RegisterEventTarget(IFWL_Widget *pEventSource = NULL, FX_DWORD dwFilter = FWL_EVENT_ALL_MASK);
    void		UnregisterEventTarget();
    void		DispatchKeyEvent(CFWL_MsgKey *pNote);
    void		DispatchEvent(CFWL_Event *pEvent);
    void		Repaint(const CFX_RectF *pRect = NULL);
    void		DrawBackground(CFX_Graphics *pGraphics, FX_INT32 iPartBk, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix = NULL);
    void		DrawBorder(CFX_Graphics *pGraphics, FX_INT32 iPartBorder, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix = NULL);
    void		DrawEdge(CFX_Graphics *pGraphics, FX_INT32 iPartEdge, IFWL_ThemeProvider *pTheme, const CFX_Matrix *pMatrix = NULL);
    void		NotifyDriver();

    FX_BOOL		IsParent(IFWL_Widget *pParent);
    CFWL_WidgetMgr			   *m_pWidgetMgr;
    CFWL_NoteThread			   *m_pOwnerThread;
    CFWL_WidgetImpProperties   *m_pProperties;
    CFX_PrivateData			   *m_pPrivateData;
    IFWL_WidgetDelegate		   *m_pDelegate;
    IFWL_WidgetDelegate		   *m_pCurDelegate;
    IFWL_Widget			       *m_pOuter;
    IFWL_Widget				   *m_pInterface;
    FX_INT32					m_iLock;
    friend class CFWL_WidgetImpDelegate;
    friend void FWL_SetWidgetRect(IFWL_Widget *widget, const CFX_RectF &rect);
    friend void FWL_SetWidgetStates(IFWL_Widget *widget, FX_DWORD dwStates);
    friend void FWL_SetWidgetStyles(IFWL_Widget *widget, FX_DWORD dwStyles);
};
class CFWL_WidgetImpDelegate : public CFX_Object
{
public:
    CFWL_WidgetImpDelegate();
    virtual FX_INT32	OnProcessMessage(CFWL_Message *pMessage);
    virtual FWL_ERR		OnProcessEvent(CFWL_Event *pEvent);
    virtual FWL_ERR		OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
};
#endif
