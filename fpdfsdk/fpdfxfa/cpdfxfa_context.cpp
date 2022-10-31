// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"

#include <stdint.h>

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_seekablemultistream.h"
#include "core/fxcrt/autonuller.h"
#include "core/fxcrt/fixed_zeroed_data_vector.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_docenvironment.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "fxbarcode/BC_Library.h"
#include "fxjs/cjs_runtime.h"
#include "fxjs/ijs_runtime.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/check.h"
#include "v8/include/cppgc/allocation.h"
#include "xfa/fgas/font/cfgas_gemodule.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_readynodeiterator.h"

namespace {

bool IsValidAlertButton(int type) {
  return type == JSPLATFORM_ALERT_BUTTON_OK ||
         type == JSPLATFORM_ALERT_BUTTON_OKCANCEL ||
         type == JSPLATFORM_ALERT_BUTTON_YESNO ||
         type == JSPLATFORM_ALERT_BUTTON_YESNOCANCEL;
}

bool IsValidAlertIcon(int type) {
  return type == JSPLATFORM_ALERT_ICON_ERROR ||
         type == JSPLATFORM_ALERT_ICON_WARNING ||
         type == JSPLATFORM_ALERT_ICON_QUESTION ||
         type == JSPLATFORM_ALERT_ICON_STATUS ||
         type == JSPLATFORM_ALERT_ICON_ASTERISK;
}

RetainPtr<CPDF_SeekableMultiStream> CreateXFAMultiStream(
    const CPDF_Document* pPDFDoc) {
  const CPDF_Dictionary* pRoot = pPDFDoc->GetRoot();
  if (!pRoot)
    return nullptr;

  RetainPtr<const CPDF_Dictionary> pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm)
    return nullptr;

  RetainPtr<const CPDF_Object> pElementXFA =
      pAcroForm->GetDirectObjectFor("XFA");
  if (!pElementXFA)
    return nullptr;

  std::vector<RetainPtr<const CPDF_Stream>> xfa_streams;
  if (pElementXFA->IsArray()) {
    const CPDF_Array* pXFAArray = pElementXFA->AsArray();
    for (size_t i = 0; i < pXFAArray->size() / 2; i++) {
      RetainPtr<const CPDF_Stream> pStream = pXFAArray->GetStreamAt(i * 2 + 1);
      if (pStream)
        xfa_streams.push_back(std::move(pStream));
    }
  } else if (pElementXFA->IsStream()) {
    xfa_streams.push_back(ToStream(pElementXFA));
  }
  if (xfa_streams.empty())
    return nullptr;

  return pdfium::MakeRetain<CPDF_SeekableMultiStream>(std::move(xfa_streams));
}

}  // namespace

void CPDFXFA_ModuleInit() {
  CFGAS_GEModule::Create();
  BC_Library_Init();
}

void CPDFXFA_ModuleDestroy() {
  BC_Library_Destroy();
  CFGAS_GEModule::Destroy();
}

CPDFXFA_Context::CPDFXFA_Context(CPDF_Document* pPDFDoc)
    : m_pPDFDoc(pPDFDoc),
      m_pDocEnv(std::make_unique<CPDFXFA_DocEnvironment>(this)),
      m_pGCHeap(FXGC_CreateHeap()) {
  DCHECK(m_pPDFDoc);

  // There might not be a heap when JS not initialized.
  if (m_pGCHeap) {
    m_pXFAApp = cppgc::MakeGarbageCollected<CXFA_FFApp>(
        m_pGCHeap->GetAllocationHandle(), this);
  }
}

CPDFXFA_Context::~CPDFXFA_Context() {
  m_nLoadStatus = LoadStatus::kClosing;
  if (m_pFormFillEnv)
    m_pFormFillEnv->ClearAllFocusedAnnots();
}

void CPDFXFA_Context::SetFormFillEnv(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  // The layout data can have pointers back into the script context. That
  // context will be different if the form fill environment closes, so, force
  // the layout data to clear.
  if (m_pXFADoc && m_pXFADoc->GetXFADoc()) {
    m_pXFADoc->GetXFADoc()->ClearLayoutData();
    m_pXFADocView.Clear();
    m_pXFADoc.Clear();
    m_pXFAApp.Clear();
    FXGC_ForceGarbageCollection(m_pGCHeap.get());
  }
  m_pFormFillEnv.Reset(pFormFillEnv);
}

bool CPDFXFA_Context::LoadXFADoc() {
  m_nLoadStatus = LoadStatus::kLoading;
  m_XFAPageList.clear();

  CJS_Runtime* actual_runtime = GetCJSRuntime();  // Null if a stub.
  if (!actual_runtime) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  auto stream = CreateXFAMultiStream(m_pPDFDoc);
  if (!stream) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  CFX_XMLParser parser(stream);
  m_pXML = parser.Parse();
  if (!m_pXML) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  AutoNuller<cppgc::Persistent<CXFA_FFDoc>> doc_nuller(&m_pXFADoc);
  m_pXFADoc = cppgc::MakeGarbageCollected<CXFA_FFDoc>(
      m_pGCHeap->GetAllocationHandle(), m_pXFAApp, m_pDocEnv.get(), m_pPDFDoc,
      m_pGCHeap.get());

  if (!m_pXFADoc->OpenDoc(m_pXML.get())) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  if (!m_pXFAApp->LoadFWLTheme(m_pXFADoc)) {
    FXSYS_SetLastError(FPDF_ERR_XFALAYOUT);
    return false;
  }

  m_pXFADoc->GetXFADoc()->InitScriptContext(actual_runtime);
  if (m_pXFADoc->GetFormType() == FormType::kXFAFull)
    m_FormType = FormType::kXFAFull;
  else
    m_FormType = FormType::kXFAForeground;

  AutoNuller<cppgc::Persistent<CXFA_FFDocView>> view_nuller(&m_pXFADocView);
  m_pXFADocView = m_pXFADoc->CreateDocView();

  if (m_pXFADocView->StartLayout() < 0) {
    m_pXFADoc->GetXFADoc()->ClearLayoutData();
    FXGC_ForceGarbageCollection(m_pGCHeap.get());
    FXSYS_SetLastError(FPDF_ERR_XFALAYOUT);
    return false;
  }

  m_pXFADocView->DoLayout();
  m_pXFADocView->StopLayout();

  view_nuller.AbandonNullification();
  doc_nuller.AbandonNullification();
  m_nLoadStatus = LoadStatus::kLoaded;
  return true;
}

int CPDFXFA_Context::GetPageCount() const {
  switch (m_FormType) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      return m_pPDFDoc->GetPageCount();
    case FormType::kXFAFull:
      return m_pXFADoc ? m_pXFADocView->CountPageViews() : 0;
  }
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetOrCreateXFAPage(int page_index) {
  if (page_index < 0)
    return nullptr;

  if (fxcrt::IndexInBounds(m_XFAPageList, page_index)) {
    if (m_XFAPageList[page_index])
      return m_XFAPageList[page_index];
  } else {
    m_nPageCount = GetPageCount();
    m_XFAPageList.resize(m_nPageCount);
  }

  auto pPage = pdfium::MakeRetain<CPDFXFA_Page>(GetPDFDoc(), page_index);
  if (!pPage->LoadPage())
    return nullptr;

  if (fxcrt::IndexInBounds(m_XFAPageList, page_index))
    m_XFAPageList[page_index] = pPage;

  return pPage;
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetXFAPage(int page_index) {
  if (!fxcrt::IndexInBounds(m_XFAPageList, page_index))
    return nullptr;

  return m_XFAPageList[page_index];
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetXFAPage(
    CXFA_FFPageView* pPage) const {
  if (!pPage)
    return nullptr;

  if (!m_pXFADoc)
    return nullptr;

  if (m_FormType != FormType::kXFAFull)
    return nullptr;

  for (auto& pTempPage : m_XFAPageList) {
    if (pTempPage && pTempPage->GetXFAPageView() == pPage)
      return pTempPage;
  }
  return nullptr;
}

void CPDFXFA_Context::DeletePage(int page_index) {
  // Delete from the document first because, if GetPage was never called for
  // this |page_index| then |m_XFAPageList| may have size < |page_index| even
  // if it's a valid page in the document.
  m_pPDFDoc->DeletePage(page_index);

  if (fxcrt::IndexInBounds(m_XFAPageList, page_index))
    m_XFAPageList[page_index].Reset();
}

uint32_t CPDFXFA_Context::GetUserPermissions() const {
  // See https://bugs.chromium.org/p/pdfium/issues/detail?id=499
  return 0xFFFFFFFF;
}

bool CPDFXFA_Context::ContainsExtensionForm() const {
  return m_FormType == FormType::kXFAFull ||
         m_FormType == FormType::kXFAForeground;
}

bool CPDFXFA_Context::ContainsExtensionFullForm() const {
  return m_FormType == FormType::kXFAFull;
}

bool CPDFXFA_Context::ContainsExtensionForegroundForm() const {
  return m_FormType == FormType::kXFAForeground;
}

void CPDFXFA_Context::ClearChangeMark() {
  if (m_pFormFillEnv)
    m_pFormFillEnv->ClearChangeMark();
}

CJS_Runtime* CPDFXFA_Context::GetCJSRuntime() const {
  if (!m_pFormFillEnv)
    return nullptr;

  return m_pFormFillEnv->GetIJSRuntime()->AsCJSRuntime();
}

WideString CPDFXFA_Context::GetAppTitle() const {
  return L"PDFium";
}

WideString CPDFXFA_Context::GetAppName() {
  return m_pFormFillEnv ? m_pFormFillEnv->FFI_GetAppName() : WideString();
}

WideString CPDFXFA_Context::GetLanguage() {
  return m_pFormFillEnv ? m_pFormFillEnv->GetLanguage() : WideString();
}

WideString CPDFXFA_Context::GetPlatform() {
  return m_pFormFillEnv ? m_pFormFillEnv->GetPlatform() : WideString();
}

void CPDFXFA_Context::Beep(uint32_t dwType) {
  if (m_pFormFillEnv)
    m_pFormFillEnv->JS_appBeep(dwType);
}

int32_t CPDFXFA_Context::MsgBox(const WideString& wsMessage,
                                const WideString& wsTitle,
                                uint32_t dwIconType,
                                uint32_t dwButtonType) {
  if (!m_pFormFillEnv || m_nLoadStatus != LoadStatus::kLoaded)
    return -1;

  int iconType =
      IsValidAlertIcon(dwIconType) ? dwIconType : JSPLATFORM_ALERT_ICON_DEFAULT;
  int iButtonType = IsValidAlertButton(dwButtonType)
                        ? dwButtonType
                        : JSPLATFORM_ALERT_BUTTON_DEFAULT;
  return m_pFormFillEnv->JS_appAlert(wsMessage, wsTitle, iButtonType, iconType);
}

WideString CPDFXFA_Context::Response(const WideString& wsQuestion,
                                     const WideString& wsTitle,
                                     const WideString& wsDefaultAnswer,
                                     bool bMark) {
  if (!m_pFormFillEnv)
    return WideString();

  constexpr int kMaxWideChars = 1024;
  FixedZeroedDataVector<uint16_t> buffer(kMaxWideChars);
  pdfium::span<uint16_t> buffer_span = buffer.writable_span();
  int byte_length = m_pFormFillEnv->JS_appResponse(
      wsQuestion, wsTitle, wsDefaultAnswer, WideString(), bMark,
      pdfium::as_writable_bytes(buffer_span));
  if (byte_length <= 0)
    return WideString();

  buffer_span = buffer_span.first(
      std::min<size_t>(kMaxWideChars, byte_length / sizeof(uint16_t)));
  return WideString::FromUTF16LE(buffer_span.data(), buffer_span.size());
}

RetainPtr<IFX_SeekableReadStream> CPDFXFA_Context::DownloadURL(
    const WideString& wsURL) {
  return m_pFormFillEnv ? m_pFormFillEnv->DownloadFromURL(wsURL) : nullptr;
}

bool CPDFXFA_Context::PostRequestURL(const WideString& wsURL,
                                     const WideString& wsData,
                                     const WideString& wsContentType,
                                     const WideString& wsEncode,
                                     const WideString& wsHeader,
                                     WideString& wsResponse) {
  if (!m_pFormFillEnv)
    return false;

  wsResponse = m_pFormFillEnv->PostRequestURL(wsURL, wsData, wsContentType,
                                              wsEncode, wsHeader);
  return true;
}

bool CPDFXFA_Context::PutRequestURL(const WideString& wsURL,
                                    const WideString& wsData,
                                    const WideString& wsEncode) {
  return m_pFormFillEnv &&
         m_pFormFillEnv->PutRequestURL(wsURL, wsData, wsEncode);
}

CFX_Timer::HandlerIface* CPDFXFA_Context::GetTimerHandler() const {
  return m_pFormFillEnv ? m_pFormFillEnv->GetTimerHandler() : nullptr;
}

cppgc::Heap* CPDFXFA_Context::GetGCHeap() const {
  return m_pGCHeap.get();
}

bool CPDFXFA_Context::SaveDatasetsPackage(
    const RetainPtr<IFX_SeekableStream>& pStream) {
  return SavePackage(pStream, XFA_HASHCODE_Datasets);
}

bool CPDFXFA_Context::SaveFormPackage(
    const RetainPtr<IFX_SeekableStream>& pStream) {
  return SavePackage(pStream, XFA_HASHCODE_Form);
}

bool CPDFXFA_Context::SavePackage(const RetainPtr<IFX_SeekableStream>& pStream,
                                  XFA_HashCode code) {
  CXFA_FFDocView* pXFADocView = GetXFADocView();
  if (!pXFADocView)
    return false;

  CXFA_FFDoc* ffdoc = pXFADocView->GetDoc();
  return ffdoc->SavePackage(ToNode(ffdoc->GetXFADoc()->GetXFAObject(code)),
                            pStream);
}

void CPDFXFA_Context::SendPostSaveToXFADoc() {
  if (!ContainsExtensionForm())
    return;

  CXFA_FFDocView* pXFADocView = GetXFADocView();
  if (!pXFADocView)
    return;

  CXFA_FFWidgetHandler* pWidgetHandler = pXFADocView->GetWidgetHandler();
  CXFA_ReadyNodeIterator it(pXFADocView->GetRootSubform());
  while (CXFA_Node* pNode = it.MoveToNext()) {
    CXFA_EventParam preParam;
    preParam.m_eType = XFA_EVENT_PostSave;
    pWidgetHandler->ProcessEvent(pNode, &preParam);
  }
  pXFADocView->UpdateDocView();
  ClearChangeMark();
}

void CPDFXFA_Context::SendPreSaveToXFADoc(
    std::vector<RetainPtr<IFX_SeekableStream>>* fileList) {
  if (!ContainsExtensionForm())
    return;

  CXFA_FFDocView* pXFADocView = GetXFADocView();
  if (!pXFADocView)
    return;

  CXFA_FFWidgetHandler* pWidgetHandler = pXFADocView->GetWidgetHandler();
  CXFA_ReadyNodeIterator it(pXFADocView->GetRootSubform());
  while (CXFA_Node* pNode = it.MoveToNext()) {
    CXFA_EventParam preParam;
    preParam.m_eType = XFA_EVENT_PreSave;
    pWidgetHandler->ProcessEvent(pNode, &preParam);
  }
  pXFADocView->UpdateDocView();
  return;
}
