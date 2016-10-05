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
#include "fpdfsdk/fsdk_define.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"

class CFFL_InteractiveFormFiller;
class CFX_SystemHandler;
class CPDFSDK_ActionHandler;
class CPDFSDK_AnnotHandlerMgr;
class CPDFSDK_Document;
class IJS_Runtime;

class CPDFSDK_FormFillEnvironment final {
 public:
  CPDFSDK_FormFillEnvironment(UnderlyingDocumentType* pDoc,
                              FPDF_FORMFILLINFO* pFFinfo);
  ~CPDFSDK_FormFillEnvironment();

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
  CPDFSDK_Document* GetSDKDocument() const { return m_pSDKDoc.get(); }
  UnderlyingDocumentType* GetUnderlyingDocument() const {
    return m_pUnderlyingDoc;
  }
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
  std::unique_ptr<CPDFSDK_Document> m_pSDKDoc;
  UnderlyingDocumentType* const m_pUnderlyingDoc;
  std::unique_ptr<CFFL_InteractiveFormFiller> m_pFormFiller;
  std::unique_ptr<CFX_SystemHandler> m_pSysHandler;
};

#endif  // FPDFSDK_CPDFSDK_FORMFILLENVIRONMENT_H_
