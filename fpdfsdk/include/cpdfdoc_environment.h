// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFDOC_ENVIRONMENT_H_
#define FPDFSDK_INCLUDE_CPDFDOC_ENVIRONMENT_H_

#include <memory>

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfdoc/include/cpdf_occontext.h"
#include "core/fxcrt/include/cfx_observable.h"
#include "fpdfsdk/cfx_systemhandler.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"

class CFFL_IFormFiller;
class CFX_SystemHandler;
class CPDFSDK_ActionHandler;
class CPDFSDK_AnnotHandlerMgr;
class CPDFSDK_Document;
class IJS_Runtime;

class CPDFDoc_Environment final {
 public:
  CPDFDoc_Environment(UnderlyingDocumentType* pDoc, FPDF_FORMFILLINFO* pFFinfo);
  ~CPDFDoc_Environment();

  void FFI_Invalidate(FPDF_PAGE page,
                      double left,
                      double top,
                      double right,
                      double bottom) {
    if (m_pInfo && m_pInfo->FFI_Invalidate)
      m_pInfo->FFI_Invalidate(m_pInfo, page, left, top, right, bottom);
  }

  void FFI_OutputSelectedRect(FPDF_PAGE page,
                              double left,
                              double top,
                              double right,
                              double bottom) {
    if (m_pInfo && m_pInfo->FFI_OutputSelectedRect)
      m_pInfo->FFI_OutputSelectedRect(m_pInfo, page, left, top, right, bottom);
  }

  void FFI_SetCursor(int nCursorType) {
    if (m_pInfo && m_pInfo->FFI_SetCursor)
      m_pInfo->FFI_SetCursor(m_pInfo, nCursorType);
  }

  int FFI_SetTimer(int uElapse, TimerCallback lpTimerFunc) {
    if (m_pInfo && m_pInfo->FFI_SetTimer)
      return m_pInfo->FFI_SetTimer(m_pInfo, uElapse, lpTimerFunc);
    return -1;
  }

  void FFI_KillTimer(int nTimerID) {
    if (m_pInfo && m_pInfo->FFI_KillTimer)
      m_pInfo->FFI_KillTimer(m_pInfo, nTimerID);
  }

  FX_SYSTEMTIME FFI_GetLocalTime() const {
    FX_SYSTEMTIME fxtime;
    if (m_pInfo && m_pInfo->FFI_GetLocalTime) {
      FPDF_SYSTEMTIME systime = m_pInfo->FFI_GetLocalTime(m_pInfo);
      fxtime.wDay = systime.wDay;
      fxtime.wDayOfWeek = systime.wDayOfWeek;
      fxtime.wHour = systime.wHour;
      fxtime.wMilliseconds = systime.wMilliseconds;
      fxtime.wMinute = systime.wMinute;
      fxtime.wMonth = systime.wMonth;
      fxtime.wSecond = systime.wSecond;
      fxtime.wYear = systime.wYear;
    }
    return fxtime;
  }

  void FFI_OnChange() {
    if (m_pInfo && m_pInfo->FFI_OnChange)
      m_pInfo->FFI_OnChange(m_pInfo);
  }

  FX_BOOL FFI_IsSHIFTKeyDown(uint32_t nFlag) const {
    return (nFlag & FWL_EVENTFLAG_ShiftKey) != 0;
  }

  FX_BOOL FFI_IsCTRLKeyDown(uint32_t nFlag) const {
    return (nFlag & FWL_EVENTFLAG_ControlKey) != 0;
  }

  FX_BOOL FFI_IsALTKeyDown(uint32_t nFlag) const {
    return (nFlag & FWL_EVENTFLAG_AltKey) != 0;
  }

  FPDF_PAGE FFI_GetPage(FPDF_DOCUMENT document, int nPageIndex) {
    if (m_pInfo && m_pInfo->FFI_GetPage)
      return m_pInfo->FFI_GetPage(m_pInfo, document, nPageIndex);
    return nullptr;
  }

  FPDF_PAGE FFI_GetCurrentPage(FPDF_DOCUMENT document) {
    if (m_pInfo && m_pInfo->FFI_GetCurrentPage)
      return m_pInfo->FFI_GetCurrentPage(m_pInfo, document);
    return nullptr;
  }

  int FFI_GetRotation(FPDF_PAGE page) {
    if (m_pInfo && m_pInfo->FFI_GetRotation)
      return m_pInfo->FFI_GetRotation(m_pInfo, page);
    return 0;
  }

  void FFI_ExecuteNamedAction(const FX_CHAR* namedAction) {
    if (m_pInfo && m_pInfo->FFI_ExecuteNamedAction)
      m_pInfo->FFI_ExecuteNamedAction(m_pInfo, namedAction);
  }

  void FFI_OnSetFieldInputFocus(void* field,
                                FPDF_WIDESTRING focusText,
                                FPDF_DWORD nTextLen,
                                FX_BOOL bFocus) {
    if (m_pInfo && m_pInfo->FFI_SetTextFieldFocus)
      m_pInfo->FFI_SetTextFieldFocus(m_pInfo, focusText, nTextLen, bFocus);
  }

  void FFI_DoURIAction(const FX_CHAR* bsURI) {
    if (m_pInfo && m_pInfo->FFI_DoURIAction)
      m_pInfo->FFI_DoURIAction(m_pInfo, bsURI);
  }

  void FFI_DoGoToAction(int nPageIndex,
                        int zoomMode,
                        float* fPosArray,
                        int sizeOfArray) {
    if (m_pInfo && m_pInfo->FFI_DoGoToAction)
      m_pInfo->FFI_DoGoToAction(m_pInfo, nPageIndex, zoomMode, fPosArray,
                                sizeOfArray);
  }

#ifdef PDF_ENABLE_XFA
  void FFI_DisplayCaret(FPDF_PAGE page,
                        FPDF_BOOL bVisible,
                        double left,
                        double top,
                        double right,
                        double bottom) {
    if (m_pInfo && m_pInfo->FFI_DisplayCaret)
      m_pInfo->FFI_DisplayCaret(m_pInfo, page, bVisible, left, top, right,
                                bottom);
  }

  int FFI_GetCurrentPageIndex(FPDF_DOCUMENT document) {
    if (!m_pInfo || !m_pInfo->FFI_GetCurrentPageIndex) {
      return -1;
    }
    return m_pInfo->FFI_GetCurrentPageIndex(m_pInfo, document);
  }

  void FFI_SetCurrentPage(FPDF_DOCUMENT document, int iCurPage) {
    if (m_pInfo && m_pInfo->FFI_SetCurrentPage)
      m_pInfo->FFI_SetCurrentPage(m_pInfo, document, iCurPage);
  }

  CFX_WideString FFI_GetAppName() const { return CFX_WideString(L"Acrobat"); }

  CFX_WideString FFI_GetPlatform() {
    if (m_pInfo && m_pInfo->FFI_GetPlatform) {
      int nRequiredLen = m_pInfo->FFI_GetPlatform(m_pInfo, nullptr, 0);
      if (nRequiredLen <= 0)
        return L"";

      char* pbuff = new char[nRequiredLen];
      memset(pbuff, 0, nRequiredLen);
      int nActualLen = m_pInfo->FFI_GetPlatform(m_pInfo, pbuff, nRequiredLen);
      if (nActualLen <= 0 || nActualLen > nRequiredLen) {
        delete[] pbuff;
        return L"";
      }
      CFX_ByteString bsRet = CFX_ByteString(pbuff, nActualLen);
      CFX_WideString wsRet = CFX_WideString::FromUTF16LE(
          (unsigned short*)bsRet.GetBuffer(bsRet.GetLength()),
          bsRet.GetLength() / sizeof(unsigned short));
      delete[] pbuff;
      return wsRet;
    }
    return L"";
  }

  void FFI_GotoURL(FPDF_DOCUMENT document,
                   const CFX_WideStringC& wsURL,
                   FX_BOOL bAppend) {
    if (m_pInfo && m_pInfo->FFI_GotoURL) {
      CFX_ByteString bsTo = CFX_WideString(wsURL).UTF16LE_Encode();
      FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(wsURL.GetLength());
      m_pInfo->FFI_GotoURL(m_pInfo, document, pTo);
      bsTo.ReleaseBuffer();
    }
  }

  void FFI_GetPageViewRect(FPDF_PAGE page, FS_RECTF& dstRect) {
    if (m_pInfo && m_pInfo->FFI_GetPageViewRect) {
      double left;
      double top;
      double right;
      double bottom;
      m_pInfo->FFI_GetPageViewRect(m_pInfo, page, &left, &top, &right, &bottom);

      dstRect.left = static_cast<float>(left);
      dstRect.top = static_cast<float>(top < bottom ? bottom : top);
      dstRect.bottom = static_cast<float>(top < bottom ? top : bottom);
      dstRect.right = static_cast<float>(right);
    }
  }

  FX_BOOL FFI_PopupMenu(FPDF_PAGE page,
                        FPDF_WIDGET hWidget,
                        int menuFlag,
                        CFX_PointF ptPopup,
                        const CFX_PointF* pRectExclude) {
    if (m_pInfo && m_pInfo->FFI_PopupMenu)
      return m_pInfo->FFI_PopupMenu(m_pInfo, page, hWidget, menuFlag, ptPopup.x,
                                    ptPopup.y);
    return FALSE;
  }

  void FFI_Alert(FPDF_WIDESTRING Msg,
                 FPDF_WIDESTRING Title,
                 int Type,
                 int Icon) {
    if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_alert)
      m_pInfo->m_pJsPlatform->app_alert(m_pInfo->m_pJsPlatform, Msg, Title,
                                        Type, Icon);
  }

  void FFI_EmailTo(FPDF_FILEHANDLER* fileHandler,
                   FPDF_WIDESTRING pTo,
                   FPDF_WIDESTRING pSubject,
                   FPDF_WIDESTRING pCC,
                   FPDF_WIDESTRING pBcc,
                   FPDF_WIDESTRING pMsg) {
    if (m_pInfo && m_pInfo->FFI_EmailTo)
      m_pInfo->FFI_EmailTo(m_pInfo, fileHandler, pTo, pSubject, pCC, pBcc,
                           pMsg);
  }

  void FFI_UploadTo(FPDF_FILEHANDLER* fileHandler,
                    int fileFlag,
                    FPDF_WIDESTRING uploadTo) {
    if (m_pInfo && m_pInfo->FFI_UploadTo)
      m_pInfo->FFI_UploadTo(m_pInfo, fileHandler, fileFlag, uploadTo);
  }

  FPDF_FILEHANDLER* FFI_OpenFile(int fileType,
                                 FPDF_WIDESTRING wsURL,
                                 const char* mode) {
    if (m_pInfo && m_pInfo->FFI_OpenFile)
      return m_pInfo->FFI_OpenFile(m_pInfo, fileType, wsURL, mode);
    return nullptr;
  }

  CFX_WideString FFI_GetFilePath(FPDF_FILEHANDLER* pFileHandler) const {
    return L"";
  }

  int FFI_GetDocumentCount() const { return 0; }
  int FFI_GetCurDocument() const { return 0; }

  IFX_FileRead* FFI_DownloadFromURL(const FX_WCHAR* url) {
    if (m_pInfo && m_pInfo->FFI_DownloadFromURL) {
      CFX_ByteString bstrURL = CFX_WideString(url).UTF16LE_Encode();
      FPDF_WIDESTRING wsURL =
          (FPDF_WIDESTRING)bstrURL.GetBuffer(bstrURL.GetLength());

      FPDF_LPFILEHANDLER fileHandler =
          m_pInfo->FFI_DownloadFromURL(m_pInfo, wsURL);

      return new CFPDF_FileStream(fileHandler);
    }
    return nullptr;
  }

  CFX_WideString FFI_PostRequestURL(const FX_WCHAR* wsURL,
                                    const FX_WCHAR* wsData,
                                    const FX_WCHAR* wsContentType,
                                    const FX_WCHAR* wsEncode,
                                    const FX_WCHAR* wsHeader) {
    if (m_pInfo && m_pInfo->FFI_PostRequestURL) {
      CFX_ByteString bsURL = CFX_WideString(wsURL).UTF16LE_Encode();
      FPDF_WIDESTRING URL = (FPDF_WIDESTRING)bsURL.GetBuffer(bsURL.GetLength());

      CFX_ByteString bsData = CFX_WideString(wsData).UTF16LE_Encode();
      FPDF_WIDESTRING data =
          (FPDF_WIDESTRING)bsData.GetBuffer(bsData.GetLength());

      CFX_ByteString bsContentType =
          CFX_WideString(wsContentType).UTF16LE_Encode();
      FPDF_WIDESTRING contentType =
          (FPDF_WIDESTRING)bsContentType.GetBuffer(bsContentType.GetLength());

      CFX_ByteString bsEncode = CFX_WideString(wsEncode).UTF16LE_Encode();
      FPDF_WIDESTRING encode =
          (FPDF_WIDESTRING)bsEncode.GetBuffer(bsEncode.GetLength());

      CFX_ByteString bsHeader = CFX_WideString(wsHeader).UTF16LE_Encode();
      FPDF_WIDESTRING header =
          (FPDF_WIDESTRING)bsHeader.GetBuffer(bsHeader.GetLength());

      FPDF_BSTR response;
      FPDF_BStr_Init(&response);
      m_pInfo->FFI_PostRequestURL(m_pInfo, URL, data, contentType, encode,
                                  header, &response);

      CFX_WideString wsRet = CFX_WideString::FromUTF16LE(
          (unsigned short*)response.str, response.len / sizeof(unsigned short));
      FPDF_BStr_Clear(&response);

      return wsRet;
    }
    return L"";
  }

  FPDF_BOOL FFI_PutRequestURL(const FX_WCHAR* wsURL,
                              const FX_WCHAR* wsData,
                              const FX_WCHAR* wsEncode) {
    if (m_pInfo && m_pInfo->FFI_PutRequestURL) {
      CFX_ByteString bsURL = CFX_WideString(wsURL).UTF16LE_Encode();
      FPDF_WIDESTRING URL = (FPDF_WIDESTRING)bsURL.GetBuffer(bsURL.GetLength());

      CFX_ByteString bsData = CFX_WideString(wsData).UTF16LE_Encode();
      FPDF_WIDESTRING data =
          (FPDF_WIDESTRING)bsData.GetBuffer(bsData.GetLength());

      CFX_ByteString bsEncode = CFX_WideString(wsEncode).UTF16LE_Encode();
      FPDF_WIDESTRING encode =
          (FPDF_WIDESTRING)bsEncode.GetBuffer(bsEncode.GetLength());

      return m_pInfo->FFI_PutRequestURL(m_pInfo, URL, data, encode);
    }
    return FALSE;
  }

  CFX_WideString FFI_GetLanguage() {
    if (m_pInfo && m_pInfo->FFI_GetLanguage) {
      int nRequiredLen = m_pInfo->FFI_GetLanguage(m_pInfo, nullptr, 0);
      if (nRequiredLen <= 0)
        return L"";

      char* pbuff = new char[nRequiredLen];
      memset(pbuff, 0, nRequiredLen);
      int nActualLen = m_pInfo->FFI_GetLanguage(m_pInfo, pbuff, nRequiredLen);
      if (nActualLen <= 0 || nActualLen > nRequiredLen) {
        delete[] pbuff;
        return L"";
      }
      CFX_ByteString bsRet = CFX_ByteString(pbuff, nActualLen);
      CFX_WideString wsRet = CFX_WideString::FromUTF16LE(
          (unsigned short*)bsRet.GetBuffer(bsRet.GetLength()),
          bsRet.GetLength() / sizeof(unsigned short));
      delete[] pbuff;
      return wsRet;
    }
    return L"";
  }

  void FFI_PageEvent(int iPageCount, uint32_t dwEventType) const {
    if (m_pInfo && m_pInfo->FFI_PageEvent)
      m_pInfo->FFI_PageEvent(m_pInfo, iPageCount, dwEventType);
  }
#endif  // PDF_ENABLE_XFA

  int JS_appAlert(const FX_WCHAR* Msg,
                  const FX_WCHAR* Title,
                  FX_UINT Type,
                  FX_UINT Icon);
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
  void SetSDKDocument(CPDFSDK_Document* pFXDoc) { m_pSDKDoc = pFXDoc; }
  CPDFSDK_Document* GetSDKDocument() const { return m_pSDKDoc; }
  UnderlyingDocumentType* GetUnderlyingDocument() const {
    return m_pUnderlyingDoc;
  }
  CFX_ByteString GetAppName() const { return ""; }
  CFX_SystemHandler* GetSysHandler() const { return m_pSysHandler.get(); }
  FPDF_FORMFILLINFO* GetFormFillInfo() const { return m_pInfo; }

  CFFL_IFormFiller* GetIFormFiller();             // Creates if not present.
  CPDFSDK_AnnotHandlerMgr* GetAnnotHandlerMgr();  // Creates if not present.
  IJS_Runtime* GetJSRuntime();                    // Creates if not present.
  CPDFSDK_ActionHandler* GetActionHander();       // Creates if not present.

 private:
  std::unique_ptr<CPDFSDK_AnnotHandlerMgr> m_pAnnotHandlerMgr;
  std::unique_ptr<CPDFSDK_ActionHandler> m_pActionHandler;
  std::unique_ptr<IJS_Runtime> m_pJSRuntime;
  FPDF_FORMFILLINFO* const m_pInfo;
  CPDFSDK_Document* m_pSDKDoc;
  UnderlyingDocumentType* const m_pUnderlyingDoc;
  std::unique_ptr<CFFL_IFormFiller> m_pIFormFiller;
  std::unique_ptr<CFX_SystemHandler> m_pSysHandler;
};

#endif  // FPDFSDK_INCLUDE_CPDFDOC_ENVIRONMENT_H_
