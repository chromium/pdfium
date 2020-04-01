// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
#define FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/timerhandler_iface.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"
#include "public/fpdf_formfill.h"

class CFFL_InteractiveFormFiller;
class CPDFSDK_ActionHandler;
class CPDFSDK_AnnotHandlerMgr;
class CPDFSDK_InteractiveForm;
class CPDFSDK_PageView;
class IJS_Runtime;

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

class CPDFSDK_FormFillEnvironment final : public Observable,
                                          public TimerHandlerIface,
                                          public IPWL_SystemHandler {
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
  void OutputSelectedRect(CFFL_FormFiller* pFormFiller,
                          const CFX_FloatRect& rect) override;
  bool IsSelectionImplemented() const override;
  void SetCursor(int32_t nCursorType) override;

  CPDFSDK_PageView* GetPageView(IPDF_Page* pUnderlyingPage, bool renew);
  CPDFSDK_PageView* GetPageViewAtIndex(int nIndex);

  void RemovePageView(IPDF_Page* pUnderlyingPage);
  void UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot);

  CPDFSDK_Annot* GetFocusAnnot() const { return m_pFocusAnnot.Get(); }
  bool SetFocusAnnot(ObservedPtr<CPDFSDK_Annot>* pAnnot);
  bool KillFocusAnnot(uint32_t nFlag);
  void ClearAllFocusedAnnots();

  int GetPageCount() const;

  // See PDF Reference 1.7, table 3.20 for the permission bits. Returns true if
  // any bit in |flags| is set.
  bool HasPermissions(uint32_t flags) const;

  bool GetChangeMark() const { return m_bChangeMask; }
  void SetChangeMark() { m_bChangeMask = true; }
  void ClearChangeMark() { m_bChangeMask = false; }

  void ProcJavascriptAction();
  bool ProcOpenAction();
  void Invalidate(IPDF_Page* page, const FX_RECT& rect);

  void OnChange();
  void ExecuteNamedAction(const char* namedAction);
  void OnSetFieldInputFocus(FPDF_WIDESTRING focusText,
                            FPDF_DWORD nTextLen,
                            bool bFocus);
  void DoURIAction(const char* bsURI);
  void DoGoToAction(int nPageIndex,
                    int zoomMode,
                    float* fPosArray,
                    int sizeOfArray);

  CPDF_Document* GetPDFDocument() const { return m_pCPDFDoc.Get(); }
  CPDF_Document::Extension* GetDocExtension() const {
    return m_pCPDFDoc->GetExtension();
  }

  bool IsJSPlatformPresent() const { return m_pInfo && m_pInfo->m_pJsPlatform; }

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
                     void* response,
                     int length);
  void JS_appBeep(int nType);
  WideString JS_fieldBrowse();
  void JS_docmailForm(void* mailData,
                      int length,
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
  bool PopupMenu(IPDF_Page* page,
                 int menuFlag,
                 const CFX_PointF& pt);
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
  TimerHandlerIface* GetTimerHandler() { return this; }
  IPWL_SystemHandler* GetSysHandler() { return this; }
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

  // Creates if not present.
  CFFL_InteractiveFormFiller* GetInteractiveFormFiller();
  IJS_Runtime* GetIJSRuntime();                   // Creates if not present.
  CPDFSDK_ActionHandler* GetActionHandler();      // Creates if not present.
  CPDFSDK_InteractiveForm* GetInteractiveForm();  // Creates if not present.

 private:
  IPDF_Page* GetPage(int nIndex);
  void SendOnFocusChange(ObservedPtr<CPDFSDK_Annot>* pAnnot);

  FPDF_FORMFILLINFO* const m_pInfo;
  std::unique_ptr<CPDFSDK_ActionHandler> m_pActionHandler;
  std::unique_ptr<IJS_Runtime> m_pIJSRuntime;
  std::map<IPDF_Page*, std::unique_ptr<CPDFSDK_PageView>> m_PageMap;
  std::unique_ptr<CPDFSDK_InteractiveForm> m_pInteractiveForm;
  ObservedPtr<CPDFSDK_Annot> m_pFocusAnnot;
  UnownedPtr<CPDF_Document> const m_pCPDFDoc;
  std::unique_ptr<CPDFSDK_AnnotHandlerMgr> m_pAnnotHandlerMgr;
  std::unique_ptr<CFFL_InteractiveFormFiller> m_pFormFiller;
  bool m_bChangeMask = false;
  bool m_bBeingDestroyed = false;

  // Holds the list of focusable annot types.
  // Annotations of type WIDGET are by default focusable.
  std::vector<CPDF_Annot::Subtype> m_FocusableAnnotTypes = {
      CPDF_Annot::Subtype::WIDGET};
};

#endif  // FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
