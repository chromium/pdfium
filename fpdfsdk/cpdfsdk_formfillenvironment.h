// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
#define FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"
#include "public/fpdf_formfill.h"
#include "third_party/base/span.h"

class CPDFSDK_ActionHandler;
class CPDFSDK_AnnotHandlerMgr;
class CPDFSDK_InteractiveForm;
class CPDFSDK_PageView;
class IJS_Runtime;
class IPDF_Page;

// NOTE: |bsUTF16LE| must outlive the use of the result. Care must be taken
// since modifying the result would impact |bsUTF16LE|.
FPDF_WIDESTRING AsFPDFWideString(ByteString* bsUTF16LE);

// The CPDFSDK_FormFillEnvironment is "owned" by the embedder across the
// C API as a FPDF_FormHandle, and may pop out of existence at any time,
// so long as the associated embedder-owned FPDF_Document outlives it.
// Pointers from objects in the FPDF_Document ownership hierarchy should
// be ObservedPtr<> so as to clear themselves when the embedder "exits"
// the form fill environment.  Pointers from objects in this ownership
// heirarcy to objects in the FPDF_Document ownership hierarcy should be
// UnownedPtr<>, as should pointers from objects in this ownership
// hierarcy back to the form fill environment itself, so as to flag any
// lingering lifetime issues via the memory tools.

class CPDFSDK_FormFillEnvironment final
    : public CFX_Timer::HandlerIface,
      public IPWL_SystemHandler,
      public CFFL_InteractiveFormFiller::CallbackIface {
 public:
  CPDFSDK_FormFillEnvironment(
      CPDF_Document* pDoc,
      FPDF_FORMFILLINFO* pFFinfo,
      std::unique_ptr<CPDFSDK_AnnotHandlerMgr> pHandlerMgr);

  ~CPDFSDK_FormFillEnvironment() override;

  // TimerHandlerIface:
  int32_t SetTimer(int32_t uElapse, TimerCallback lpTimerFunc) override;
  void KillTimer(int32_t nTimerID) override;

  // IPWL_SystemHandler:
  void InvalidateRect(PerWindowData* pWidgetData,
                      const CFX_FloatRect& rect) override;
  void OutputSelectedRect(PerWindowData* pWidgetData,
                          const CFX_FloatRect& rect) override;
  bool IsSelectionImplemented() const override;
  void SetCursor(CursorStyle nCursorType) override;

  // CFFL_InteractiveFormFiller::CallbackIface:
  void OnSetFieldInputFocus(const WideString& text) override;
  void OnCalculate(ObservedPtr<CPDFSDK_Annot>& pAnnot) override;
  void OnFormat(ObservedPtr<CPDFSDK_Annot>& pAnnot) override;
  void Invalidate(IPDF_Page* page, const FX_RECT& rect) override;
  CPDFSDK_PageView* GetOrCreatePageView(IPDF_Page* pUnderlyingPage) override;
  CPDFSDK_PageView* GetPageView(IPDF_Page* pUnderlyingPage) override;
  CFX_Timer::HandlerIface* GetTimerHandler() override;
  IPWL_SystemHandler* GetSysHandler() override;
  CPDFSDK_Annot* GetFocusAnnot() const override;
  bool SetFocusAnnot(ObservedPtr<CPDFSDK_Annot>& pAnnot) override;
  bool HasPermissions(uint32_t flags) const override;
  void OnChange() override;

  CPDFSDK_PageView* GetPageViewAtIndex(int nIndex);
  void RemovePageView(IPDF_Page* pUnderlyingPage);
  void UpdateAllViews(CPDFSDK_Annot* pAnnot);

  bool KillFocusAnnot(Mask<FWL_EVENTFLAG> nFlag);
  void ClearAllFocusedAnnots();

  int GetPageCount() const;

  bool GetChangeMark() const { return m_bChangeMask; }
  void SetChangeMark() { m_bChangeMask = true; }
  void ClearChangeMark() { m_bChangeMask = false; }

  void ProcJavascriptAction();
  bool ProcOpenAction();

  void ExecuteNamedAction(const ByteString& namedAction);
  void DoURIAction(const ByteString& bsURI, Mask<FWL_EVENTFLAG> modifiers);
  void DoGoToAction(int nPageIndex,
                    int zoomMode,
                    pdfium::span<float> fPosArray);

  CPDF_Document* GetPDFDocument() const { return m_pCPDFDoc.Get(); }
  CPDF_Document::Extension* GetDocExtension() const {
    return m_pCPDFDoc->GetExtension();
  }

  bool IsJSPlatformPresent() const { return m_pInfo && m_pInfo->m_pJsPlatform; }
  IPDF_JSPLATFORM* GetJSPlatform() const {
    return m_pInfo ? m_pInfo->m_pJsPlatform : nullptr;
  }

#ifdef PDF_ENABLE_V8
  CPDFSDK_PageView* GetCurrentView();
  FPDF_PAGE GetCurrentPage() const;

  WideString GetLanguage();
  WideString GetPlatform();

  int JS_appAlert(const WideString& Msg,
                  const WideString& Title,
                  int Type,
                  int Icon);
  int JS_appResponse(const WideString& Question,
                     const WideString& Title,
                     const WideString& Default,
                     const WideString& cLabel,
                     FPDF_BOOL bPassword,
                     pdfium::span<uint8_t> response);
  void JS_appBeep(int nType);
  WideString JS_fieldBrowse();
  void JS_docmailForm(pdfium::span<uint8_t> mailData,
                      FPDF_BOOL bUI,
                      const WideString& To,
                      const WideString& Subject,
                      const WideString& CC,
                      const WideString& BCC,
                      const WideString& Msg);
  void JS_docprint(FPDF_BOOL bUI,
                   int nStart,
                   int nEnd,
                   FPDF_BOOL bSilent,
                   FPDF_BOOL bShrinkToFit,
                   FPDF_BOOL bPrintAsImage,
                   FPDF_BOOL bReverse,
                   FPDF_BOOL bAnnotations);
  void JS_docgotoPage(int nPageNum);
  WideString JS_docGetFilePath();

#ifdef PDF_ENABLE_XFA
  int GetPageViewCount() const;
  void DisplayCaret(IPDF_Page* page,
                    FPDF_BOOL bVisible,
                    double left,
                    double top,
                    double right,
                    double bottom);
  int GetCurrentPageIndex() const;
  void SetCurrentPage(int iCurPage);

  // TODO(dsinclair): This should probably change to PDFium?
  WideString FFI_GetAppName() const { return WideString(L"Acrobat"); }

  void GotoURL(const WideString& wsURL);
  FS_RECTF GetPageViewRect(IPDF_Page* page);
  bool PopupMenu(IPDF_Page* page, int menuFlag, const CFX_PointF& pt);
  void EmailTo(FPDF_FILEHANDLER* fileHandler,
               FPDF_WIDESTRING pTo,
               FPDF_WIDESTRING pSubject,
               FPDF_WIDESTRING pCC,
               FPDF_WIDESTRING pBcc,
               FPDF_WIDESTRING pMsg);
  void UploadTo(FPDF_FILEHANDLER* fileHandler,
                int fileFlag,
                FPDF_WIDESTRING uploadTo);
  FPDF_FILEHANDLER* OpenFile(int fileType,
                             FPDF_WIDESTRING wsURL,
                             const char* mode);
  RetainPtr<IFX_SeekableReadStream> DownloadFromURL(const WideString& url);
  WideString PostRequestURL(const WideString& wsURL,
                            const WideString& wsData,
                            const WideString& wsContentType,
                            const WideString& wsEncode,
                            const WideString& wsHeader);
  FPDF_BOOL PutRequestURL(const WideString& wsURL,
                          const WideString& wsData,
                          const WideString& wsEncode);

  void PageEvent(int iPageCount, uint32_t dwEventType) const;
#endif  // PDF_ENABLE_XFA
#endif  // PDF_ENABLE_V8

  WideString GetFilePath() const;
  ByteString GetAppName() const { return ByteString(); }
  FPDF_FORMFILLINFO* GetFormFillInfo() const { return m_pInfo; }
  void SubmitForm(pdfium::span<uint8_t> form_data, const WideString& URL);

  CPDFSDK_AnnotHandlerMgr* GetAnnotHandlerMgr();  // Always present.

  void SetFocusableAnnotSubtypes(
      const std::vector<CPDF_Annot::Subtype>& focusableAnnotTypes) {
    m_FocusableAnnotTypes = focusableAnnotTypes;
  }
  const std::vector<CPDF_Annot::Subtype>& GetFocusableAnnotSubtypes() const {
    return m_FocusableAnnotTypes;
  }

  // Never returns null.
  CFFL_InteractiveFormFiller* GetInteractiveFormFiller() {
    return m_pInteractiveFormFiller.get();
  }

  IJS_Runtime* GetIJSRuntime();                   // Creates if not present.
  CPDFSDK_ActionHandler* GetActionHandler();      // Creates if not present.
  CPDFSDK_InteractiveForm* GetInteractiveForm();  // Creates if not present.

 private:
  IPDF_Page* GetPage(int nIndex) const;
  void OnSetFieldInputFocusInternal(const WideString& text, bool bFocus);
  void SendOnFocusChange(ObservedPtr<CPDFSDK_Annot>& pAnnot);

  UnownedPtr<FPDF_FORMFILLINFO> const m_pInfo;
  std::unique_ptr<CPDFSDK_ActionHandler> m_pActionHandler;
  std::unique_ptr<IJS_Runtime> m_pIJSRuntime;
  std::map<IPDF_Page*, std::unique_ptr<CPDFSDK_PageView>> m_PageMap;
  std::unique_ptr<CPDFSDK_InteractiveForm> m_pInteractiveForm;
  ObservedPtr<CPDFSDK_Annot> m_pFocusAnnot;
  UnownedPtr<CPDF_Document> const m_pCPDFDoc;
  std::unique_ptr<CPDFSDK_AnnotHandlerMgr> m_pAnnotHandlerMgr;
  std::unique_ptr<CFFL_InteractiveFormFiller> m_pInteractiveFormFiller;
  bool m_bChangeMask = false;
  bool m_bBeingDestroyed = false;

  // Holds the list of focusable annot types.
  // Annotations of type WIDGET are by default focusable.
  std::vector<CPDF_Annot::Subtype> m_FocusableAnnotTypes = {
      CPDF_Annot::Subtype::WIDGET};
};

#endif  // FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
