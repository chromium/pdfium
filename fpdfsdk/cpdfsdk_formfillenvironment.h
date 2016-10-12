// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
#define FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_

#include <memory>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_occontext.h"
#include "core/fxcrt/cfx_observable.h"
#include "fpdfsdk/cfx_systemhandler.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/fsdk_define.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"

class CFFL_InteractiveFormFiller;
class CFX_SystemHandler;
class CPDFSDK_ActionHandler;
class CPDFSDK_AnnotHandlerMgr;
class CPDFSDK_InterForm;
class CPDFSDK_PageView;
class IJS_Runtime;

class CPDFSDK_FormFillEnvironment
    : public CFX_Observable<CPDFSDK_FormFillEnvironment> {
 public:
  CPDFSDK_FormFillEnvironment(UnderlyingDocumentType* pDoc,
                              FPDF_FORMFILLINFO* pFFinfo);
  ~CPDFSDK_FormFillEnvironment();

  CPDFSDK_InterForm* GetInterForm();

  // Gets the document object for the next layer down; for master this is
  // a CPDF_Document, but for XFA it is a CPDFXFA_Document.
  UnderlyingDocumentType* GetUnderlyingDocument() const {
#ifdef PDF_ENABLE_XFA
    return GetXFADocument();
#else   // PDF_ENABLE_XFA
    return GetPDFDocument();
#endif  // PDF_ENABLE_XFA
  }

  // Gets the CPDF_Document, either directly in master, or from the
  // CPDFXFA_Document for XFA.
  CPDF_Document* GetPDFDocument() const {
#ifdef PDF_ENABLE_XFA
    return m_pUnderlyingDoc ? m_pUnderlyingDoc->GetPDFDoc() : nullptr;
#else   // PDF_ENABLE_XFA
    return m_pUnderlyingDoc;
#endif  // PDF_ENABLE_XFA
  }

#ifdef PDF_ENABLE_XFA
  // Gets the XFA document directly (XFA-only).
  CPDFXFA_Document* GetXFADocument() const { return m_pUnderlyingDoc; }
  void ResetXFADocument() { m_pUnderlyingDoc = nullptr; }

  int GetPageViewCount() const { return m_pageMap.size(); }
#endif  // PDF_ENABLE_XFA

  CPDFSDK_PageView* GetPageView(UnderlyingPageType* pPage, bool ReNew);
  CPDFSDK_PageView* GetPageView(int nIndex);
  CPDFSDK_PageView* GetCurrentView();
  void RemovePageView(UnderlyingPageType* pPage);
  void UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot);

  CPDFSDK_Annot* GetFocusAnnot() { return m_pFocusAnnot.Get(); }
  FX_BOOL SetFocusAnnot(CPDFSDK_Annot::ObservedPtr* pAnnot);
  FX_BOOL KillFocusAnnot(uint32_t nFlag);
  void ClearAllFocusedAnnots();

  FX_BOOL ExtractPages(const std::vector<uint16_t>& arrExtraPages,
                       CPDF_Document* pDstDoc);
  FX_BOOL InsertPages(int nInsertAt,
                      const CPDF_Document* pSrcDoc,
                      const std::vector<uint16_t>& arrSrcPages);
  FX_BOOL ReplacePages(int nPage,
                       const CPDF_Document* pSrcDoc,
                       const std::vector<uint16_t>& arrSrcPages);

  int GetPageCount() { return m_pUnderlyingDoc->GetPageCount(); }
  FX_BOOL GetPermissions(int nFlag);

  FX_BOOL GetChangeMark() { return m_bChangeMask; }
  void SetChangeMark() { m_bChangeMask = TRUE; }
  void ClearChangeMark() { m_bChangeMask = FALSE; }

  UnderlyingPageType* GetPage(int nIndex);

  void ProcJavascriptFun();
  FX_BOOL ProcOpenAction();

  void Invalidate(FPDF_PAGE page,
                  double left,
                  double top,
                  double right,
                  double bottom);
  void OutputSelectedRect(FPDF_PAGE page,
                          double left,
                          double top,
                          double right,
                          double bottom);

  void SetCursor(int nCursorType);
  int SetTimer(int uElapse, TimerCallback lpTimerFunc);
  void KillTimer(int nTimerID);
  FX_SYSTEMTIME GetLocalTime() const;

  void OnChange();
  FX_BOOL IsSHIFTKeyDown(uint32_t nFlag) const;
  FX_BOOL IsCTRLKeyDown(uint32_t nFlag) const;
  FX_BOOL IsALTKeyDown(uint32_t nFlag) const;

  FPDF_PAGE GetPage(FPDF_DOCUMENT document, int nPageIndex);
  FPDF_PAGE GetCurrentPage(FPDF_DOCUMENT document);

  void ExecuteNamedAction(const FX_CHAR* namedAction);
  void OnSetFieldInputFocus(FPDF_WIDESTRING focusText,
                            FPDF_DWORD nTextLen,
                            FX_BOOL bFocus);
  void DoURIAction(const FX_CHAR* bsURI);
  void DoGoToAction(int nPageIndex,
                    int zoomMode,
                    float* fPosArray,
                    int sizeOfArray);

#ifdef PDF_ENABLE_XFA
  void DisplayCaret(FPDF_PAGE page,
                    FPDF_BOOL bVisible,
                    double left,
                    double top,
                    double right,
                    double bottom);
  int GetCurrentPageIndex(FPDF_DOCUMENT document);
  void SetCurrentPage(FPDF_DOCUMENT document, int iCurPage);

  // TODO(dsinclair): This should probably change to PDFium?
  CFX_WideString FFI_GetAppName() const { return CFX_WideString(L"Acrobat"); }

  CFX_WideString GetPlatform();
  void GotoURL(FPDF_DOCUMENT document, const CFX_WideStringC& wsURL);
  void GetPageViewRect(FPDF_PAGE page, FS_RECTF& dstRect);
  FX_BOOL PopupMenu(FPDF_PAGE page,
                    FPDF_WIDGET hWidget,
                    int menuFlag,
                    CFX_PointF pt);

  void Alert(FPDF_WIDESTRING Msg, FPDF_WIDESTRING Title, int Type, int Icon);
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
  IFX_FileRead* DownloadFromURL(const FX_WCHAR* url);
  CFX_WideString PostRequestURL(const FX_WCHAR* wsURL,
                                const FX_WCHAR* wsData,
                                const FX_WCHAR* wsContentType,
                                const FX_WCHAR* wsEncode,
                                const FX_WCHAR* wsHeader);
  FPDF_BOOL PutRequestURL(const FX_WCHAR* wsURL,
                          const FX_WCHAR* wsData,
                          const FX_WCHAR* wsEncode);
  CFX_WideString GetLanguage();

  void PageEvent(int iPageCount, uint32_t dwEventType) const;
#endif  // PDF_ENABLE_XFA

  int JS_appAlert(const FX_WCHAR* Msg,
                  const FX_WCHAR* Title,
                  uint32_t Type,
                  uint32_t Icon);
  int JS_appResponse(const FX_WCHAR* Question,
                     const FX_WCHAR* Title,
                     const FX_WCHAR* Default,
                     const FX_WCHAR* cLabel,
                     FPDF_BOOL bPassword,
                     void* response,
                     int length);
  void JS_appBeep(int nType);
  CFX_WideString JS_fieldBrowse();
  CFX_WideString JS_docGetFilePath();
  void JS_docSubmitForm(void* formData, int length, const FX_WCHAR* URL);
  void JS_docmailForm(void* mailData,
                      int length,
                      FPDF_BOOL bUI,
                      const FX_WCHAR* To,
                      const FX_WCHAR* Subject,
                      const FX_WCHAR* CC,
                      const FX_WCHAR* BCC,
                      const FX_WCHAR* Msg);
  void JS_docprint(FPDF_BOOL bUI,
                   int nStart,
                   int nEnd,
                   FPDF_BOOL bSilent,
                   FPDF_BOOL bShrinkToFit,
                   FPDF_BOOL bPrintAsImage,
                   FPDF_BOOL bReverse,
                   FPDF_BOOL bAnnotations);
  void JS_docgotoPage(int nPageNum);

  FX_BOOL IsJSInitiated() const { return m_pInfo && m_pInfo->m_pJsPlatform; }
  CFX_ByteString GetAppName() const { return ""; }
  CFX_SystemHandler* GetSysHandler() const { return m_pSysHandler.get(); }
  FPDF_FORMFILLINFO* GetFormFillInfo() const { return m_pInfo; }

  // Creates if not present.
  CFFL_InteractiveFormFiller* GetInteractiveFormFiller();
  CPDFSDK_AnnotHandlerMgr* GetAnnotHandlerMgr();  // Creates if not present.
  IJS_Runtime* GetJSRuntime();                    // Creates if not present.
  CPDFSDK_ActionHandler* GetActionHander();       // Creates if not present.

 private:
  std::unique_ptr<CPDFSDK_AnnotHandlerMgr> m_pAnnotHandlerMgr;
  std::unique_ptr<CPDFSDK_ActionHandler> m_pActionHandler;
  std::unique_ptr<IJS_Runtime> m_pJSRuntime;
  FPDF_FORMFILLINFO* const m_pInfo;
  std::map<UnderlyingPageType*, CPDFSDK_PageView*> m_pageMap;
  std::unique_ptr<CPDFSDK_InterForm> m_pInterForm;
  CPDFSDK_Annot::ObservedPtr m_pFocusAnnot;
  UnderlyingDocumentType* m_pUnderlyingDoc;
  std::unique_ptr<CFFL_InteractiveFormFiller> m_pFormFiller;
  std::unique_ptr<CFX_SystemHandler> m_pSysHandler;
  FX_BOOL m_bChangeMask;
  FX_BOOL m_bBeingDestroyed;
};

#endif  // FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
