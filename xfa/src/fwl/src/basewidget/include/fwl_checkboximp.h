// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CHECKBOX_IMP_H
#define _FWL_CHECKBOX_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class CFWL_CheckBoxImp;
class CFWL_CheckBoxImpDelegate;
class CFWL_CheckBoxImp : public CFWL_WidgetImp
{
public:
    CFWL_CheckBoxImp(IFWL_Widget *pOuter = NULL);
    CFWL_CheckBoxImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    ~CFWL_CheckBoxImp();
    virtual FWL_ERR		GetClassName(CFX_WideString &wsClass) const;
    virtual FX_DWORD	GetClassID() const;
    virtual FWL_ERR		Initialize();
    virtual FWL_ERR		Finalize();
    virtual FWL_ERR		GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual	FWL_ERR		Update();
    virtual FWL_ERR		DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
    virtual FX_INT32	GetCheckState();
    virtual FWL_ERR		SetCheckState(FX_INT32 iCheck);
protected:
    void		Layout();
    FX_DWORD	GetPartStates();
    void		UpdateTextOutStyles();
    void		NextStates();
    CFX_RectF	m_rtClient;
    CFX_RectF	m_rtBox;
    CFX_RectF	m_rtCaption;
    CFX_RectF	m_rtFocus;
    FX_DWORD	m_dwTTOStyles;
    FX_INT32	m_iTTOAlign;
    FX_BOOL		m_bBtnDown;
    friend class CFWL_CheckBoxImpDelegate;
};
class CFWL_CheckBoxImpDelegate : public CFWL_WidgetImpDelegate
{
public:
    CFWL_CheckBoxImpDelegate(CFWL_CheckBoxImp *pOwner);
    virtual FX_INT32	OnProcessMessage(CFWL_Message *pMessage);
    virtual FWL_ERR		OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix = NULL);
protected:
    void	OnActivate(CFWL_Message *pMsg);
    void	OnFocusChanged(CFWL_Message *pMsg, FX_BOOL bSet = TRUE);
    void	OnLButtonDown(CFWL_MsgMouse *pMsg);
    void	OnLButtonUp(CFWL_MsgMouse *pMsg);
    void	OnMouseMove(CFWL_MsgMouse *pMsg);
    void	OnMouseLeave(CFWL_MsgMouse *pMsg);
    void	OnKeyDown(CFWL_MsgKey *pMsg);
    CFWL_CheckBoxImp *m_pOwner;
};
#endif
