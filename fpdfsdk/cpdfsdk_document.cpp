// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_document.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_docjsactions.h"
#include "core/fpdfdoc/cpdf_occontext.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annothandlermgr.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/fsdk_actionhandler.h"
#include "third_party/base/ptr_util.h"

// static
CPDFSDK_Document* CPDFSDK_Document::FromFPDFFormHandle(
    FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pEnv =
      static_cast<CPDFSDK_FormFillEnvironment*>(hHandle);
  return pEnv ? pEnv->GetSDKDocument() : nullptr;
}

CPDFSDK_Document::CPDFSDK_Document(UnderlyingDocumentType* pDoc,
                                   CPDFSDK_FormFillEnvironment* pEnv)
    : m_pDoc(pDoc),
      m_pEnv(pEnv),
      m_bChangeMask(FALSE),
      m_bBeingDestroyed(FALSE) {}

CPDFSDK_Document::~CPDFSDK_Document() {
  m_bBeingDestroyed = TRUE;

  ClearAllFocusedAnnots();
  for (auto& it : m_pageMap)
    delete it.second;
  m_pageMap.clear();
}

void CPDFSDK_Document::ClearAllFocusedAnnots() {
  for (auto& it : m_pageMap) {
    if (it.second->IsValidSDKAnnot(GetFocusAnnot()))
      KillFocusAnnot(0);
  }
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(
    UnderlyingPageType* pUnderlyingPage,
    bool ReNew) {
  auto it = m_pageMap.find(pUnderlyingPage);
  if (it != m_pageMap.end())
    return it->second;

  if (!ReNew)
    return nullptr;

  CPDFSDK_PageView* pPageView = new CPDFSDK_PageView(this, pUnderlyingPage);
  m_pageMap[pUnderlyingPage] = pPageView;
  // Delay to load all the annotations, to avoid endless loop.
  pPageView->LoadFXAnnots();
  return pPageView;
}

CPDFSDK_PageView* CPDFSDK_Document::GetCurrentView() {
  UnderlyingPageType* pPage =
      UnderlyingFromFPDFPage(m_pEnv->GetCurrentPage(m_pDoc));
  return pPage ? GetPageView(pPage, true) : nullptr;
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(int nIndex) {
  UnderlyingPageType* pTempPage =
      UnderlyingFromFPDFPage(m_pEnv->GetPage(m_pDoc, nIndex));
  if (!pTempPage)
    return nullptr;

  auto it = m_pageMap.find(pTempPage);
  return it != m_pageMap.end() ? it->second : nullptr;
}

void CPDFSDK_Document::ProcJavascriptFun() {
  CPDF_Document* pPDFDoc = GetPDFDocument();
  CPDF_DocJSActions docJS(pPDFDoc);
  int iCount = docJS.CountJSActions();
  if (iCount < 1)
    return;
  for (int i = 0; i < iCount; i++) {
    CFX_ByteString csJSName;
    CPDF_Action jsAction = docJS.GetJSAction(i, csJSName);
    if (m_pEnv->GetActionHander())
      m_pEnv->GetActionHander()->DoAction_JavaScript(
          jsAction, CFX_WideString::FromLocal(csJSName.AsStringC()), this);
  }
}

FX_BOOL CPDFSDK_Document::ProcOpenAction() {
  if (!m_pDoc)
    return FALSE;

  CPDF_Dictionary* pRoot = GetPDFDocument()->GetRoot();
  if (!pRoot)
    return FALSE;

  CPDF_Object* pOpenAction = pRoot->GetDictFor("OpenAction");
  if (!pOpenAction)
    pOpenAction = pRoot->GetArrayFor("OpenAction");

  if (!pOpenAction)
    return FALSE;

  if (pOpenAction->IsArray())
    return TRUE;

  if (CPDF_Dictionary* pDict = pOpenAction->AsDictionary()) {
    CPDF_Action action(pDict);
    if (m_pEnv->GetActionHander())
      m_pEnv->GetActionHander()->DoAction_DocOpen(action, this);
    return TRUE;
  }
  return FALSE;
}

void CPDFSDK_Document::RemovePageView(UnderlyingPageType* pUnderlyingPage) {
  auto it = m_pageMap.find(pUnderlyingPage);
  if (it == m_pageMap.end())
    return;

  CPDFSDK_PageView* pPageView = it->second;
  if (pPageView->IsLocked() || pPageView->IsBeingDestroyed())
    return;

  // Mark the page view so we do not come into |RemovePageView| a second
  // time while we're in the process of removing.
  pPageView->SetBeingDestroyed();

  // This must happen before we remove |pPageView| from the map because
  // |KillFocusAnnot| can call into the |GetPage| method which will
  // look for this page view in the map, if it doesn't find it a new one will
  // be created. We then have two page views pointing to the same page and
  // bad things happen.
  if (pPageView->IsValidSDKAnnot(GetFocusAnnot()))
    KillFocusAnnot(0);

  // Remove the page from the map to make sure we don't accidentally attempt
  // to use the |pPageView| while we're cleaning it up.
  m_pageMap.erase(it);

  delete pPageView;
}

UnderlyingPageType* CPDFSDK_Document::GetPage(int nIndex) {
  return UnderlyingFromFPDFPage(m_pEnv->GetPage(m_pDoc, nIndex));
}

CPDFSDK_InterForm* CPDFSDK_Document::GetInterForm() {
  if (!m_pInterForm)
    m_pInterForm = pdfium::MakeUnique<CPDFSDK_InterForm>(this);
  return m_pInterForm.get();
}

void CPDFSDK_Document::UpdateAllViews(CPDFSDK_PageView* pSender,
                                      CPDFSDK_Annot* pAnnot) {
  for (const auto& it : m_pageMap) {
    CPDFSDK_PageView* pPageView = it.second;
    if (pPageView != pSender)
      pPageView->UpdateView(pAnnot);
  }
}

FX_BOOL CPDFSDK_Document::SetFocusAnnot(CPDFSDK_Annot::ObservedPtr* pAnnot) {
  if (m_bBeingDestroyed)
    return FALSE;
  if (m_pFocusAnnot == *pAnnot)
    return TRUE;
  if (m_pFocusAnnot && !KillFocusAnnot(0))
    return FALSE;
  if (!*pAnnot)
    return FALSE;

#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot::ObservedPtr pLastFocusAnnot(m_pFocusAnnot.Get());
#endif  // PDF_ENABLE_XFA
  CPDFSDK_PageView* pPageView = (*pAnnot)->GetPageView();
  if (pPageView && pPageView->IsValid()) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandler = m_pEnv->GetAnnotHandlerMgr();
    if (!m_pFocusAnnot) {
#ifdef PDF_ENABLE_XFA
      if (!pAnnotHandler->Annot_OnChangeFocus(pAnnot, &pLastFocusAnnot))
        return FALSE;
#endif  // PDF_ENABLE_XFA
      if (!pAnnotHandler->Annot_OnSetFocus(pAnnot, 0))
        return FALSE;
      if (!m_pFocusAnnot) {
        m_pFocusAnnot.Reset(pAnnot->Get());
        return TRUE;
      }
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_Document::KillFocusAnnot(uint32_t nFlag) {
  if (m_pFocusAnnot) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandler = m_pEnv->GetAnnotHandlerMgr();
    CPDFSDK_Annot::ObservedPtr pFocusAnnot(m_pFocusAnnot.Get());
    m_pFocusAnnot.Reset();

#ifdef PDF_ENABLE_XFA
    CPDFSDK_Annot::ObservedPtr pNull;
    if (!pAnnotHandler->Annot_OnChangeFocus(&pNull, &pFocusAnnot))
      return FALSE;
#endif  // PDF_ENABLE_XFA

    if (pAnnotHandler->Annot_OnKillFocus(&pFocusAnnot, nFlag)) {
      if (pFocusAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET) {
        CPDFSDK_Widget* pWidget =
            static_cast<CPDFSDK_Widget*>(pFocusAnnot.Get());
        int nFieldType = pWidget->GetFieldType();
        if (FIELDTYPE_TEXTFIELD == nFieldType ||
            FIELDTYPE_COMBOBOX == nFieldType) {
          m_pEnv->OnSetFieldInputFocus(nullptr, 0, FALSE);
        }
      }
      if (!m_pFocusAnnot)
        return TRUE;
    } else {
      m_pFocusAnnot.Reset(pFocusAnnot.Get());
    }
  }
  return FALSE;
}

void CPDFSDK_Document::OnCloseDocument() {
  KillFocusAnnot(0);
}

FX_BOOL CPDFSDK_Document::GetPermissions(int nFlag) {
  return GetPDFDocument()->GetUserPermissions() & nFlag;
}

IJS_Runtime* CPDFSDK_Document::GetJsRuntime() {
  return m_pEnv->GetJSRuntime();
}

CFX_WideString CPDFSDK_Document::GetPath() {
  return m_pEnv->JS_docGetFilePath();
}
