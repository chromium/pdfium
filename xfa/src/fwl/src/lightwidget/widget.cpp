// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../../src/core/include/fwl_noteimp.h"
#include "../core/include/fwl_targetimp.h"
#include "../core/include/fwl_noteimp.h"
#include "../core/include/fwl_widgetimp.h"
#include "../core/include/fwl_widgetmgrimp.h"
IFWL_Widget* CFWL_Widget::GetWidget()
{
    return m_pImp;
}
FX_DWORD CFWL_Widget::Release()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    FX_DWORD dwRef = m_pImp->GetRefCount();
    if (dwRef == 1) {
        m_pImp->Finalize();
    }
    m_pImp->Release();
    if (dwRef == 1) {
        m_pImp = NULL;
        delete this;
    }
    return dwRef - 1;
}
CFWL_Widget* CFWL_Widget::Retain()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    m_pImp->Retain();
    return this;
}
FX_DWORD CFWL_Widget::GetRefCount() const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 1);
    return m_pImp->GetRefCount();
}
FWL_ERR CFWL_Widget::GetClassName(CFX_WideString &wsClass) const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetClassName(wsClass);
}
FX_DWORD CFWL_Widget::GetClassID() const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return m_pImp->GetClassID();
}
FX_BOOL CFWL_Widget::IsInstance(FX_WSTR wsClass) const
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FALSE);
    return m_pImp->IsInstance(wsClass);
}
static FX_LPVOID gs_pFWLWidget = (FX_LPVOID)FXBSTR_ID('l', 'i', 'g', 't');
FWL_ERR CFWL_Widget::Initialize(const CFWL_WidgetProperties *pProperties )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetPrivateData(gs_pFWLWidget, this, NULL);
}
FWL_ERR CFWL_Widget::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetWidgetRect(rect, bAutoSize);
}
FWL_ERR CFWL_Widget::GetGlobalRect(CFX_RectF &rect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetGlobalRect(rect);
}
FWL_ERR CFWL_Widget::SetWidgetRect(const CFX_RectF &rect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetWidgetRect(rect);
}
FWL_ERR CFWL_Widget::GetClientRect(CFX_RectF &rect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetClientRect(rect);
}
CFWL_Widget* CFWL_Widget::GetParent()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    IFWL_Widget *parent = m_pImp->GetParent();
    if (parent) {
        return (CFWL_Widget*)parent->GetPrivateData(gs_pFWLWidget);
    }
    return NULL;
}
FWL_ERR	CFWL_Widget::SetParent(CFWL_Widget *pParent)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetParent(pParent ? pParent->GetWidget() : NULL);
}
CFWL_Widget* CFWL_Widget::GetOwner()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    return NULL;
}
FWL_ERR	CFWL_Widget::SetOwner(CFWL_Widget *pOwner)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_Widget::GetStyles()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return m_pImp->GetStyles();
}
FWL_ERR	CFWL_Widget::ModifyStyles(FX_DWORD dwStylesAdded, FX_DWORD dwStylesRemoved)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->ModifyStyles(dwStylesAdded, dwStylesRemoved);
}
FX_DWORD CFWL_Widget::GetStylesEx()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return m_pImp->GetStylesEx();
}
FWL_ERR	CFWL_Widget::ModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved)
{
    return m_pImp->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
FX_DWORD CFWL_Widget::GetStates()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetStates();
}
FWL_ERR	CFWL_Widget::SetStates(FX_DWORD dwStates, FX_BOOL bSet )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetStates(dwStates, bSet);
}
FWL_ERR	CFWL_Widget::SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetPrivateData(module_id, pData, callback);
}
FX_LPVOID CFWL_Widget::GetPrivateData(FX_LPVOID module_id)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    return m_pImp->GetPrivateData(module_id);
}
FWL_ERR	CFWL_Widget::Update()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->Update();
}
FWL_ERR	CFWL_Widget::LockUpdate()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->LockUpdate();
}
FWL_ERR	CFWL_Widget::UnlockUpdate()
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->UnlockUpdate();
}
FX_DWORD CFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, 0);
    return m_pImp->HitTest(fx, fy);
}
FWL_ERR	CFWL_Widget::TransformTo(CFWL_Widget *pWidget, FX_FLOAT &fx, FX_FLOAT &fy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->TransformTo(pWidget ? pWidget->GetWidget() : NULL, fx, fy);
}
FWL_ERR	CFWL_Widget::TransformTo(CFWL_Widget *pWidget, CFX_RectF &rt)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->TransformTo(pWidget ? pWidget->GetWidget() : NULL, rt);
}
FWL_ERR	CFWL_Widget::GetMatrix(CFX_Matrix &matrix, FX_BOOL bGlobal )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->GetMatrix(matrix, bGlobal);
}
FWL_ERR	CFWL_Widget::SetMatrix(const CFX_Matrix &matrix)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->SetMatrix(matrix);
}
FWL_ERR	CFWL_Widget::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    return m_pImp->DrawWidget(pGraphics, pMatrix);
}
FWL_ERR	CFWL_Widget::GetProperties(CFWL_WidgetProperties &properties)
{
    properties.m_ctmOnParent = m_pProperties->m_ctmOnParent;
    properties.m_rtWidget = m_pProperties->m_rtWidget;
    properties.m_dwStyles = m_pProperties->m_dwStyles;
    properties.m_dwStyleExes = m_pProperties->m_dwStyleExes;
    properties.m_dwStates = m_pProperties->m_dwStates;
    properties.m_pParent = m_pProperties->m_pParent;
    properties.m_pOwner = m_pProperties->m_pOwner;
    properties.m_wsWindowclass = m_pProperties->m_wsWindowclass;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_Widget::SetProperties(const CFWL_WidgetProperties &properties)
{
    m_pProperties->m_ctmOnParent = properties.m_ctmOnParent;
    m_pProperties->m_rtWidget = properties.m_rtWidget;
    m_pProperties->m_dwStyles = properties.m_dwStyles;
    m_pProperties->m_dwStyleExes = properties.m_dwStyleExes;
    m_pProperties->m_dwStates = properties.m_dwStates;
    m_pProperties->m_pParent = properties.m_pParent;
    m_pProperties->m_pOwner = properties.m_pOwner;
    m_pProperties->m_wsWindowclass = properties.m_wsWindowclass;
    return FWL_ERR_Succeeded;
}
IFWL_WidgetDelegate* CFWL_Widget::SetDelegate(IFWL_WidgetDelegate* pDelegate)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, NULL);
    m_pDelegate = m_pImp->SetDelegate(pDelegate);
    return m_pDelegate;
}
CFWL_Widget::CFWL_Widget()
    : m_pImp(NULL)
    , m_pDelegate(NULL)
    , m_pProperties(NULL)
{
    m_pProperties = FX_NEW CFWL_WidgetProperties;
    m_pWidgetMgr = (CFWL_WidgetMgr*)FWL_GetWidgetMgr();
    FXSYS_assert(m_pWidgetMgr != NULL);
}
CFWL_Widget::~CFWL_Widget()
{
    if (m_pProperties) {
        delete m_pProperties;
        m_pProperties = NULL;
    }
    if (m_pImp) {
        m_pImp->Finalize();
        m_pImp->Release();
        m_pImp = NULL;
    }
}
FWL_ERR CFWL_Widget::Repaint(const CFX_RectF *pRect)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    CFX_RectF rect;
    if (pRect) {
        rect = *pRect;
    } else {
        m_pImp->GetWidgetRect(rect);
        rect.left = rect.top = 0;
    }
    return m_pWidgetMgr->RepaintWidget(m_pImp, &rect);
}
FWL_ERR CFWL_Widget::SetFocus(FX_BOOL bFocus)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    IFWL_NoteThread *pThread = m_pImp->GetOwnerThread();
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    IFWL_NoteDriver *pDriver = pThread->GetNoteDriver();
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    if (bFocus) {
        pDriver->SetFocus(m_pImp);
    } else {
        if (((CFWL_NoteDriver*)pDriver)->GetFocus() == m_pImp) {
            pDriver->SetFocus(NULL);
        }
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_Widget::SetGrab(FX_BOOL bSet)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    IFWL_NoteThread *pThread = m_pImp->GetOwnerThread();
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    CFWL_NoteDriver *pDriver = (CFWL_NoteDriver*)pThread->GetNoteDriver();
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, FWL_ERR_Indefinite);
    pDriver->SetGrab(m_pImp, bSet);
    return FWL_ERR_Succeeded;
}
void CFWL_Widget::RegisterEventTarget(CFWL_Widget *pEventSource , FX_DWORD dwFilter )
{
    _FWL_RETURN_IF_FAIL(m_pImp);
    IFWL_NoteThread *pThread = m_pImp->GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pNoteDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pNoteDriver);
    IFWL_Widget *pEventSourceImp = !pEventSource ? NULL : pEventSource->GetWidget();
    pNoteDriver->RegisterEventTarget(GetWidget(), pEventSourceImp, dwFilter);
}
void CFWL_Widget::DispatchEvent(CFWL_Event *pEvent)
{
    _FWL_RETURN_IF_FAIL(m_pImp);
    if (m_pImp->GetOuter()) {
        return;
    }
    IFWL_NoteThread *pThread = m_pImp->GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pNoteDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pNoteDriver);
    pNoteDriver->SendNote(pEvent);
}
#define FWL_WGT_CalcHeight				2048
#define FWL_WGT_CalcWidth				2048
#define FWL_WGT_CalcMultiLineDefWidth	120.0f
CFX_SizeF CFWL_Widget::CalcTextSize(const CFX_WideString &wsText, FX_BOOL bMultiLine , FX_INT32 iLineWidth )
{
    CFX_SizeF sz;
    sz.Set(0, 0);
    _FWL_RETURN_VALUE_IF_FAIL(m_pImp, sz);
    IFWL_ThemeProvider *pTheme = m_pImp->GetThemeProvider();
    _FWL_RETURN_VALUE_IF_FAIL(pTheme, sz);
    CFWL_ThemeText calPart;
    calPart.m_pWidget = m_pImp;
    calPart.m_wsText = wsText;
    calPart.m_dwTTOStyles = bMultiLine ? FDE_TTOSTYLE_LineWrap : FDE_TTOSTYLE_SingleLine;
    calPart.m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
    CFX_RectF rect;
    FX_FLOAT fWidth = bMultiLine ?
                      (iLineWidth > 0 ? (FX_FLOAT)iLineWidth : FWL_WGT_CalcMultiLineDefWidth) :
                      FWL_WGT_CalcWidth;
    rect.Set(0, 0, fWidth, FWL_WGT_CalcHeight);
    pTheme->CalcTextRect(&calPart, rect);
    sz.x = rect.width;
    sz.y = rect.height;
    return sz;
}
CFWL_WidgetDelegate::CFWL_WidgetDelegate()
{
}
CFWL_WidgetDelegate::~CFWL_WidgetDelegate()
{
}
FX_INT32 CFWL_WidgetDelegate::OnProcessMessage(CFWL_Message *pMessage)
{
    return 1;
}
FWL_ERR CFWL_WidgetDelegate::OnProcessEvent(CFWL_Event *pEvent)
{
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetDelegate::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    return FWL_ERR_Succeeded;
}
