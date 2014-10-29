// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FFNOTIFY_H_
#define _FXFA_FFNOTIFY_H_
class CXFA_FFNotify : public IXFA_Notify, public CFX_Object
{
public:
    CXFA_FFNotify(CXFA_FFDoc* pDoc);
    ~CXFA_FFNotify();

    virtual void				OnPageEvent(IXFA_LayoutPage *pSender, XFA_PAGEEVENT eEvent, FX_LPVOID pParam = NULL);

    virtual void				OnNodeEvent(CXFA_Node *pSender, XFA_NODEEVENT eEvent, FX_LPVOID pParam = NULL, FX_LPVOID pParam2 = NULL, FX_LPVOID pParam3 = NULL, FX_LPVOID pParam4 = NULL);
    virtual void		        OnWidgetDataEvent(CXFA_WidgetData* pSender, FX_DWORD dwEvent, FX_LPVOID pParam = NULL, FX_LPVOID pAdditional = NULL, FX_LPVOID pAdditional2 = NULL);
    virtual CXFA_LayoutItem*	OnCreateLayoutItem(CXFA_Node *pNode);
    virtual void				OnLayoutEvent(IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, XFA_LAYOUTEVENT eEvent, FX_LPVOID pParam = NULL, FX_LPVOID pParam2 = NULL);

    virtual void				StartFieldDrawLayout(CXFA_Node *pItem, FX_FLOAT &fCalcWidth, FX_FLOAT &fCalcHeight);
    virtual FX_BOOL				FindSplitPos(CXFA_Node *pItem, FX_INT32 iBlockIndex, FX_FLOAT &fCalcHeightPos);
    virtual FX_BOOL				RunScript(CXFA_Node* pScript, CXFA_Node* pFormItem);
    virtual	FX_INT32			ExecEventByDeepFirst(CXFA_Node* pFormNode, XFA_EVENTTYPE eEventType, FX_BOOL bIsFormReady = FALSE, FX_BOOL bRecursive = TRUE, CXFA_WidgetAcc	* pExclude = NULL);
    virtual void				AddCalcValidate(CXFA_Node* pNode);
    virtual XFA_HDOC			GetHDOC();
    virtual IXFA_DocProvider*	GetDocProvider();
    virtual IXFA_AppProvider*	GetAppProvider();
    virtual IXFA_WidgetHandler*	GetWidgetHandler();
    virtual XFA_HWIDGET			GetHWidget(CXFA_LayoutItem* pLayoutItem);
    virtual void				OpenDropDownList(XFA_HWIDGET hWidget);
    virtual CFX_WideString		GetCurrentDateTime();
    virtual void				ResetData(CXFA_WidgetData* pWidgetData = NULL);
    virtual FX_INT32			GetLayoutStatus();
    virtual void				RunNodeInitialize(CXFA_Node* pNode);
    virtual void				RunSubformIndexChange(CXFA_Node* pSubformNode);
    virtual CXFA_Node*			GetFocusWidgetNode();
    virtual void				SetFocusWidgetNode(CXFA_Node* pNode);
protected:
    void	OnNodeReady(CXFA_Node *pNode);
    void	OnValueChanging(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnValueChanged(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2, FX_LPVOID pParam3, FX_LPVOID pParam4);
    void	OnChildAdded(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnChildRemoved(CXFA_Node *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnLayoutItemAdd(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnLayoutItemRemoving(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnLayoutItemRectChanged(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    void	OnLayoutItemStatustChanged(CXFA_FFDocView* pDocView, IXFA_DocLayout *pLayout, CXFA_LayoutItem *pSender, FX_LPVOID pParam, FX_LPVOID pParam2);
    CXFA_FFDoc*			m_pDoc;
};
#endif
