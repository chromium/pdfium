// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/include/fpdfxfa_doc.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_app.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_page.h"
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_util.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/javascript/ijs_runtime.h"
#include "public/fpdf_formfill.h"
#include "xfa/fxfa/include/cxfa_eventparam.h"
#include "xfa/fxfa/include/xfa_ffapp.h"
#include "xfa/fxfa/include/xfa_ffdoc.h"
#include "xfa/fxfa/include/xfa_ffdocview.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidgethandler.h"

#ifndef _WIN32
extern void SetLastError(int err);
extern int GetLastError();
#endif

CPDFXFA_Document::CPDFXFA_Document(std::unique_ptr<CPDF_Document> pPDFDoc,
                                   CPDFXFA_App* pProvider)
    : m_iDocType(DOCTYPE_PDF),
      m_pPDFDoc(std::move(pPDFDoc)),
      m_pXFADocView(nullptr),
      m_pApp(pProvider),
      m_nLoadStatus(FXFA_LOADSTATUS_PRELOAD),
      m_nPageCount(0),
      m_DocEnv(this) {}

CPDFXFA_Document::~CPDFXFA_Document() {
  m_nLoadStatus = FXFA_LOADSTATUS_CLOSING;

  if (m_pXFADoc) {
    CXFA_FFApp* pApp = m_pApp->GetXFAApp();
    if (pApp) {
      CXFA_FFDocHandler* pDocHandler = pApp->GetDocHandler();
      if (pDocHandler)
        CloseXFADoc(pDocHandler);
    }
    m_pXFADoc.reset();
  }

  m_nLoadStatus = FXFA_LOADSTATUS_CLOSED;
}

FX_BOOL CPDFXFA_Document::LoadXFADoc() {
  m_nLoadStatus = FXFA_LOADSTATUS_LOADING;

  if (!m_pPDFDoc)
    return FALSE;

  m_XFAPageList.RemoveAll();

  CXFA_FFApp* pApp = m_pApp->GetXFAApp();
  if (!pApp)
    return FALSE;

  m_pXFADoc.reset(pApp->CreateDoc(&m_DocEnv, m_pPDFDoc.get()));
  if (!m_pXFADoc) {
    SetLastError(FPDF_ERR_XFALOAD);
    return FALSE;
  }

  CXFA_FFDocHandler* pDocHandler = pApp->GetDocHandler();
  if (!pDocHandler) {
    SetLastError(FPDF_ERR_XFALOAD);
    return FALSE;
  }

  m_pXFADoc->StartLoad();
  int iStatus = m_pXFADoc->DoLoad(nullptr);
  if (iStatus != XFA_PARSESTATUS_Done) {
    CloseXFADoc(pDocHandler);
    SetLastError(FPDF_ERR_XFALOAD);
    return FALSE;
  }
  m_pXFADoc->StopLoad();
  m_pXFADoc->GetXFADoc()->InitScriptContext(m_pApp->GetJSERuntime());

  if (m_pXFADoc->GetDocType() == XFA_DOCTYPE_Dynamic)
    m_iDocType = DOCTYPE_DYNAMIC_XFA;
  else
    m_iDocType = DOCTYPE_STATIC_XFA;

  m_pXFADocView = m_pXFADoc->CreateDocView(XFA_DOCVIEW_View);
  if (m_pXFADocView->StartLayout() < 0) {
    CloseXFADoc(pDocHandler);
    SetLastError(FPDF_ERR_XFALAYOUT);
    return FALSE;
  }

  m_pXFADocView->DoLayout(nullptr);
  m_pXFADocView->StopLayout();
  m_nLoadStatus = FXFA_LOADSTATUS_LOADED;

  return TRUE;
}

int CPDFXFA_Document::GetPageCount() const {
  if (!m_pPDFDoc && !m_pXFADoc)
    return 0;

  switch (m_iDocType) {
    case DOCTYPE_PDF:
    case DOCTYPE_STATIC_XFA:
      if (m_pPDFDoc)
        return m_pPDFDoc->GetPageCount();
    case DOCTYPE_DYNAMIC_XFA:
      if (m_pXFADoc)
        return m_pXFADocView->CountPageViews();
    default:
      return 0;
  }
}

CPDFXFA_Page* CPDFXFA_Document::GetXFAPage(int page_index) {
  if (page_index < 0)
    return nullptr;

  CPDFXFA_Page* pPage = nullptr;
  int nCount = m_XFAPageList.GetSize();
  if (nCount > 0 && page_index < nCount) {
    pPage = m_XFAPageList.GetAt(page_index);
    if (pPage)
      pPage->Retain();
  } else {
    m_nPageCount = GetPageCount();
    m_XFAPageList.SetSize(m_nPageCount);
  }
  if (pPage)
    return pPage;

  pPage = new CPDFXFA_Page(this, page_index);
  if (!pPage->LoadPage()) {
    pPage->Release();
    return nullptr;
  }
  m_XFAPageList.SetAt(page_index, pPage);
  return pPage;
}

CPDFXFA_Page* CPDFXFA_Document::GetXFAPage(CXFA_FFPageView* pPage) const {
  if (!pPage)
    return nullptr;

  if (!m_pXFADoc)
    return nullptr;

  if (m_iDocType != DOCTYPE_DYNAMIC_XFA)
    return nullptr;

  int nSize = m_XFAPageList.GetSize();
  for (int i = 0; i < nSize; i++) {
    CPDFXFA_Page* pTempPage = m_XFAPageList.GetAt(i);
    if (!pTempPage)
      continue;
    if (pTempPage->GetXFAPageView() && pTempPage->GetXFAPageView() == pPage)
      return pTempPage;
  }

  return nullptr;
}

void CPDFXFA_Document::DeletePage(int page_index) {
  // Delete from the document first because, if GetPage was never called for
  // this |page_index| then |m_XFAPageList| may have size < |page_index| even
  // if it's a valid page in the document.
  if (m_pPDFDoc)
    m_pPDFDoc->DeletePage(page_index);

  if (page_index < 0 || page_index >= m_XFAPageList.GetSize())
    return;

  if (CPDFXFA_Page* pPage = m_XFAPageList.GetAt(page_index))
    pPage->Release();
}

void CPDFXFA_Document::RemovePage(CPDFXFA_Page* page) {
  m_XFAPageList.SetAt(page->GetPageIndex(), nullptr);
}

CPDFSDK_Document* CPDFXFA_Document::GetSDKDocument(
    CPDFSDK_Environment* pFormFillEnv) {
  if (!m_pSDKDoc && pFormFillEnv)
    m_pSDKDoc.reset(new CPDFSDK_Document(this, pFormFillEnv));
  return m_pSDKDoc.get();
}

void CPDFXFA_Document::ClearChangeMark() {
  if (m_pSDKDoc)
    m_pSDKDoc->ClearChangeMark();
}
