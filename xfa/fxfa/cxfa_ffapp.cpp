// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffapp.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/cxfa_fwltheme.h"

namespace {

bool g_skipFontLoadForTesting = false;

}  // namespace

// static
void CXFA_FFApp::SkipFontLoadForTesting(bool skip) {
  g_skipFontLoadForTesting = skip;
}

CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider) : m_pProvider(pProvider) {
  // Ensure fully initialized before making an app based on |this|.
  m_pFWLApp = std::make_unique<CFWL_App>(this);
}

CXFA_FFApp::~CXFA_FFApp() = default;

CFGAS_FontMgr* CXFA_FFApp::GetFDEFontMgr() {
  if (!m_pFDEFontMgr) {
    m_pFDEFontMgr = std::make_unique<CFGAS_FontMgr>();
    if (!g_skipFontLoadForTesting) {
      if (!m_pFDEFontMgr->EnumFonts())
        m_pFDEFontMgr = nullptr;
    }
  }
  return m_pFDEFontMgr.get();
}

bool CXFA_FFApp::LoadFWLTheme(CXFA_FFDoc* doc) {
  auto fwl_theme = std::make_unique<CXFA_FWLTheme>(this);
  if (!fwl_theme->LoadCalendarFont(doc))
    return false;

  m_pFWLTheme = std::move(fwl_theme);
  return true;
}

CFWL_WidgetMgr::AdapterIface* CXFA_FFApp::GetWidgetMgrAdapter() {
  if (!m_pAdapterWidgetMgr)
    m_pAdapterWidgetMgr = std::make_unique<CXFA_FWLAdapterWidgetMgr>();
  return m_pAdapterWidgetMgr.get();
}

TimerHandlerIface* CXFA_FFApp::GetTimerHandler() {
  return m_pProvider->GetTimerHandler();
}

IFWL_ThemeProvider* CXFA_FFApp::GetThemeProvider() {
  return m_pFWLTheme.get();
}
