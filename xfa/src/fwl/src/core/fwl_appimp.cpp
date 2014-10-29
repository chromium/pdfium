// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
#include "include/fwl_threadimp.h"
#include "include/fwl_noteimp.h"
#include "include/fwl_widgetmgrimp.h"
#include "include/fwl_appimp.h"
IFWL_App* IFWL_App::Create(IFWL_AdapterNative *pAdapter)
{
    return (IFWL_App*) FX_NEW CFWL_AppImp(pAdapter);
}
CFWL_AppImp::CFWL_AppImp(IFWL_AdapterNative *pAdapter)
    : m_pWidgetMgr(NULL)
    , m_pThemeProvider(NULL)
{
    if (!pAdapter) {
        pAdapter = FWL_CreateFuelAdapterNative();
        m_bFuelAdapter = TRUE;
    } else {
        m_bFuelAdapter = FALSE;
    }
    m_pAdapterNative = pAdapter;
}
CFWL_AppImp::~CFWL_AppImp()
{
    delete CFWL_ToolTipContainer::getInstance();
    if (m_bFuelAdapter) {
        FWL_ReleaseFuelAdapterNative(m_pAdapterNative);
        m_pAdapterNative = NULL;
    }
    if (m_pWidgetMgr) {
        delete m_pWidgetMgr;
        m_pWidgetMgr = NULL;
    }
}
FWL_ERR CFWL_AppImp::Initialize()
{
    if (!m_pWidgetMgr) {
        m_pWidgetMgr = FX_NEW CFWL_WidgetMgr(m_pAdapterNative);
    }
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_AppImp::Finalize()
{
    if (m_pWidgetMgr) {
        delete m_pWidgetMgr;
        m_pWidgetMgr = NULL;
    }
    return FWL_ERR_Succeeded;
}
IFWL_AdapterNative* CFWL_AppImp::GetAdapterNative()
{
    return m_pAdapterNative;
}
IFWL_AdapterWidgetMgr* FWL_GetAdapterWidgetMgr()
{
    return ((CFWL_WidgetMgr*)FWL_GetWidgetMgr())->GetAdapterWidgetMgr();
}
IFWL_WidgetMgr*	CFWL_AppImp::GetWidgetMgr()
{
    return (IFWL_WidgetMgr*)m_pWidgetMgr;
}
FWL_ERR CFWL_AppImp::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider)
{
    m_pThemeProvider = pThemeProvider;
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_AppImp::Exit(FX_INT32 iExitCode )
{
    while (m_pNoteDriver->PopNoteLoop());
    return m_pWidgetMgr->GetAdapterWidgetMgr()->Exit(0);
}
IFWL_ThemeProvider*	CFWL_AppImp::GetThemeProvider()
{
    return m_pThemeProvider;
}
IFWL_AdapterNative*	FWL_GetAdapterNative()
{
    IFWL_App* pApp = FWL_GetApp();
    _FWL_RETURN_VALUE_IF_FAIL(pApp, NULL);
    return pApp->GetAdapterNative();
}
IFWL_ThemeProvider*	FWL_GetThemeProvider()
{
    return NULL;
}
static IFWL_App *_theApp = NULL;
IFWL_App* FWL_GetApp()
{
    return _theApp;
}
void FWL_SetApp(IFWL_App *pApp)
{
    _theApp = pApp;
}
FWL_ERR FWL_SetFullScreen(IFWL_Widget *pWidget, FX_BOOL bFullScreen)
{
    _FWL_RETURN_VALUE_IF_FAIL(pWidget, FWL_ERR_Succeeded);
    IFWL_NoteThread *pNoteTread = pWidget->GetOwnerThread();
    _FWL_RETURN_VALUE_IF_FAIL(pNoteTread, FWL_ERR_Succeeded);
    CFWL_NoteDriver *pNoteDriver = (CFWL_NoteDriver*)pNoteTread->GetNoteDriver();
    _FWL_RETURN_VALUE_IF_FAIL(pNoteTread, FWL_ERR_Succeeded);
    pNoteDriver->NotifyFullScreenMode(pWidget, bFullScreen);
    return FWL_GetAdapterWidgetMgr()->SetFullScreen(pWidget, bFullScreen);
}
