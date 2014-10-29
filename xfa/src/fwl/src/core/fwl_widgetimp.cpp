// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_appimp.h"
#include "include/fwl_widgetmgrimp.h"
#include "include/fwl_widgetimp.h"
FWL_ERR	IFWL_Widget::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    return ((CFWL_WidgetImp*)m_pData)->GetWidgetRect(rect, bAutoSize);
}
FWL_ERR	IFWL_Widget::GetGlobalRect(CFX_RectF &rect)
{
    return ((CFWL_WidgetImp*)m_pData)->GetGlobalRect(rect);
}
FWL_ERR	IFWL_Widget::SetWidgetRect(const CFX_RectF &rect)
{
    return ((CFWL_WidgetImp*)m_pData)->SetWidgetRect(rect);
}
FWL_ERR	IFWL_Widget::GetClientRect(CFX_RectF &rect)
{
    return ((CFWL_WidgetImp*)m_pData)->GetClientRect(rect);
}
IFWL_Widget* IFWL_Widget::GetParent()
{
    return ((CFWL_WidgetImp*)m_pData)->GetParent();
}
FWL_ERR	IFWL_Widget::SetParent(IFWL_Widget *pParent)
{
    return ((CFWL_WidgetImp*)m_pData)->SetParent(pParent);
}
IFWL_Widget* IFWL_Widget::GetOwner()
{
    return ((CFWL_WidgetImp*)m_pData)->GetOwner();
}
FWL_ERR	IFWL_Widget::SetOwner(IFWL_Widget *pOwner)
{
    return ((CFWL_WidgetImp*)m_pData)->SetOwner(pOwner);
}
IFWL_Widget* IFWL_Widget::GetOuter()
{
    return ((CFWL_WidgetImp*)m_pData)->GetOuter();
}
FX_DWORD IFWL_Widget::GetStyles()
{
    return ((CFWL_WidgetImp*)m_pData)->GetStyles();
}
FWL_ERR	IFWL_Widget::ModifyStyles(FX_DWORD dwStylesAdded, FX_DWORD dwStylesRemoved)
{
    return ((CFWL_WidgetImp*)m_pData)->ModifyStyles(dwStylesAdded, dwStylesRemoved);
}
FX_DWORD IFWL_Widget::GetStylesEx()
{
    return ((CFWL_WidgetImp*)m_pData)->GetStylesEx();
}
FWL_ERR	IFWL_Widget::ModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved)
{
    return ((CFWL_WidgetImp*)m_pData)->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}
FX_DWORD IFWL_Widget::GetStates()
{
    return ((CFWL_WidgetImp*)m_pData)->GetStates();
}
FWL_ERR	IFWL_Widget::SetStates(FX_DWORD dwStates, FX_BOOL bSet )
{
    return ((CFWL_WidgetImp*)m_pData)->SetStates(dwStates, bSet);
}
FWL_ERR	IFWL_Widget::SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback)
{
    return ((CFWL_WidgetImp*)m_pData)->SetPrivateData(module_id, pData, callback);
}
FX_LPVOID IFWL_Widget::GetPrivateData(FX_LPVOID module_id)
{
    return ((CFWL_WidgetImp*)m_pData)->GetPrivateData(module_id);
}
FWL_ERR	IFWL_Widget::Update()
{
    return ((CFWL_WidgetImp*)m_pData)->Update();
}
FWL_ERR	IFWL_Widget::LockUpdate()
{
    return ((CFWL_WidgetImp*)m_pData)->LockUpdate();
}
FWL_ERR	IFWL_Widget::UnlockUpdate()
{
    return ((CFWL_WidgetImp*)m_pData)->UnlockUpdate();
}
FX_DWORD IFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy)
{
    return ((CFWL_WidgetImp*)m_pData)->HitTest(fx, fy);
}
FWL_ERR	IFWL_Widget::TransformTo(IFWL_Widget *pWidget, FX_FLOAT &fx, FX_FLOAT &fy)
{
    return ((CFWL_WidgetImp*)m_pData)->TransformTo(pWidget, fx, fy);
}
FWL_ERR	IFWL_Widget::TransformTo(IFWL_Widget *pWidget, CFX_RectF &rt)
{
    return ((CFWL_WidgetImp*)m_pData)->TransformTo(pWidget, rt);
}
FWL_ERR	IFWL_Widget::GetMatrix(CFX_Matrix &matrix, FX_BOOL bGlobal )
{
    return ((CFWL_WidgetImp*)m_pData)->GetMatrix(matrix, bGlobal);
}
FWL_ERR	IFWL_Widget::SetMatrix(const CFX_Matrix &matrix)
{
    return ((CFWL_WidgetImp*)m_pData)->SetMatrix(matrix);
}
FWL_ERR	IFWL_Widget::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    return ((CFWL_WidgetImp*)m_pData)->DrawWidget(pGraphics, pMatrix);
}
IFWL_ThemeProvider*	IFWL_Widget::GetThemeProvider()
{
    return ((CFWL_WidgetImp*)m_pData)->GetThemeProvider();
}
FWL_ERR	IFWL_Widget::SetThemeProvider(IFWL_ThemeProvider *pThemeProvider)
{
    return ((CFWL_WidgetImp*)m_pData)->SetThemeProvider(pThemeProvider);
}
FWL_ERR	IFWL_Widget::SetDataProvider(IFWL_DataProvider *pDataProvider)
{
    return ((CFWL_WidgetImp*)m_pData)->SetDataProvider(pDataProvider);
}
IFWL_WidgetDelegate* IFWL_Widget::SetDelegate(IFWL_WidgetDelegate *pDelegate)
{
    return ((CFWL_WidgetImp*)m_pData)->SetDelegate(pDelegate);
}
IFWL_NoteThread* IFWL_Widget::GetOwnerThread() const
{
    return ((CFWL_WidgetImp*)m_pData)->GetOwnerThread();
}
CFX_SizeF IFWL_Widget::GetOffsetFromParent(IFWL_Widget *pParent)
{
    return ((CFWL_WidgetImp*)m_pData)->GetOffsetFromParent(pParent);
}
FWL_ERR CFWL_WidgetImp::Initialize()
{
    IFWL_App *pApp = FWL_GetApp();
    _FWL_RETURN_VALUE_IF_FAIL(pApp, FWL_ERR_Indefinite);
    IFWL_AdapterNative *pAdapter = pApp->GetAdapterNative();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapter, FWL_ERR_Indefinite);
    IFWL_AdapterThreadMgr *pAdapterThread = pAdapter->GetThreadMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pAdapterThread, FWL_ERR_Indefinite);
    SetOwnerThread((CFWL_NoteThread*)pAdapterThread->GetCurrentThread());
    IFWL_Widget *pParent = m_pProperties->m_pParent;
    m_pWidgetMgr->InsertWidget(pParent, m_pInterface);
    if (!IsChild()) {
        {
            IFWL_Widget *pOwner = m_pProperties->m_pOwner;
            if (pOwner) {
                m_pWidgetMgr->SetOwner(pOwner, m_pInterface);
            }
        }
        m_pWidgetMgr->CreateWidget_Native(m_pInterface);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::Finalize()
{
    NotifyDriver();
    IFWL_Form *pForm = (IFWL_Form *)FWL_GetWidgetMgr()->GetWidget(m_pInterface, FWL_WGTRELATION_SystemForm);
    if (pForm && pForm != m_pInterface) {
        IFWL_Content *pContent = pForm->GetContent();
        if (pContent) {
            pContent->RemoveWidget(m_pInterface);
        }
    }
    if (!IsChild()) {
        m_pWidgetMgr->DestroyWidget_Native(m_pInterface);
    }
    m_pWidgetMgr->RemoveWidget(m_pInterface);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (bAutoSize) {
        if (HasEdge()) {
            FX_FLOAT fEdge = GetEdgeWidth();
            rect.Inflate(fEdge, fEdge);
        }
        if (HasBorder()) {
            FX_FLOAT fBorder = GetBorderSize();
            rect.Inflate(fBorder, fBorder);
        }
    } else {
        rect = m_pProperties->m_rtWidget;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImp::GetGlobalRect(CFX_RectF &rect)
{
    IFWL_Widget *pForm = m_pWidgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_SystemForm);
    _FWL_RETURN_VALUE_IF_FAIL(pForm, FWL_ERR_Indefinite);
    rect.Set(0, 0, m_pProperties->m_rtWidget.width, m_pProperties->m_rtWidget.height);
    if (pForm == m_pInterface) {
        return FWL_ERR_Succeeded;
    }
    return TransformTo(pForm, rect);
}
FWL_ERR CFWL_WidgetImp::SetWidgetRect(const CFX_RectF &rect)
{
    CFX_RectF rtOld = m_pProperties->m_rtWidget;
    m_pProperties->m_rtWidget = rect;
    if (IsChild()) {
        if (FXSYS_fabs(rtOld.width - rect.width) > 0.5f || FXSYS_fabs(rtOld.height - rect.height) > 0.5f) {
            CFWL_EvtSizeChanged ev;
            ev.m_pSrcTarget = m_pInterface;
            ev.m_rtOld = rtOld;
            ev.m_rtNew = rect;
            IFWL_WidgetDelegate *pDelegate = SetDelegate(NULL);
            if (pDelegate) {
                pDelegate->OnProcessEvent(&ev);
            }
        }
        return FWL_ERR_Succeeded;
    }
    m_pWidgetMgr->SetWidgetRect_Native(m_pInterface, rect);
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::GetClientRect(CFX_RectF &rect)
{
    GetEdgeRect(rect);
    if (HasEdge()) {
        FX_FLOAT fEdge = GetEdgeWidth();
        rect.Deflate(fEdge, fEdge);
    }
    return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_WidgetImp::GetParent()
{
    return m_pWidgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_Parent);
}
FWL_ERR CFWL_WidgetImp::SetParent(IFWL_Widget* pParent)
{
    m_pProperties->m_pParent = pParent;
    m_pWidgetMgr->SetParent(pParent, m_pInterface);
    return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_WidgetImp::GetOwner()
{
    return m_pWidgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_Owner);
}
FWL_ERR CFWL_WidgetImp::SetOwner(IFWL_Widget* pOwner)
{
    m_pProperties->m_pOwner = pOwner;
    m_pWidgetMgr->SetOwner(pOwner, m_pInterface);
    return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_WidgetImp::GetOuter()
{
    return m_pOuter;
}
FX_DWORD CFWL_WidgetImp::GetStyles()
{
    return m_pProperties->m_dwStyles;
}
FWL_ERR CFWL_WidgetImp::ModifyStyles(FX_DWORD dwStylesAdded, FX_DWORD dwStylesRemoved)
{
    m_pProperties->m_dwStyles = (m_pProperties->m_dwStyles & ~dwStylesRemoved) | dwStylesAdded;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_WidgetImp::GetStylesEx()
{
    return m_pProperties->m_dwStyleExes;
}
FWL_ERR CFWL_WidgetImp::ModifyStylesEx(FX_DWORD dwStylesExAdded, FX_DWORD dwStylesExRemoved)
{
    m_pProperties->m_dwStyleExes = (m_pProperties->m_dwStyleExes & ~dwStylesExRemoved) | dwStylesExAdded;
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_WidgetImp::GetStates()
{
    return m_pProperties->m_dwStates;
}
static void NotifyHideChildWidget(IFWL_WidgetMgr *widgetMgr, IFWL_Widget *widget, CFWL_NoteDriver *noteDriver)
{
    IFWL_Widget *child = widgetMgr->GetWidget(widget, FWL_WGTRELATION_FirstChild);
    while (child) {
        noteDriver->NotifyTargetHide(child);
        NotifyHideChildWidget(widgetMgr, child, noteDriver);
        child = widgetMgr->GetWidget(child, FWL_WGTRELATION_NextSibling);
    }
}
FWL_ERR CFWL_WidgetImp::SetStates(FX_DWORD dwStates, FX_BOOL bSet )
{
    bSet ? (m_pProperties->m_dwStates |= dwStates) : (m_pProperties->m_dwStates &= ~dwStates);
    FWL_ERR ret = FWL_ERR_Succeeded;
    if (dwStates & FWL_WGTSTATE_Invisible) {
        if (bSet) {
            ret = m_pWidgetMgr->HideWidget_Native(m_pInterface);
            CFWL_NoteDriver *noteDriver = (CFWL_NoteDriver*)GetOwnerThread()->GetNoteDriver();
            IFWL_WidgetMgr *widgetMgr = FWL_GetWidgetMgr();
            noteDriver->NotifyTargetHide(m_pInterface);
            IFWL_Widget *child = widgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_FirstChild);
            while (child) {
                noteDriver->NotifyTargetHide(child);
                NotifyHideChildWidget(widgetMgr, child, noteDriver);
                child = widgetMgr->GetWidget(child, FWL_WGTRELATION_NextSibling);
            }
        } else {
            ret = m_pWidgetMgr->ShowWidget_Native(m_pInterface);
        }
    }
    return ret;
}
FWL_ERR CFWL_WidgetImp::SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback)
{
    if (!m_pPrivateData) {
        m_pPrivateData = new CFX_PrivateData;
    }
    m_pPrivateData->SetPrivateData(module_id, pData, callback);
    return FWL_ERR_Succeeded;
}
FX_LPVOID CFWL_WidgetImp::GetPrivateData(FX_LPVOID module_id)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pPrivateData, NULL);
    return m_pPrivateData->GetPrivateData(module_id);
}
FWL_ERR CFWL_WidgetImp::Update()
{
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::LockUpdate()
{
    m_iLock++;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::UnlockUpdate()
{
    if (IsLocked()) {
        m_iLock--;
    }
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_WidgetImp::HitTest(FX_FLOAT fx, FX_FLOAT fy)
{
    CFX_RectF rtClient;
    GetClientRect(rtClient);
    if (rtClient.Contains(fx, fy)) {
        return FWL_WGTHITTEST_Client;
    }
    if (HasEdge()) {
        CFX_RectF rtEdge;
        GetEdgeRect(rtEdge);
        if (rtEdge.Contains(fx, fy)) {
            return FWL_WGTHITTEST_Edge;
        }
    }
    if (HasBorder()) {
        CFX_RectF rtRelative;
        GetRelativeRect(rtRelative);
        if (rtRelative.Contains(fx, fy)) {
            return FWL_WGTHITTEST_Border;
        }
    }
    return FWL_WGTHITTEST_Unknown;
}
FWL_ERR CFWL_WidgetImp::TransformTo(IFWL_Widget *pWidget, FX_FLOAT &fx, FX_FLOAT &fy)
{
    if (m_pWidgetMgr->IsFormDisabled()) {
        CFX_SizeF szOffset;
        if (IsParent(pWidget)) {
            szOffset = GetOffsetFromParent(pWidget);
        } else {
            szOffset = ((IFWL_Widget*)pWidget)->GetOffsetFromParent(m_pInterface);
            szOffset.x = -szOffset.x;
            szOffset.y = -szOffset.y;
        }
        fx += szOffset.x;
        fy += szOffset.y;
        return FWL_ERR_Succeeded;
    }
    CFX_RectF	r;
    CFX_Matrix	m;
    IFWL_Widget * parent = GetParent();
    if (parent) {
        GetWidgetRect(r);
        fx += r.left;
        fy += r.top;
        GetMatrix(m, TRUE);
        m.TransformPoint(fx, fy);
    }
    IFWL_Widget * form1 = m_pWidgetMgr->GetWidget(m_pInterface, FWL_WGTRELATION_SystemForm);
    _FWL_RETURN_VALUE_IF_FAIL(form1, FWL_ERR_Indefinite);
    if (!pWidget) {
        form1->GetWidgetRect(r);
        fx += r.left;
        fy += r.top;
#ifdef FWL_UseMacSystemBorder
        if (form1->GetStyles() & FWL_WGTSTYLE_Caption) {
            FX_FLOAT l, t, r, b;
            l = t = r = b = 0;
            FWL_GetAdapterWidgetMgr()->GetSystemBorder(l, t, r, b);
            fy += t;
        }
#endif
        return FWL_ERR_Succeeded;
    }
    IFWL_Widget * form2 = m_pWidgetMgr->GetWidget(pWidget, FWL_WGTRELATION_SystemForm);
    _FWL_RETURN_VALUE_IF_FAIL(form2, FWL_ERR_Indefinite);
    if (form1 != form2) {
        form1->GetWidgetRect(r);
        fx += r.left;
        fy += r.top;
        form2->GetWidgetRect(r);
        fx -= r.left;
        fy -= r.top;
#ifdef FWL_UseMacSystemBorder
        if ((form1->GetStyles() & FWL_WGTSTYLE_Caption) != (form2->GetStyles() & FWL_WGTSTYLE_Caption)) {
            FX_FLOAT l, t, r, b;
            l = t = r = b = 0;
            FWL_GetAdapterWidgetMgr()->GetSystemBorder(l, t, r, b);
            (form1->GetStyles() & FWL_WGTSTYLE_Caption) ? (fy += t) : (fy -= t);
        }
#endif
    }
    parent = pWidget->GetParent();
    if (parent) {
        pWidget->GetMatrix(m, TRUE);
        CFX_Matrix m1;
        m1.Reset();
        m1.SetReverse(m);
        m1.TransformPoint(fx, fy);
        pWidget->GetWidgetRect(r);
        fx -= r.left;
        fy -= r.top;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::TransformTo(IFWL_Widget *pWidget, CFX_RectF &rt)
{
    return TransformTo(pWidget, rt.left, rt.top);
}
FWL_ERR	CFWL_WidgetImp::GetMatrix(CFX_Matrix &matrix, FX_BOOL bGlobal )
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties, FWL_ERR_Indefinite);
    if (bGlobal) {
        IFWL_Widget * parent = GetParent();
        CFX_PtrArray	parents;
        while (parent) {
            parents.Add(parent);
            parent = parent->GetParent();
        }
        matrix.Reset();
        CFX_Matrix ctmOnParent;
        CFX_RectF rect;
        FX_INT32 count = parents.GetSize();
        for (FX_INT32 i = count - 2; i >= 0; i--) {
            parent = (IFWL_Widget *) parents.GetAt(i);
            parent->GetMatrix(ctmOnParent, FALSE);
            parent->GetWidgetRect(rect);
            matrix.Concat(ctmOnParent, TRUE);
            matrix.Translate(rect.left, rect.top, TRUE);
        }
        matrix.Concat(m_pProperties->m_ctmOnParent, TRUE);
        parents.RemoveAll();
    } else {
        matrix = m_pProperties->m_ctmOnParent;
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImp::SetMatrix(const CFX_Matrix &matrix)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pProperties, FWL_ERR_Indefinite);
    IFWL_Widget * parent = GetParent();
    if (!parent) {
        return FWL_ERR_Indefinite;
    }
    m_pProperties->m_ctmOnParent = matrix;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImp::DrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    return FWL_ERR_Indefinite;
}
IFWL_ThemeProvider* CFWL_WidgetImp::GetThemeProvider()
{
    return m_pProperties->m_pThemeProvider;
}
FWL_ERR CFWL_WidgetImp::SetThemeProvider(IFWL_ThemeProvider *pThemeProvider)
{
    m_pProperties->m_pThemeProvider = pThemeProvider;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_WidgetImp::SetDataProvider(IFWL_DataProvider *pDataProvider)
{
    m_pProperties->m_pDataProvider = pDataProvider;
    return FWL_ERR_Succeeded;
}
IFWL_WidgetDelegate* CFWL_WidgetImp::SetDelegate(IFWL_WidgetDelegate *pDelegate)
{
    if (!m_pCurDelegate) {
        m_pCurDelegate = m_pDelegate;
    }
    if (!pDelegate) {
        return m_pCurDelegate;
    }
    IFWL_WidgetDelegate *pOldDelegate = m_pCurDelegate;
    m_pCurDelegate = pDelegate;
    return pOldDelegate;
}
IFWL_NoteThread* CFWL_WidgetImp::GetOwnerThread() const
{
    return (IFWL_NoteThread*)m_pOwnerThread;
}
FWL_ERR CFWL_WidgetImp::SetOwnerThread(CFWL_NoteThread *pOwnerThread)
{
    m_pOwnerThread = pOwnerThread;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImp::GetProperties(CFWL_WidgetImpProperties &properties)
{
    properties = *m_pProperties;
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImp::SetProperties(const CFWL_WidgetImpProperties &properties)
{
    *m_pProperties = properties;
    return FWL_ERR_Succeeded;
}
IFWL_Widget* CFWL_WidgetImp::GetInterface()
{
    return m_pInterface;
}
void CFWL_WidgetImp::SetInterface(IFWL_Widget *pInterface)
{
    m_pInterface = pInterface;
}
CFWL_WidgetImp::CFWL_WidgetImp(IFWL_Widget *pOuter )
    : m_pProperties(NULL)
    , m_pPrivateData(NULL)
    , m_pDelegate(NULL)
    , m_pCurDelegate(NULL)
    , m_pOuter(pOuter)
    , m_pInterface(NULL)
    , m_iLock(0)
{
    m_pProperties = FX_NEW CFWL_WidgetImpProperties;
    m_pWidgetMgr = (CFWL_WidgetMgr*)FWL_GetWidgetMgr();
    FXSYS_assert(m_pWidgetMgr != NULL);
}
CFWL_WidgetImp::CFWL_WidgetImp(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : m_pProperties(NULL)
    , m_pPrivateData(NULL)
    , m_pDelegate(NULL)
    , m_pCurDelegate(NULL)
    , m_pOuter(pOuter)
    , m_pInterface(NULL)
    , m_iLock(0)
{
    if (!m_pProperties) {
        m_pProperties = FX_NEW CFWL_WidgetImpProperties;
    }
    *m_pProperties = properties;
    m_pWidgetMgr = (CFWL_WidgetMgr*)FWL_GetWidgetMgr();
    FXSYS_assert(m_pWidgetMgr != NULL);
}
CFWL_WidgetImp::~CFWL_WidgetImp()
{
    if (m_pPrivateData) {
        delete m_pPrivateData;
        m_pPrivateData = NULL;
    }
    if (m_pProperties) {
        delete m_pProperties;
        m_pProperties = NULL;
    }
}
FX_BOOL	CFWL_WidgetImp::IsEnabled()
{
    return (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) == 0;
}
FX_BOOL CFWL_WidgetImp::IsVisible()
{
    return (m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible) == 0;
}
FX_BOOL CFWL_WidgetImp::IsActive()
{
    return (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) == 0;
}
FX_BOOL CFWL_WidgetImp::IsOverLapper()
{
    return (m_pProperties->m_dwStyles & FWL_WGTSTYLE_WindowTypeMask) == FWL_WGTSTYLE_OverLapper;
}
FX_BOOL CFWL_WidgetImp::IsPopup()
{
    return m_pProperties->m_dwStyles & FWL_WGTSTYLE_Popup;
}
FX_BOOL CFWL_WidgetImp::IsChild()
{
    return m_pProperties->m_dwStyles & FWL_WGTSTYLE_Child;
}
FX_BOOL	CFWL_WidgetImp::IsLocked()
{
    return m_iLock > 0;
}
FX_BOOL CFWL_WidgetImp::IsOffscreen()
{
    return m_pProperties->m_dwStyles & FWL_WGTSTYLE_Offscreen;
}
FX_BOOL CFWL_WidgetImp::HasBorder()
{
    return m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border;
}
FX_BOOL CFWL_WidgetImp::HasEdge()
{
    return m_pProperties->m_dwStyles & FWL_WGTSTYLE_EdgeMask;
}
void CFWL_WidgetImp::GetEdgeRect(CFX_RectF &rtEdge)
{
    rtEdge = m_pProperties->m_rtWidget;
    rtEdge.left = rtEdge.top = 0;
    if (HasBorder()) {
        FX_FLOAT fCX = GetBorderSize();
        FX_FLOAT fCY = GetBorderSize(FALSE);
        rtEdge.Deflate(fCX, fCY);
    }
}
FX_FLOAT CFWL_WidgetImp::GetBorderSize(FX_BOOL bCX )
{
    FX_FLOAT *pfBorder = (FX_FLOAT*)GetThemeCapacity(bCX ? FWL_WGTCAPACITY_CXBorder : FWL_WGTCAPACITY_CYBorder);
    _FWL_RETURN_VALUE_IF_FAIL(pfBorder, 0);
    return *pfBorder;
}
FX_FLOAT CFWL_WidgetImp::GetEdgeWidth()
{
    FX_DWORD dwCapacity = 0;
    switch(m_pProperties->m_dwStyles & FWL_WGTSTYLE_EdgeMask) {
        case FWL_WGTSTYLE_EdgeFlat: {
                dwCapacity = FWL_WGTCAPACITY_EdgeFlat;
                break;
            }
        case FWL_WGTSTYLE_EdgeRaised: {
                dwCapacity = FWL_WGTCAPACITY_EdgeRaised;
                break;
            }
        case FWL_WGTSTYLE_EdgeSunken: {
                dwCapacity = FWL_WGTCAPACITY_EdgeSunken;
                break;
            }
    }
    if (dwCapacity > 0) {
        FX_FLOAT *fRet = (FX_FLOAT*)GetThemeCapacity(dwCapacity);
        _FWL_RETURN_VALUE_IF_FAIL(fRet, 0);
        return *fRet;
    }
    return 0;
}
void CFWL_WidgetImp::GetRelativeRect(CFX_RectF &rect)
{
    rect = m_pProperties->m_rtWidget;
    rect.left = rect.top = 0;
}
FX_LPVOID CFWL_WidgetImp::GetThemeCapacity(FX_DWORD dwCapacity)
{
    IFWL_ThemeProvider *pTheme = GetAvailableTheme();
    _FWL_RETURN_VALUE_IF_FAIL(pTheme, NULL);
    CFWL_ThemePart part;
    part.m_pWidget = m_pInterface;
    return pTheme->GetCapacity(&part, dwCapacity);
}
IFWL_ThemeProvider* CFWL_WidgetImp::GetAvailableTheme()
{
    if (m_pProperties->m_pThemeProvider) {
        return m_pProperties->m_pThemeProvider;
    }
    IFWL_Widget *pUp = m_pInterface;
    do {
        FWL_WGTRELATION relation = (pUp->GetStyles() & FWL_WGTSTYLE_Popup)
                                   ? FWL_WGTRELATION_Owner : FWL_WGTRELATION_Parent;
        pUp = m_pWidgetMgr->GetWidget(pUp, relation);
        if (pUp) {
            IFWL_ThemeProvider *pRet = pUp->GetThemeProvider();
            if (pRet && pRet->IsValidWidget(m_pInterface)) {
                return pRet;
            }
        }
    } while (pUp);
    return ((CFWL_AppImp*)FWL_GetApp())->GetThemeProvider();
}
CFWL_WidgetImp* CFWL_WidgetImp::GetRootOuter()
{
    IFWL_Widget *pRet = m_pOuter;
    IFWL_Widget *pOuter = pRet;
    while (pOuter) {
        pRet = pOuter;
        pOuter = pOuter->GetOuter();
    }
    return (CFWL_WidgetImp*)pRet;
}
CFWL_WidgetImp* CFWL_WidgetImp::GetSameAncestor(CFWL_WidgetImp *pWidget)
{
    CFX_PtrArray arr1, arr2;
    CFWL_WidgetImp *pAncestor = pWidget;
    FWL_WGTRELATION relation;
    do {
        arr1.Add(pAncestor);
        relation =  pAncestor->IsPopup() ? FWL_WGTRELATION_Owner : FWL_WGTRELATION_Parent;
    } while ((pAncestor = (CFWL_WidgetImp*)m_pWidgetMgr->GetWidget(pAncestor->m_pInterface, relation)) != NULL);
    pAncestor = this;
    do {
        arr2.Add(pAncestor);
        relation =  pAncestor->IsPopup() ? FWL_WGTRELATION_Owner : FWL_WGTRELATION_Parent;
    } while ((pAncestor = (CFWL_WidgetImp*)m_pWidgetMgr->GetWidget(pAncestor->m_pInterface, relation)) != NULL);
    for (FX_INT32 i = 0; i < arr1.GetSize(); i ++) {
        FX_LPVOID pVoid = arr1[i];
        if (arr2.Find(pVoid) < 0) {
            continue;
        } else {
            return (CFWL_WidgetImp*)pVoid;
        }
    }
    return NULL;
}
CFX_SizeF CFWL_WidgetImp::GetOffsetFromAncestor(CFWL_WidgetImp *pAncestor)
{
    CFX_SizeF szRet;
    szRet.Set(0, 0);
    if (pAncestor == this) {
        return szRet;
    }
    CFWL_WidgetImp *pWidget = this;
    do {
        CFX_RectF rect;
        pWidget->GetWidgetRect(rect);
        szRet.x += rect.left;
        szRet.y += rect.top;
        FWL_WGTRELATION relation = pWidget->IsPopup() ? FWL_WGTRELATION_Owner : FWL_WGTRELATION_Parent;
        pWidget = (CFWL_WidgetImp*)m_pWidgetMgr->GetWidget((IFWL_Widget*)pWidget, relation);
    } while(pWidget && pWidget != pAncestor);
    return szRet;
}
FX_BOOL	CFWL_WidgetImp::TransformToOuter(FX_FLOAT &fx, FX_FLOAT &fy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pOuter, FALSE);
    fx += m_pProperties->m_rtWidget.left;
    fx += m_pProperties->m_rtWidget.top;
    return TRUE;
}
FX_BOOL	CFWL_WidgetImp::TransformFromOuter(FX_FLOAT &fx, FX_FLOAT &fy)
{
    _FWL_RETURN_VALUE_IF_FAIL(m_pOuter, FALSE);
    CFX_RectF rect;
    m_pOuter->GetWidgetRect(rect);
    fx -= rect.left;
    fx -= rect.top;
    return TRUE;
}
#define FWL_WGT_CalcHeight				2048
#define FWL_WGT_CalcWidth				2048
#define FWL_WGT_CalcMultiLineDefWidth	120.0f
CFX_SizeF CFWL_WidgetImp::CalcTextSize(const CFX_WideString &wsText, IFWL_ThemeProvider *pTheme, FX_BOOL bMultiLine , FX_INT32 iLineWidth )
{
    CFX_SizeF sz;
    sz.Set(0, 0);
    _FWL_RETURN_VALUE_IF_FAIL(pTheme, sz);
    CFWL_ThemeText calPart;
    calPart.m_pWidget = m_pInterface;
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
void CFWL_WidgetImp::CalcTextRect(const CFX_WideString &wsText, IFWL_ThemeProvider *pTheme, FX_DWORD dwTTOStyles, FX_INT32 iTTOAlign, CFX_RectF &rect)
{
    CFWL_ThemeText calPart;
    calPart.m_pWidget = m_pInterface;
    calPart.m_wsText = wsText;
    calPart.m_dwTTOStyles = dwTTOStyles;
    calPart.m_iTTOAlign = iTTOAlign;
    pTheme->CalcTextRect(&calPart, rect);
}
void CFWL_WidgetImp::SetFocus(FX_BOOL bFocus)
{
    if (m_pWidgetMgr->IsFormDisabled()) {
        return;
    }
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pDriver);
    IFWL_Widget *curFocus = ((CFWL_NoteDriver*)pDriver)->GetFocus();
    if (bFocus && curFocus != m_pInterface) {
        pDriver->SetFocus(m_pInterface);
    } else if (!bFocus && curFocus == m_pInterface) {
        pDriver->SetFocus(NULL);
    }
}
void CFWL_WidgetImp::SetGrab(FX_BOOL bSet)
{
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    CFWL_NoteDriver *pDriver = (CFWL_NoteDriver*)pThread->GetNoteDriver();
    pDriver->SetGrab(m_pInterface, bSet);
}
FX_BOOL	CFWL_WidgetImp::GetPopupPos(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup)
{
    if (GetClassID() == FWL_CLASSHASH_Menu) {
        return GetPopupPosMenu(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
    } else {
        if (GetClassID() == FWL_CLASSHASH_ComboBox) {
            if (m_pWidgetMgr->IsFormDisabled()) {
                return m_pWidgetMgr->GetAdapterPopupPos(m_pInterface, fMinHeight, fMaxHeight, rtAnchor, rtPopup);
            } else {
                return GetPopupPosComboBox(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
            }
        } else if (GetClassID() == FWL_CLASSHASH_DateTimePicker && m_pWidgetMgr->IsFormDisabled()) {
            return m_pWidgetMgr->GetAdapterPopupPos(m_pInterface, fMinHeight, fMaxHeight, rtAnchor, rtPopup);
        } else {
            return GetPopupPosGeneral(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
        }
    }
    return FALSE;
}
FX_BOOL	CFWL_WidgetImp::GetPopupPosMenu(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup)
{
    FX_FLOAT fx = 0;
    FX_FLOAT fy = 0;
    FX_FLOAT fScreenWidth = 0;
    FX_FLOAT fScreenHeight = 0;
    GetScreenSize(fScreenWidth, fScreenHeight);
    if (GetStylesEx() & FWL_STYLEEXT_MNU_Vert) {
        FX_BOOL bLeft = m_pProperties->m_rtWidget.left < 0;
        FX_FLOAT fRight = rtAnchor.right() + rtPopup.width;
        TransformTo(NULL, fx, fy);
        if (fRight + fx > fScreenWidth || bLeft) {
            rtPopup.Set(rtAnchor.left - rtPopup.width, rtAnchor.top, rtPopup.width,	rtPopup.height);
        } else {
            rtPopup.Set(rtAnchor.right(), rtAnchor.top, rtPopup.width, rtPopup.height);
        }
    } else {
        FX_FLOAT fBottom = rtAnchor.bottom() + rtPopup.height;
        TransformTo(NULL, fx, fy);
        if (fBottom + fy > fScreenHeight) {
            rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width, rtPopup.height);
        } else {
            rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width, rtPopup.height);
        }
    }
    rtPopup.Offset(fx, fy);
    return TRUE;
}
FX_BOOL	CFWL_WidgetImp::GetPopupPosComboBox(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup)
{
    FX_FLOAT fx = 0;
    FX_FLOAT fy = 0;
    FX_FLOAT fScreenWidth = 0;
    FX_FLOAT fScreenHeight = 0;
    GetScreenSize(fScreenWidth, fScreenHeight);
    FX_FLOAT fPopHeight = rtPopup.height;
    if (rtPopup.height > fMaxHeight) {
        fPopHeight = fMaxHeight;
    } else if (rtPopup.height < fMinHeight) {
        fPopHeight = fMinHeight;
    }
    FX_FLOAT fWidth = FX_MAX(rtAnchor.width, rtPopup.width);
    FX_FLOAT fBottom = rtAnchor.bottom() + fPopHeight;
    TransformTo(NULL, fx, fy);
    if (fBottom + fy > fScreenHeight) {
        rtPopup.Set(rtAnchor.left, rtAnchor.top - fPopHeight, fWidth, fPopHeight);
    } else {
        rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), fWidth, fPopHeight);
    }
    rtPopup.Offset(fx, fy);
    return TRUE;
}
FX_BOOL	CFWL_WidgetImp::GetPopupPosGeneral(FX_FLOAT fMinHeight, FX_FLOAT fMaxHeight, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup)
{
    FX_FLOAT fx = 0;
    FX_FLOAT fy = 0;
    FX_FLOAT fScreenWidth = 0;
    FX_FLOAT fScreenHeight = 0;
    GetScreenSize(fScreenWidth, fScreenHeight);
    TransformTo(NULL, fx, fy);
    if (rtAnchor.bottom() + fy > fScreenHeight) {
        rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width, rtPopup.height);
    } else {
        rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width, rtPopup.height);
    }
    rtPopup.Offset(fx, fy);
    return TRUE;
}
FX_BOOL	CFWL_WidgetImp::GetScreenSize(FX_FLOAT &fx, FX_FLOAT &fy)
{
    IFWL_AdapterNative *pNative = FWL_GetAdapterNative();
    IFWL_AdapterMonitorMgr *pMonitorMgr = pNative->GetMonitorMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pMonitorMgr, FALSE);
    FWL_HMONITOR hMonitor = pMonitorMgr->GetMonitorByPoint(fx, fy);
    pMonitorMgr->GetMonitorSize(hMonitor, fx, fy);
    return TRUE;
}
void CFWL_WidgetImp::RegisterEventTarget(IFWL_Widget *pEventSource , FX_DWORD dwFilter )
{
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pNoteDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pNoteDriver);
    pNoteDriver->RegisterEventTarget(m_pInterface, pEventSource, dwFilter);
}
void CFWL_WidgetImp::UnregisterEventTarget()
{
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pNoteDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pNoteDriver);
    pNoteDriver->UnregisterEventTarget(m_pInterface);
}
void CFWL_WidgetImp::DispatchKeyEvent(CFWL_MsgKey *pNote)
{
    _FWL_RETURN_IF_FAIL(pNote);
    CFWL_MsgKey *pEvent = (CFWL_MsgKey*)pNote->CloneToEvent();
    pEvent->m_pSrcTarget = m_pInterface;
    pEvent->m_dwCmd = pNote->m_dwCmd;
    pEvent->m_dwKeyCode = pNote->m_dwKeyCode;
    pEvent->m_dwFlags = pNote->m_dwFlags;
    DispatchEvent((CFWL_Event*)pEvent);
    pEvent->Release();
}
void CFWL_WidgetImp::DispatchEvent(CFWL_Event *pEvent)
{
    if (m_pOuter) {
        IFWL_WidgetDelegate *pDelegate = m_pOuter->SetDelegate(NULL);
        pDelegate->OnProcessEvent(pEvent);
        return;
    }
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    IFWL_NoteDriver *pNoteDriver = pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pNoteDriver);
    pNoteDriver->SendNote(pEvent);
}
void CFWL_WidgetImp::Repaint(const CFX_RectF *pRect )
{
    if (pRect) {
        m_pWidgetMgr->RepaintWidget(m_pInterface, pRect);
        return ;
    }
    CFX_RectF rect;
    rect = m_pProperties->m_rtWidget;
    rect.left = rect.top = 0;
    m_pWidgetMgr->RepaintWidget(m_pInterface, &rect);
}
void CFWL_WidgetImp::DrawBackground(CFX_Graphics		 *pGraphics,
                                    FX_INT32			  iPartBk,
                                    IFWL_ThemeProvider   *pTheme,
                                    const CFX_Matrix	 *pMatrix )
{
    CFX_RectF rtRelative;
    GetRelativeRect(rtRelative);
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = iPartBk;
    param.m_pGraphics = pGraphics;
    if (pMatrix) {
        param.m_matrix.Concat(*pMatrix, TRUE);
    }
    param.m_rtPart = rtRelative;
    pTheme->DrawBackground(&param);
}
void CFWL_WidgetImp::DrawBorder(CFX_Graphics		*pGraphics,
                                FX_INT32			 iPartBorder,
                                IFWL_ThemeProvider	*pTheme,
                                const CFX_Matrix	*pMatrix )
{
    CFX_RectF rtRelative;
    GetRelativeRect(rtRelative);
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = iPartBorder;
    param.m_pGraphics = pGraphics;
    if (pMatrix) {
        param.m_matrix.Concat(*pMatrix, TRUE);
    }
    param.m_rtPart = rtRelative;
    pTheme->DrawBackground(&param);
}
void CFWL_WidgetImp::DrawEdge(CFX_Graphics			*pGraphics,
                              FX_INT32				 iPartEdge,
                              IFWL_ThemeProvider    *pTheme,
                              const CFX_Matrix		*pMatrix )
{
    CFX_RectF rtEdge;
    GetEdgeRect(rtEdge);
    CFWL_ThemeBackground param;
    param.m_pWidget = m_pInterface;
    param.m_iPart = iPartEdge;
    param.m_pGraphics = pGraphics;
    if (pMatrix) {
        param.m_matrix.Concat(*pMatrix, TRUE);
    }
    param.m_rtPart = rtEdge;
    pTheme->DrawBackground(&param);
}
void CFWL_WidgetImp::NotifyDriver()
{
    IFWL_NoteThread *pThread = GetOwnerThread();
    _FWL_RETURN_IF_FAIL(pThread);
    CFWL_NoteDriver *pDriver = (CFWL_NoteDriver*)pThread->GetNoteDriver();
    _FWL_RETURN_IF_FAIL(pDriver);
    pDriver->NotifyTargetDestroy(m_pInterface);
}
CFX_SizeF CFWL_WidgetImp::GetOffsetFromParent(IFWL_Widget *pParent)
{
    CFX_SizeF szRet;
    szRet.Set(0, 0);
    if (pParent == (IFWL_Widget*)this) {
        return szRet;
    }
    IFWL_WidgetMgr *pWidgetMgr = FWL_GetWidgetMgr();
    _FWL_RETURN_VALUE_IF_FAIL(pWidgetMgr, szRet);
    szRet.x += m_pProperties->m_rtWidget.left;
    szRet.y += m_pProperties->m_rtWidget.top;
    IFWL_Widget *pDstWidget = GetParent();
    while (pDstWidget && pDstWidget != pParent) {
        CFX_RectF rtDst;
        pDstWidget->GetWidgetRect(rtDst);
        szRet.x += rtDst.left;
        szRet.y += rtDst.top;
        pDstWidget = pWidgetMgr->GetWidget(pDstWidget, FWL_WGTRELATION_Parent);
    }
    return szRet;
}
FX_BOOL	CFWL_WidgetImp::IsParent(IFWL_Widget *pParent)
{
    if (pParent == (IFWL_Widget*)this) {
        return FALSE;
    }
    IFWL_Widget *pUpWidget = GetParent();
    while (pUpWidget) {
        if (pUpWidget == pParent) {
            return TRUE;
        }
        pUpWidget = pUpWidget->GetParent();
    }
    return FALSE;
}
CFWL_WidgetImpDelegate::CFWL_WidgetImpDelegate()
{
}
FX_INT32 CFWL_WidgetImpDelegate::OnProcessMessage(CFWL_Message *pMessage)
{
    _FWL_RETURN_VALUE_IF_FAIL(pMessage->m_pDstTarget, 0);
    CFWL_WidgetImp *pWidget = (CFWL_WidgetImp*)((IFWL_TargetData*)pMessage->m_pDstTarget)->GetData();
    FX_DWORD dwMsgCode = pMessage->GetClassID();
    switch (dwMsgCode) {
        case FWL_MSGHASH_Mouse: {
                CFWL_EvtMouse evt;
                evt.m_pSrcTarget = pWidget->m_pInterface;
                evt.m_pDstTarget = pWidget->m_pInterface;
                evt.m_dwCmd = ((CFWL_MsgMouse*)pMessage)->m_dwCmd;
                evt.m_dwFlags = ((CFWL_MsgMouse*)pMessage)->m_dwFlags;
                evt.m_fx = ((CFWL_MsgMouse*)pMessage)->m_fx;
                evt.m_fy = ((CFWL_MsgMouse*)pMessage)->m_fy;
                pWidget->DispatchEvent(&evt);
                break;
            }
        case FWL_MSGHASH_MouseWheel: {
                CFWL_EvtMouseWheel evt;
                evt.m_pSrcTarget = pWidget->m_pInterface;
                evt.m_pDstTarget = pWidget->m_pInterface;
                evt.m_dwFlags = ((CFWL_MsgMouseWheel*)pMessage)->m_dwFlags;
                evt.m_fDeltaX = ((CFWL_MsgMouseWheel*)pMessage)->m_fDeltaX;
                evt.m_fDeltaY = ((CFWL_MsgMouseWheel*)pMessage)->m_fDeltaY;
                evt.m_fx = ((CFWL_MsgMouseWheel*)pMessage)->m_fx;
                evt.m_fy = ((CFWL_MsgMouseWheel*)pMessage)->m_fy;
                pWidget->DispatchEvent(&evt);
                break;
            }
        case FWL_MSGHASH_Key: {
                CFWL_EvtKey evt;
                evt.m_pSrcTarget = pWidget->m_pInterface;;
                evt.m_pDstTarget = pWidget->m_pInterface;;
                evt.m_dwKeyCode = ((CFWL_MsgKey*)pMessage)->m_dwKeyCode;
                evt.m_dwFlags = ((CFWL_MsgKey*)pMessage)->m_dwFlags;
                evt.m_dwCmd = ((CFWL_MsgKey*)pMessage)->m_dwCmd;
                pWidget->DispatchEvent(&evt);
                break;
            }
        case FWL_MSGHASH_SetFocus: {
                CFWL_EvtSetFocus evt;
                evt.m_pSrcTarget = ((CFWL_MsgSetFocus*)pMessage)->m_pDstTarget;
                evt.m_pDstTarget = ((CFWL_MsgSetFocus*)pMessage)->m_pDstTarget;
                evt.m_pSetFocus = pWidget->m_pInterface;
                pWidget->DispatchEvent(&evt);
                break;
            }
        case FWL_MSGHASH_KillFocus: {
                CFWL_EvtKillFocus evt;
                evt.m_pSrcTarget = ((CFWL_MsgKillFocus*)pMessage)->m_pDstTarget;
                evt.m_pDstTarget = ((CFWL_MsgKillFocus*)pMessage)->m_pDstTarget;
                evt.m_pKillFocus = pWidget->m_pInterface;
                pWidget->DispatchEvent(&evt);
                break;
            }
        default: {
            }
    }
    return 1;
}
FWL_ERR CFWL_WidgetImpDelegate::OnProcessEvent(CFWL_Event *pEvent)
{
    return FWL_ERR_Succeeded;
}
FWL_ERR	CFWL_WidgetImpDelegate::OnDrawWidget(CFX_Graphics *pGraphics, const CFX_Matrix *pMatrix )
{
    CFWL_EvtDraw evt;
    evt.m_pGraphics = pGraphics;
    return FWL_ERR_Succeeded;
}
class CFWL_Custom : public CFWL_WidgetImp
{
public:
    CFWL_Custom(IFWL_Widget *pOuter = NULL);
    CFWL_Custom(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    virtual	FWL_ERR		GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize = FALSE);
    virtual	FWL_ERR		Update();
    virtual FWL_ERR		SetProxy(IFWL_Proxy *pProxy);
protected:
    IFWL_Proxy	*m_pProxy;
};
CFWL_Custom::CFWL_Custom(IFWL_Widget *pOuter )
    : CFWL_WidgetImp(pOuter)
    , m_pProxy(NULL)
{
}
CFWL_Custom::CFWL_Custom(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
    : CFWL_WidgetImp(properties, pOuter)
    , m_pProxy(NULL)
{
}
FWL_ERR CFWL_Custom::GetWidgetRect(CFX_RectF &rect, FX_BOOL bAutoSize )
{
    if (m_pProxy && (m_pProxy->GetWidgetRect(rect, bAutoSize) == FWL_ERR_Succeeded)) {
        return FWL_ERR_Succeeded;
    }
    return CFWL_WidgetImp::GetWidgetRect(rect, bAutoSize);
}
FWL_ERR	CFWL_Custom::Update()
{
    if (m_pProxy) {
        return m_pProxy->Update();
    }
    return CFWL_WidgetImp::Update();
}
FWL_ERR	CFWL_Custom::SetProxy(IFWL_Proxy *pProxy)
{
    m_pProxy = pProxy;
    return FWL_ERR_Succeeded;
}
IFWL_Custom* IFWL_Custom::Create()
{
    return new IFWL_Custom;
}
IFWL_Custom::IFWL_Custom()
{
    m_pData = NULL;
}
IFWL_Custom::~IFWL_Custom()
{
    if (m_pData) {
        delete (CFWL_Custom*)m_pData;
        m_pData = NULL;
    }
}
FWL_ERR IFWL_Custom::Initialize(IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_Custom(pOuter);
    ((CFWL_Custom*)m_pData)->SetInterface(this);
    return ((CFWL_Custom*)m_pData)->Initialize();
}
FWL_ERR IFWL_Custom::Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter )
{
    m_pData = FX_NEW CFWL_Custom(properties, pOuter);
    ((CFWL_Custom*)m_pData)->SetInterface(this);
    return ((CFWL_Custom*)m_pData)->Initialize();
}
FWL_ERR IFWL_Custom::SetProxy(IFWL_Proxy *pProxy)
{
    return ((CFWL_Custom*)m_pData)->SetProxy(pProxy);
}
void FWL_SetWidgetRect(IFWL_Widget *widget, const CFX_RectF &rect)
{
    ((CFWL_WidgetImp*)((IFWL_TargetData*)widget)->GetData())->m_pProperties->m_rtWidget = rect;
}
void FWL_SetWidgetStates(IFWL_Widget *widget, FX_DWORD dwStates)
{
    ((CFWL_WidgetImp*)((IFWL_TargetData*)widget)->GetData())->m_pProperties->m_dwStates = dwStates;
}
void FWL_SetWidgetStyles(IFWL_Widget *widget, FX_DWORD dwStyles)
{
    ((CFWL_WidgetImp*)((IFWL_TargetData*)widget)->GetData())->m_pProperties->m_dwStyles = dwStyles;
}
FWL_ERR FWL_EnabelWidget(IFWL_Widget *widget, FX_BOOL bEnable)
{
    widget->SetStates(FWL_WGTSTATE_Disabled, !bEnable);
    IFWL_WidgetMgr *widgetMgr = FWL_GetWidgetMgr();
    IFWL_Widget *child = widgetMgr->GetWidget(widget, FWL_WGTRELATION_FirstChild);
    while (child) {
        FWL_EnabelWidget(child, bEnable);
        child = widgetMgr->GetWidget(child, FWL_WGTRELATION_NextSibling);
    }
    return FWL_ERR_Succeeded;
}
