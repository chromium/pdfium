// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/fwl_appimp.h"

#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/fwl_targetimp.h"
#include "xfa/fwl/core/fwl_threadimp.h"
#include "xfa/fwl/core/fwl_widgetmgrimp.h"
#include "xfa/fwl/core/ifwl_adapterwidgetmgr.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_widget.h"

IFWL_App* IFWL_App::Create(IFWL_AdapterNative* pAdapter) {
  IFWL_App* pApp = new IFWL_App;
  pApp->SetImpl(new CFWL_AppImp(pApp, pAdapter));
  return pApp;
}
FWL_ERR IFWL_App::Initialize() {
  return static_cast<CFWL_AppImp*>(GetImpl())->Initialize();
}
FWL_ERR IFWL_App::Finalize() {
  return static_cast<CFWL_AppImp*>(GetImpl())->Finalize();
}
IFWL_AdapterNative* IFWL_App::GetAdapterNative() {
  return static_cast<CFWL_AppImp*>(GetImpl())->GetAdapterNative();
}
IFWL_WidgetMgr* IFWL_App::GetWidgetMgr() {
  return static_cast<CFWL_AppImp*>(GetImpl())->GetWidgetMgr();
}
IFWL_ThemeProvider* IFWL_App::GetThemeProvider() {
  return static_cast<CFWL_AppImp*>(GetImpl())->GetThemeProvider();
}
FWL_ERR IFWL_App::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  return static_cast<CFWL_AppImp*>(GetImpl())->SetThemeProvider(pThemeProvider);
}
FWL_ERR IFWL_App::Exit(int32_t iExitCode) {
  return static_cast<CFWL_AppImp*>(GetImpl())->Exit(iExitCode);
}

CFWL_AppImp::CFWL_AppImp(IFWL_App* pIface, IFWL_AdapterNative* pAdapter)
    : CFWL_NoteThreadImp(pIface),
      m_pAdapterNative(pAdapter),
      m_pThemeProvider(nullptr) {}

CFWL_AppImp::~CFWL_AppImp() {
  CFWL_ToolTipContainer::DeleteInstance();
}

FWL_ERR CFWL_AppImp::Initialize() {
  if (!m_pWidgetMgr) {
    m_pWidgetMgr.reset(new CFWL_WidgetMgr(m_pAdapterNative));
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_AppImp::Finalize() {
  m_pWidgetMgr.reset();
  return FWL_ERR_Succeeded;
}
IFWL_AdapterNative* CFWL_AppImp::GetAdapterNative() const {
  return m_pAdapterNative;
}
IFWL_AdapterWidgetMgr* FWL_GetAdapterWidgetMgr() {
  return static_cast<CFWL_WidgetMgr*>(FWL_GetWidgetMgr())
      ->GetAdapterWidgetMgr();
}
IFWL_WidgetMgr* CFWL_AppImp::GetWidgetMgr() const {
  return m_pWidgetMgr.get();
}
FWL_ERR CFWL_AppImp::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  m_pThemeProvider = pThemeProvider;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_AppImp::Exit(int32_t iExitCode) {
  while (m_pNoteDriver->PopNoteLoop()) {
    continue;
  }
  return m_pWidgetMgr->GetAdapterWidgetMgr()->Exit(0);
}
IFWL_ThemeProvider* CFWL_AppImp::GetThemeProvider() const {
  return m_pThemeProvider;
}
IFWL_AdapterNative* FWL_GetAdapterNative() {
  IFWL_App* pApp = FWL_GetApp();
  if (!pApp)
    return NULL;
  return pApp->GetAdapterNative();
}
IFWL_ThemeProvider* FWL_GetThemeProvider() {
  return NULL;
}
static IFWL_App* _theApp = NULL;
IFWL_App* FWL_GetApp() {
  return _theApp;
}
void FWL_SetApp(IFWL_App* pApp) {
  _theApp = pApp;
}
FWL_ERR FWL_SetFullScreen(IFWL_Widget* pWidget, FX_BOOL bFullScreen) {
  if (!pWidget)
    return FWL_ERR_Succeeded;
  IFWL_NoteThread* pNoteTread = pWidget->GetOwnerThread();
  if (!pNoteTread)
    return FWL_ERR_Succeeded;
  CFWL_NoteDriver* pNoteDriver =
      static_cast<CFWL_NoteDriver*>(pNoteTread->GetNoteDriver());
  if (!pNoteTread)
    return FWL_ERR_Succeeded;
  pNoteDriver->NotifyFullScreenMode(pWidget, bFullScreen);
  return FWL_GetAdapterWidgetMgr()->SetFullScreen(pWidget, bFullScreen);
}
