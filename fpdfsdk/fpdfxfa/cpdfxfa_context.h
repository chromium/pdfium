// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CPDFXFA_CONTEXT_H_
#define FPDFSDK_FPDFXFA_CPDFXFA_CONTEXT_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

class CFX_XMLDocument;
class CJS_Runtime;
class CPDFXFA_DocEnvironment;

// Per-process initializations.
void CPDFXFA_ModuleInit();
void CPDFXFA_ModuleDestroy();

class CPDFXFA_Context final : public CPDF_Document::Extension,
                              public CXFA_FFApp::CallbackIface {
 public:
  enum class LoadStatus : uint8_t {
    kPreload = 0,
    kLoading,
    kLoaded,
    kClosing,
  };

  explicit CPDFXFA_Context(CPDF_Document* pPDFDoc);
  ~CPDFXFA_Context() override;

  bool LoadXFADoc();
  LoadStatus GetLoadStatus() const { return m_nLoadStatus; }
  FormType GetFormType() const { return m_FormType; }
  int GetOriginalPageCount() const { return m_nPageCount; }
  void SetOriginalPageCount(int count) {
    m_nPageCount = count;
    m_XFAPageList.resize(count);
  }

  CPDF_Document* GetPDFDoc() const { return m_pPDFDoc; }
  CFX_XMLDocument* GetXMLDoc() { return m_pXML.get(); }
  CXFA_FFDoc* GetXFADoc() { return m_pXFADoc; }
  CXFA_FFDocView* GetXFADocView() const { return m_pXFADocView.Get(); }
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const {
    return m_pFormFillEnv.Get();
  }
  void SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  RetainPtr<CPDFXFA_Page> GetOrCreateXFAPage(int page_index);
  RetainPtr<CPDFXFA_Page> GetXFAPage(int page_index);
  RetainPtr<CPDFXFA_Page> GetXFAPage(CXFA_FFPageView* pPage) const;
  void ClearChangeMark();

  // CPDF_Document::Extension:
  int GetPageCount() const override;
  void DeletePage(int page_index) override;
  uint32_t GetUserPermissions() const override;
  bool ContainsExtensionForm() const override;
  bool ContainsExtensionFullForm() const override;
  bool ContainsExtensionForegroundForm() const override;

  // CXFA_FFApp::CallbackIface:
  WideString GetLanguage() override;
  WideString GetPlatform() override;
  WideString GetAppName() override;
  WideString GetAppTitle() const override;
  void Beep(uint32_t dwType) override;
  int32_t MsgBox(const WideString& wsMessage,
                 const WideString& wsTitle,
                 uint32_t dwIconType,
                 uint32_t dwButtonType) override;
  WideString Response(const WideString& wsQuestion,
                      const WideString& wsTitle,
                      const WideString& wsDefaultAnswer,
                      bool bMark) override;
  RetainPtr<IFX_SeekableReadStream> DownloadURL(
      const WideString& wsURL) override;
  bool PostRequestURL(const WideString& wsURL,
                      const WideString& wsData,
                      const WideString& wsContentType,
                      const WideString& wsEncode,
                      const WideString& wsHeader,
                      WideString& wsResponse) override;
  bool PutRequestURL(const WideString& wsURL,
                     const WideString& wsData,
                     const WideString& wsEncode) override;
  CFX_Timer::HandlerIface* GetTimerHandler() const override;
  cppgc::Heap* GetGCHeap() const override;

  bool SaveDatasetsPackage(const RetainPtr<IFX_SeekableStream>& pStream);
  bool SaveFormPackage(const RetainPtr<IFX_SeekableStream>& pStream);
  void SendPostSaveToXFADoc();
  void SendPreSaveToXFADoc(
      std::vector<RetainPtr<IFX_SeekableStream>>* fileList);

 private:
  CJS_Runtime* GetCJSRuntime() const;
  bool SavePackage(const RetainPtr<IFX_SeekableStream>& pStream,
                   XFA_HashCode code);

  FormType m_FormType = FormType::kNone;
  LoadStatus m_nLoadStatus = LoadStatus::kPreload;
  int m_nPageCount = 0;

  // The order in which the following members are destroyed is critical.
  UnownedPtr<CPDF_Document> const m_pPDFDoc;
  std::unique_ptr<CFX_XMLDocument> m_pXML;
  ObservedPtr<CPDFSDK_FormFillEnvironment> m_pFormFillEnv;
  std::vector<RetainPtr<CPDFXFA_Page>> m_XFAPageList;

  // Can't outlive |m_pFormFillEnv|.
  std::unique_ptr<CPDFXFA_DocEnvironment> m_pDocEnv;

  FXGCScopedHeap m_pGCHeap;
  cppgc::Persistent<CXFA_FFApp> m_pXFAApp;          // can't outlive |m_pGCHeap|
  cppgc::Persistent<CXFA_FFDoc> m_pXFADoc;          // Can't outlive |m_pGCHeap|
  cppgc::Persistent<CXFA_FFDocView> m_pXFADocView;  // Can't outlive |m_pGCHeap|
};

#endif  // FPDFSDK_FPDFXFA_CPDFXFA_CONTEXT_H_
