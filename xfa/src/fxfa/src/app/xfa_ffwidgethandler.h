// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_ANNOTHANDLER_IMP_H
#define _FXFA_FORMFILLER_ANNOTHANDLER_IMP_H
class CXFA_FFDocView;
class CXFA_FFWidgetHandler : public IXFA_WidgetHandler, public CFX_Object
{
public:
    CXFA_FFWidgetHandler(CXFA_FFDocView* pDocView);
    ~CXFA_FFWidgetHandler();
    virtual XFA_HWIDGET		CreateWidget(XFA_HWIDGET hParent, XFA_WIDGETTYPE eType, XFA_HWIDGET hBefore = NULL);
    virtual IXFA_PageView*	GetPageView(XFA_HWIDGET hWidget);
    virtual void			GetRect(XFA_HWIDGET hWidget, CFX_RectF &rt);
    virtual FX_DWORD		GetStatus(XFA_HWIDGET hWidget);
    virtual FX_BOOL			GetBBox(XFA_HWIDGET hWidget, CFX_RectF &rtBox, FX_DWORD dwStatus, FX_BOOL bDrawFocus = FALSE);
    virtual CXFA_WidgetAcc*	GetDataAcc(XFA_HWIDGET hWidget);
    virtual void			GetName(XFA_HWIDGET hWidget, CFX_WideString &wsName, FX_INT32 iNameType = 0);
    virtual	FX_BOOL			GetToolTip(XFA_HWIDGET hWidget, CFX_WideString &wsToolTip);
    virtual	void			SetPrivateData(XFA_HWIDGET hWidget, FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback);
    virtual	FX_LPVOID		GetPrivateData(XFA_HWIDGET hWidget, FX_LPVOID module_id);
    virtual FX_BOOL		OnMouseEnter(XFA_HWIDGET hWidget);
    virtual FX_BOOL		OnMouseExit(XFA_HWIDGET hWidget);
    virtual FX_BOOL		OnLButtonDown(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnLButtonUp(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnLButtonDblClk(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnMouseMove(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnMouseWheel(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_SHORT zDelta, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnRButtonDown(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnRButtonUp(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnRButtonDblClk(XFA_HWIDGET hWidget, FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);

    virtual FX_BOOL		OnKeyDown(XFA_HWIDGET hWidget, FX_DWORD dwKeyCode, FX_DWORD dwFlags);
    virtual FX_BOOL		OnKeyUp(XFA_HWIDGET hWidget, FX_DWORD dwKeyCode, FX_DWORD dwFlags);
    virtual FX_BOOL		OnChar(XFA_HWIDGET hWidget, FX_DWORD dwChar, FX_DWORD dwFlags);
    virtual	FX_DWORD	OnHitTest(XFA_HWIDGET hWidget, FX_FLOAT fx, FX_FLOAT fy);
    virtual FX_BOOL		OnSetCursor(XFA_HWIDGET hWidget, FX_FLOAT fx, FX_FLOAT fy);
    virtual void		RenderWidget(XFA_HWIDGET hWidget, CFX_Graphics* pGS, CFX_Matrix* pMatrix = NULL, FX_BOOL bHighlight = FALSE);
    virtual FX_BOOL		HasEvent(CXFA_WidgetAcc* pWidgetAcc, XFA_EVENTTYPE eEventType);
    virtual FX_INT32	ProcessEvent(CXFA_WidgetAcc* pWidgetAcc, CXFA_EventParam* pParam);
protected:
    CXFA_Node*		CreateWidgetFormItem(XFA_WIDGETTYPE eType, CXFA_Node* pParent, CXFA_Node* pBefore) const;

    CXFA_Node*		CreatePushButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateCheckButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateExclGroup(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateRadioButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateDatetimeEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateDecimalField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateNumericField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateSignature(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateTextEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateDropdownList(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateListBox(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateImageField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreatePasswordEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateField(XFA_ELEMENT eElement, CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateArc(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateRectangle(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateImage(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateLine(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateText(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateDraw(XFA_ELEMENT eElement, CXFA_Node* pParent, CXFA_Node* pBefore) const;

    CXFA_Node*		CreateSubform(CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*		CreateFormItem(XFA_ELEMENT eElement, CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*			CreateCopyNode(XFA_ELEMENT eElement, CXFA_Node* pParent, CXFA_Node* pBefore = NULL) const;
    CXFA_Node*			CreateTemplateNode(XFA_ELEMENT eElement, CXFA_Node* pParent, CXFA_Node* pBefore) const;
    CXFA_Node*			CreateFontNode(CXFA_Node* pParent) const;
    CXFA_Node*			CreateMarginNode(CXFA_Node* pParent, FX_DWORD dwFlags, FX_FLOAT fInsets[4]) const;
    CXFA_Node*			CreateValueNode(XFA_ELEMENT eValue, CXFA_Node* pParent) const;
    IXFA_ObjFactory*	GetObjFactory() const;
    CXFA_Document*		GetXFADoc() const;

    CXFA_FFDocView*		m_pDocView;
};
class CXFA_FFMenuHandler : public IXFA_MenuHandler, public CFX_Object
{
public:
    CXFA_FFMenuHandler();
    ~CXFA_FFMenuHandler();
    virtual FX_BOOL		CanCopy(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanCut(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanPaste(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanSelectAll(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanDelete(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanDeSelect(XFA_HWIDGET hWidget);
    virtual FX_BOOL		Copy(XFA_HWIDGET hWidget, CFX_WideString &wsText);
    virtual FX_BOOL		Cut(XFA_HWIDGET hWidget, CFX_WideString &wsText);
    virtual FX_BOOL		Paste(XFA_HWIDGET hWidget, const CFX_WideString &wsText);
    virtual FX_BOOL		SelectAll(XFA_HWIDGET hWidget);
    virtual FX_BOOL		Delete(XFA_HWIDGET hWidget);
    virtual FX_BOOL		DeSelect(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanUndo(XFA_HWIDGET hWidget);
    virtual FX_BOOL		CanRedo(XFA_HWIDGET hWidget);
    virtual FX_BOOL		Undo(XFA_HWIDGET hWidget);
    virtual FX_BOOL		Redo(XFA_HWIDGET hWidget);
    virtual FX_BOOL		GetSuggestWords(XFA_HWIDGET hWidget, CFX_PointF pointf, CFX_ByteStringArray &sSuggest);
    virtual FX_BOOL		ReplaceSpellCheckWord(XFA_HWIDGET hWidget, CFX_PointF pointf, FX_BSTR bsReplace);
};
#endif
