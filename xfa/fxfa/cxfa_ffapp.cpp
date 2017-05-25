// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffapp.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/app/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/app/cxfa_fwltheme.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdochandler.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fontmgr.h"

CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider) : m_pProvider(pProvider) {
  // Ensure fully initialized before making an app based on |this|.
  m_pFWLApp = pdfium::MakeUnique<CFWL_App>(this);
}

CXFA_FFApp::~CXFA_FFApp() {}

CXFA_FFDocHandler* CXFA_FFApp::GetDocHandler() {
  if (!m_pDocHandler)
    m_pDocHandler = pdfium::MakeUnique<CXFA_FFDocHandler>();
  return m_pDocHandler.get();
}

std::unique_ptr<CXFA_FFDoc> CXFA_FFApp::CreateDoc(
    IXFA_DocEnvironment* pDocEnvironment,
    CPDF_Document* pPDFDoc) {
  if (!pPDFDoc)
    return nullptr;

  auto pDoc = pdfium::MakeUnique<CXFA_FFDoc>(this, pDocEnvironment);
  if (!pDoc->OpenDoc(pPDFDoc))
    return nullptr;

  return pDoc;
}

void CXFA_FFApp::SetDefaultFontMgr(std::unique_ptr<CXFA_DefFontMgr> pFontMgr) {
  if (!m_pFontMgr)
    m_pFontMgr = pdfium::MakeUnique<CXFA_FontMgr>();
  m_pFontMgr->SetDefFontMgr(std::move(pFontMgr));
}

CXFA_FontMgr* CXFA_FFApp::GetXFAFontMgr() const {
  return m_pFontMgr.get();
}

CFGAS_FontMgr* CXFA_FFApp::GetFDEFontMgr() {
  if (!m_pFDEFontMgr) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    m_pFDEFontMgr = CFGAS_FontMgr::Create(FX_GetDefFontEnumerator());
#else
    m_pFontSource = pdfium::MakeUnique<CFX_FontSourceEnum_File>();
    m_pFDEFontMgr = CFGAS_FontMgr::Create(m_pFontSource.get());
#endif
  }
  return m_pFDEFontMgr.get();
}

CXFA_FWLTheme* CXFA_FFApp::GetFWLTheme() {
  if (!m_pFWLTheme)
    m_pFWLTheme = pdfium::MakeUnique<CXFA_FWLTheme>(this);
  return m_pFWLTheme.get();
}

CXFA_FWLAdapterWidgetMgr* CXFA_FFApp::GetWidgetMgr(
    CFWL_WidgetMgrDelegate* pDelegate) {
  if (!m_pAdapterWidgetMgr) {
    m_pAdapterWidgetMgr = pdfium::MakeUnique<CXFA_FWLAdapterWidgetMgr>();
    pDelegate->OnSetCapability(FWL_WGTMGR_DisableForm);
    m_pWidgetMgrDelegate = pDelegate;
  }
  return m_pAdapterWidgetMgr.get();
}

IFWL_AdapterTimerMgr* CXFA_FFApp::GetTimerMgr() const {
  return m_pProvider->GetTimerMgr();
}

void CXFA_FFApp::ClearEventTargets() {
  m_pFWLApp->GetNoteDriver()->ClearEventTargets();
}
