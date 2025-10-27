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
#include "core/fxcrt/check.h"
#include "core/fxcrt/fixed_size_data_vector.h"
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
  if (!pRoot) {
    return nullptr;
  }

  RetainPtr<const CPDF_Dictionary> pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm) {
    return nullptr;
  }

  RetainPtr<const CPDF_Object> pElementXFA =
      pAcroForm->GetDirectObjectFor("XFA");
  if (!pElementXFA) {
    return nullptr;
  }

  std::vector<RetainPtr<const CPDF_Stream>> xfa_streams;
  if (pElementXFA->IsArray()) {
    const CPDF_Array* pXFAArray = pElementXFA->AsArray();
    for (size_t i = 0; i < pXFAArray->size() / 2; i++) {
      RetainPtr<const CPDF_Stream> pStream = pXFAArray->GetStreamAt(i * 2 + 1);
      if (pStream) {
        xfa_streams.push_back(std::move(pStream));
      }
    }
  } else if (pElementXFA->IsStream()) {
    xfa_streams.push_back(ToStream(pElementXFA));
  }
  if (xfa_streams.empty()) {
    return nullptr;
  }

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
    : pdfdoc_(pPDFDoc),
      doc_env_(std::make_unique<CPDFXFA_DocEnvironment>(this)),
      gc_heap_(FXGC_CreateHeap()) {
  DCHECK(pdfdoc_);

  // There might not be a heap when JS not initialized.
  if (gc_heap_) {
    xfaapp_ = cppgc::MakeGarbageCollected<CXFA_FFApp>(
        gc_heap_->GetAllocationHandle(), this);
  }
}

CPDFXFA_Context::~CPDFXFA_Context() {
  load_status_ = LoadStatus::kClosing;
  if (form_fill_env_) {
    form_fill_env_->ClearAllFocusedAnnots();
  }
}

void CPDFXFA_Context::SetFormFillEnv(
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  // The layout data can have pointers back into the script context. That
  // context will be different if the form fill environment closes, so, force
  // the layout data to clear.
  if (xfadoc_ && xfadoc_->GetXFADoc()) {
    xfadoc_->GetXFADoc()->ClearLayoutData();
    xfadoc_view_.Clear();
    xfadoc_.Clear();
    xfaapp_.Clear();
    FXGC_ForceGarbageCollection(gc_heap_.get());
  }
  form_fill_env_.Reset(pFormFillEnv);
}

bool CPDFXFA_Context::LoadXFADoc() {
  load_status_ = LoadStatus::kLoading;
  xfa_page_list_.clear();

  CJS_Runtime* actual_runtime = GetCJSRuntime();  // Null if a stub.
  if (!actual_runtime) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  auto stream = CreateXFAMultiStream(pdfdoc_);
  if (!stream) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  CFX_XMLParser parser(stream);
  xml_ = parser.Parse();
  if (!xml_) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  AutoNuller<cppgc::Persistent<CXFA_FFDoc>> doc_nuller(&xfadoc_);
  xfadoc_ = cppgc::MakeGarbageCollected<CXFA_FFDoc>(
      gc_heap_->GetAllocationHandle(), xfaapp_, doc_env_.get(), pdfdoc_,
      gc_heap_.get());

  if (!xfadoc_->OpenDoc(xml_.get())) {
    FXSYS_SetLastError(FPDF_ERR_XFALOAD);
    return false;
  }

  if (!xfaapp_->LoadFWLTheme(xfadoc_)) {
    FXSYS_SetLastError(FPDF_ERR_XFALAYOUT);
    return false;
  }

  xfadoc_->GetXFADoc()->InitScriptContext(actual_runtime);
  if (xfadoc_->GetFormType() == FormType::kXFAFull) {
    form_type_ = FormType::kXFAFull;
  } else {
    form_type_ = FormType::kXFAForeground;
  }

  AutoNuller<cppgc::Persistent<CXFA_FFDocView>> view_nuller(&xfadoc_view_);
  xfadoc_view_ = xfadoc_->CreateDocView();

  if (xfadoc_view_->StartLayout() < 0) {
    xfadoc_->GetXFADoc()->ClearLayoutData();
    FXGC_ForceGarbageCollection(gc_heap_.get());
    FXSYS_SetLastError(FPDF_ERR_XFALAYOUT);
    return false;
  }

  xfadoc_view_->DoLayout();
  xfadoc_view_->StopLayout();

  view_nuller.AbandonNullification();
  doc_nuller.AbandonNullification();
  load_status_ = LoadStatus::kLoaded;
  return true;
}

int CPDFXFA_Context::GetPageCount() const {
  switch (form_type_) {
    case FormType::kNone:
    case FormType::kAcroForm:
    case FormType::kXFAForeground:
      return pdfdoc_->GetPageCount();
    case FormType::kXFAFull:
      return xfadoc_ ? xfadoc_view_->CountPageViews() : 0;
  }
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetOrCreateXFAPage(int page_index) {
  if (page_index < 0) {
    return nullptr;
  }

  if (fxcrt::IndexInBounds(xfa_page_list_, page_index)) {
    if (xfa_page_list_[page_index]) {
      return xfa_page_list_[page_index];
    }
  } else {
    page_count_ = GetPageCount();
    xfa_page_list_.resize(page_count_);
  }

  auto pPage = pdfium::MakeRetain<CPDFXFA_Page>(GetPDFDoc(), page_index);
  if (!pPage->LoadPage()) {
    return nullptr;
  }

  if (fxcrt::IndexInBounds(xfa_page_list_, page_index)) {
    xfa_page_list_[page_index] = pPage;
  }

  return pPage;
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetXFAPage(int page_index) {
  if (!fxcrt::IndexInBounds(xfa_page_list_, page_index)) {
    return nullptr;
  }

  return xfa_page_list_[page_index];
}

RetainPtr<CPDFXFA_Page> CPDFXFA_Context::GetXFAPage(
    CXFA_FFPageView* pPage) const {
  if (!pPage) {
    return nullptr;
  }

  if (!xfadoc_) {
    return nullptr;
  }

  if (form_type_ != FormType::kXFAFull) {
    return nullptr;
  }

  for (auto& pTempPage : xfa_page_list_) {
    if (pTempPage && pTempPage->GetXFAPageView() == pPage) {
      return pTempPage;
    }
  }
  return nullptr;
}

uint32_t CPDFXFA_Context::DeletePage(int page_index) {
  // Delete from the document first because, if GetPage was never called for
  // this |page_index| then |xfa_page_list_| may have size < |page_index| even
  // if it's a valid page in the document.
  uint32_t page_obj_num = pdfdoc_->DeletePage(page_index);

  if (page_obj_num != 0) {
    --page_count_;
  }

  if (fxcrt::IndexInBounds(xfa_page_list_, page_index)) {
    xfa_page_list_.erase(std::next(xfa_page_list_.begin(), page_index));
    for (int i = page_index; i < fxcrt::CollectionSize<int>(xfa_page_list_);
         i++) {
      if (xfa_page_list_[i]) {
        xfa_page_list_[i]->SetXFAPageViewIndex(i);
      }
    }
  }

  return page_obj_num;
}

void CPDFXFA_Context::PagesInserted(int page_index, size_t num_pages) {
  if (fxcrt::IndexInBounds(xfa_page_list_, page_index)) {
    xfa_page_list_.insert(std::next(xfa_page_list_.begin(), page_index),
                          num_pages, nullptr);
    page_count_ += num_pages;
  }
}

bool CPDFXFA_Context::ContainsExtensionForm() const {
  return form_type_ == FormType::kXFAFull ||
         form_type_ == FormType::kXFAForeground;
}

bool CPDFXFA_Context::ContainsExtensionFullForm() const {
  return form_type_ == FormType::kXFAFull;
}

bool CPDFXFA_Context::ContainsExtensionForegroundForm() const {
  return form_type_ == FormType::kXFAForeground;
}

void CPDFXFA_Context::ClearChangeMark() {
  if (form_fill_env_) {
    form_fill_env_->ClearChangeMark();
  }
}

CJS_Runtime* CPDFXFA_Context::GetCJSRuntime() const {
  if (!form_fill_env_) {
    return nullptr;
  }

  return form_fill_env_->GetIJSRuntime()->AsCJSRuntime();
}

WideString CPDFXFA_Context::GetAppTitle() const {
  return WideString::FromASCII("PDFium");
}

WideString CPDFXFA_Context::GetAppName() {
  return form_fill_env_ ? form_fill_env_->FFI_GetAppName() : WideString();
}

WideString CPDFXFA_Context::GetLanguage() {
  return form_fill_env_ ? form_fill_env_->GetLanguage() : WideString();
}

WideString CPDFXFA_Context::GetPlatform() {
  return form_fill_env_ ? form_fill_env_->GetPlatform() : WideString();
}

void CPDFXFA_Context::Beep(uint32_t dwType) {
  if (form_fill_env_) {
    form_fill_env_->JS_appBeep(dwType);
  }
}

int32_t CPDFXFA_Context::MsgBox(const WideString& wsMessage,
                                const WideString& wsTitle,
                                uint32_t dwIconType,
                                uint32_t dwButtonType) {
  if (!form_fill_env_ || load_status_ != LoadStatus::kLoaded) {
    return -1;
  }

  int iconType =
      IsValidAlertIcon(dwIconType) ? dwIconType : JSPLATFORM_ALERT_ICON_DEFAULT;
  int iButtonType = IsValidAlertButton(dwButtonType)
                        ? dwButtonType
                        : JSPLATFORM_ALERT_BUTTON_DEFAULT;
  return form_fill_env_->JS_appAlert(wsMessage, wsTitle, iButtonType, iconType);
}

WideString CPDFXFA_Context::Response(const WideString& wsQuestion,
                                     const WideString& wsTitle,
                                     const WideString& wsDefaultAnswer,
                                     bool bMark) {
  if (!form_fill_env_) {
    return WideString();
  }

  static constexpr int kMaxWideChars = 1024;
  static constexpr int kMaxBytes = kMaxWideChars * sizeof(uint16_t);
  auto buffer = FixedSizeDataVector<uint8_t>::Zeroed(kMaxBytes);
  pdfium::span<uint8_t> buffer_span = buffer.span();
  int byte_length = form_fill_env_->JS_appResponse(
      wsQuestion, wsTitle, wsDefaultAnswer, WideString(), bMark, buffer_span);
  if (byte_length <= 0) {
    return WideString();
  }

  buffer_span = buffer_span.first(std::min<size_t>(kMaxBytes, byte_length));
  return WideString::FromUTF16LE(buffer_span);
}

RetainPtr<IFX_SeekableReadStream> CPDFXFA_Context::DownloadURL(
    const WideString& wsURL) {
  return form_fill_env_ ? form_fill_env_->DownloadFromURL(wsURL) : nullptr;
}

bool CPDFXFA_Context::PostRequestURL(const WideString& wsURL,
                                     const WideString& wsData,
                                     const WideString& wsContentType,
                                     const WideString& wsEncode,
                                     const WideString& wsHeader,
                                     WideString& wsResponse) {
  if (!form_fill_env_) {
    return false;
  }

  wsResponse = form_fill_env_->PostRequestURL(wsURL, wsData, wsContentType,
                                              wsEncode, wsHeader);
  return true;
}

bool CPDFXFA_Context::PutRequestURL(const WideString& wsURL,
                                    const WideString& wsData,
                                    const WideString& wsEncode) {
  return form_fill_env_ &&
         form_fill_env_->PutRequestURL(wsURL, wsData, wsEncode);
}

CFX_Timer::HandlerIface* CPDFXFA_Context::GetTimerHandler() const {
  return form_fill_env_ ? form_fill_env_->GetTimerHandler() : nullptr;
}

cppgc::Heap* CPDFXFA_Context::GetGCHeap() const {
  return gc_heap_.get();
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
  if (!pXFADocView) {
    return false;
  }

  CXFA_FFDoc* ffdoc = pXFADocView->GetDoc();
  return ffdoc->SavePackage(ToNode(ffdoc->GetXFADoc()->GetXFAObject(code)),
                            pStream);
}

void CPDFXFA_Context::SendPostSaveToXFADoc() {
  if (!ContainsExtensionForm()) {
    return;
  }

  CXFA_FFDocView* pXFADocView = GetXFADocView();
  if (!pXFADocView) {
    return;
  }

  CXFA_FFWidgetHandler* pWidgetHandler = pXFADocView->GetWidgetHandler();
  CXFA_ReadyNodeIterator it(pXFADocView->GetRootSubform());
  while (CXFA_Node* pNode = it.MoveToNext()) {
    CXFA_EventParam preParam(XFA_EVENT_PostSave);
    preParam.targeted_ = false;
    pWidgetHandler->ProcessEvent(pNode, &preParam);
  }
  pXFADocView->UpdateDocView();
  ClearChangeMark();
}

void CPDFXFA_Context::SendPreSaveToXFADoc(
    std::vector<RetainPtr<IFX_SeekableStream>>* fileList) {
  if (!ContainsExtensionForm()) {
    return;
  }

  CXFA_FFDocView* pXFADocView = GetXFADocView();
  if (!pXFADocView) {
    return;
  }

  CXFA_FFWidgetHandler* pWidgetHandler = pXFADocView->GetWidgetHandler();
  CXFA_ReadyNodeIterator it(pXFADocView->GetRootSubform());
  while (CXFA_Node* pNode = it.MoveToNext()) {
    CXFA_EventParam preParam(XFA_EVENT_PreSave);
    preParam.targeted_ = false;
    pWidgetHandler->ProcessEvent(pNode, &preParam);
  }
  pXFADocView->UpdateDocView();
  return;
}
