// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_formfillenvironment.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfdoc/cpdf_docjsactions.h"
#include "fpdfsdk/cpdfsdk_annothandlermgr.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/fsdk_actionhandler.h"
#include "fxjs/ijs_runtime.h"
#include "third_party/base/ptr_util.h"

namespace {

// NOTE: |bsUTF16LE| must outlive the use of the result. Care must be taken
// since modifying the result would impact |bsUTF16LE|.
FPDF_WIDESTRING AsFPDFWideString(ByteString* bsUTF16LE) {
  return reinterpret_cast<FPDF_WIDESTRING>(
      bsUTF16LE->GetBuffer(bsUTF16LE->GetLength()));
}

}  // namespace

CPDFSDK_FormFillEnvironment::CPDFSDK_FormFillEnvironment(
    UnderlyingDocumentType* pDoc,
    FPDF_FORMFILLINFO* pFFinfo)
    : m_pInfo(pFFinfo),
      m_pUnderlyingDoc(pDoc),
      m_pSysHandler(pdfium::MakeUnique<CFX_SystemHandler>(this)),
      m_bChangeMask(false),
      m_bBeingDestroyed(false) {}

CPDFSDK_FormFillEnvironment::~CPDFSDK_FormFillEnvironment() {
  m_bBeingDestroyed = true;
  ClearAllFocusedAnnots();

  // |m_PageMap| will try to access |m_pInterForm| when it cleans itself up.
  // Make sure it is deleted before |m_pInterForm|.
  m_PageMap.clear();

  // |m_pAnnotHandlerMgr| will try to access |m_pFormFiller| when it cleans
  // itself up. Make sure it is deleted before |m_pFormFiller|.
  m_pAnnotHandlerMgr.reset();

  // Must destroy the |m_pFormFiller| before the environment (|this|)
  // because any created form widgets hold a pointer to the environment.
  // Those widgets may call things like KillTimer() as they are shutdown.
  m_pFormFiller.reset();

  if (m_pInfo && m_pInfo->Release)
    m_pInfo->Release(m_pInfo);
}

int CPDFSDK_FormFillEnvironment::JS_appAlert(const wchar_t* Msg,
                                             const wchar_t* Title,
                                             uint32_t Type,
                                             uint32_t Icon) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_alert) {
    return -1;
  }
  ByteString bsMsg = WideString(Msg).UTF16LE_Encode();
  ByteString bsTitle = WideString(Title).UTF16LE_Encode();
  return m_pInfo->m_pJsPlatform->app_alert(
      m_pInfo->m_pJsPlatform, AsFPDFWideString(&bsMsg),
      AsFPDFWideString(&bsTitle), Type, Icon);
}

int CPDFSDK_FormFillEnvironment::JS_appResponse(const wchar_t* Question,
                                                const wchar_t* Title,
                                                const wchar_t* Default,
                                                const wchar_t* cLabel,
                                                FPDF_BOOL bPassword,
                                                void* response,
                                                int length) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_response) {
    return -1;
  }
  ByteString bsQuestion = WideString(Question).UTF16LE_Encode();
  ByteString bsTitle = WideString(Title).UTF16LE_Encode();
  ByteString bsDefault = WideString(Default).UTF16LE_Encode();
  ByteString bsLabel = WideString(cLabel).UTF16LE_Encode();
  return m_pInfo->m_pJsPlatform->app_response(
      m_pInfo->m_pJsPlatform, AsFPDFWideString(&bsQuestion),
      AsFPDFWideString(&bsTitle), AsFPDFWideString(&bsDefault),
      AsFPDFWideString(&bsLabel), bPassword, response, length);
}

void CPDFSDK_FormFillEnvironment::JS_appBeep(int nType) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_beep) {
    return;
  }
  m_pInfo->m_pJsPlatform->app_beep(m_pInfo->m_pJsPlatform, nType);
}

WideString CPDFSDK_FormFillEnvironment::JS_fieldBrowse() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Field_browse) {
    return WideString();
  }
  const int nRequiredLen =
      m_pInfo->m_pJsPlatform->Field_browse(m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return WideString();

  std::vector<uint8_t> pBuff(nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Field_browse(
      m_pInfo->m_pJsPlatform, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return WideString();

  pBuff.resize(nActualLen);
  return WideString::FromLocal(ByteStringView(pBuff));
}

WideString CPDFSDK_FormFillEnvironment::JS_docGetFilePath() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_getFilePath) {
    return WideString();
  }
  const int nRequiredLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return WideString();

  std::vector<uint8_t> pBuff(nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return WideString();

  pBuff.resize(nActualLen);
  return WideString::FromLocal(ByteStringView(pBuff));
}

void CPDFSDK_FormFillEnvironment::JS_docSubmitForm(void* formData,
                                                   int length,
                                                   const wchar_t* URL) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_submitForm) {
    return;
  }
  ByteString bsDestination = WideString(URL).UTF16LE_Encode();
  m_pInfo->m_pJsPlatform->Doc_submitForm(m_pInfo->m_pJsPlatform, formData,
                                         length,
                                         AsFPDFWideString(&bsDestination));
}

void CPDFSDK_FormFillEnvironment::JS_docmailForm(void* mailData,
                                                 int length,
                                                 FPDF_BOOL bUI,
                                                 const wchar_t* To,
                                                 const wchar_t* Subject,
                                                 const wchar_t* CC,
                                                 const wchar_t* BCC,
                                                 const wchar_t* Msg) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_mail) {
    return;
  }
  ByteString bsTo = WideString(To).UTF16LE_Encode();
  ByteString bsSubject = WideString(Subject).UTF16LE_Encode();
  ByteString bsCC = WideString(CC).UTF16LE_Encode();
  ByteString bsBcc = WideString(BCC).UTF16LE_Encode();
  ByteString bsMsg = WideString(Msg).UTF16LE_Encode();
  m_pInfo->m_pJsPlatform->Doc_mail(
      m_pInfo->m_pJsPlatform, mailData, length, bUI, AsFPDFWideString(&bsTo),
      AsFPDFWideString(&bsSubject), AsFPDFWideString(&bsCC),
      AsFPDFWideString(&bsBcc), AsFPDFWideString(&bsMsg));
}

void CPDFSDK_FormFillEnvironment::JS_docprint(FPDF_BOOL bUI,
                                              int nStart,
                                              int nEnd,
                                              FPDF_BOOL bSilent,
                                              FPDF_BOOL bShrinkToFit,
                                              FPDF_BOOL bPrintAsImage,
                                              FPDF_BOOL bReverse,
                                              FPDF_BOOL bAnnotations) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_print) {
    return;
  }
  m_pInfo->m_pJsPlatform->Doc_print(m_pInfo->m_pJsPlatform, bUI, nStart, nEnd,
                                    bSilent, bShrinkToFit, bPrintAsImage,
                                    bReverse, bAnnotations);
}

void CPDFSDK_FormFillEnvironment::JS_docgotoPage(int nPageNum) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_gotoPage) {
    return;
  }
  m_pInfo->m_pJsPlatform->Doc_gotoPage(m_pInfo->m_pJsPlatform, nPageNum);
}

IJS_Runtime* CPDFSDK_FormFillEnvironment::GetJSRuntime() {
  if (!IsJSInitiated())
    return nullptr;
  if (!m_pJSRuntime)
    m_pJSRuntime.reset(IJS_Runtime::Create(this));
  return m_pJSRuntime.get();
}

CPDFSDK_AnnotHandlerMgr* CPDFSDK_FormFillEnvironment::GetAnnotHandlerMgr() {
  if (!m_pAnnotHandlerMgr)
    m_pAnnotHandlerMgr = pdfium::MakeUnique<CPDFSDK_AnnotHandlerMgr>(this);
  return m_pAnnotHandlerMgr.get();
}

CPDFSDK_ActionHandler* CPDFSDK_FormFillEnvironment::GetActionHandler() {
  if (!m_pActionHandler)
    m_pActionHandler = pdfium::MakeUnique<CPDFSDK_ActionHandler>();
  return m_pActionHandler.get();
}

CFFL_InteractiveFormFiller*
CPDFSDK_FormFillEnvironment::GetInteractiveFormFiller() {
  if (!m_pFormFiller)
    m_pFormFiller = pdfium::MakeUnique<CFFL_InteractiveFormFiller>(this);
  return m_pFormFiller.get();
}

void CPDFSDK_FormFillEnvironment::Invalidate(UnderlyingPageType* page,
                                             const FX_RECT& rect) {
  if (m_pInfo && m_pInfo->FFI_Invalidate) {
    m_pInfo->FFI_Invalidate(m_pInfo, page, rect.left, rect.top, rect.right,
                            rect.bottom);
  }
}

void CPDFSDK_FormFillEnvironment::OutputSelectedRect(
    UnderlyingPageType* page,
    const CFX_FloatRect& rect) {
  if (m_pInfo && m_pInfo->FFI_OutputSelectedRect) {
    m_pInfo->FFI_OutputSelectedRect(m_pInfo, page, rect.left, rect.top,
                                    rect.right, rect.bottom);
  }
}

void CPDFSDK_FormFillEnvironment::SetCursor(int nCursorType) {
  if (m_pInfo && m_pInfo->FFI_SetCursor)
    m_pInfo->FFI_SetCursor(m_pInfo, nCursorType);
}

int CPDFSDK_FormFillEnvironment::SetTimer(int uElapse,
                                          TimerCallback lpTimerFunc) {
  if (m_pInfo && m_pInfo->FFI_SetTimer)
    return m_pInfo->FFI_SetTimer(m_pInfo, uElapse, lpTimerFunc);
  return -1;
}

void CPDFSDK_FormFillEnvironment::KillTimer(int nTimerID) {
  if (m_pInfo && m_pInfo->FFI_KillTimer)
    m_pInfo->FFI_KillTimer(m_pInfo, nTimerID);
}

FX_SYSTEMTIME CPDFSDK_FormFillEnvironment::GetLocalTime() const {
  FX_SYSTEMTIME fxtime;
  if (!m_pInfo || !m_pInfo->FFI_GetLocalTime)
    return fxtime;

  FPDF_SYSTEMTIME systime = m_pInfo->FFI_GetLocalTime(m_pInfo);
  fxtime.wDay = systime.wDay;
  fxtime.wDayOfWeek = systime.wDayOfWeek;
  fxtime.wHour = systime.wHour;
  fxtime.wMilliseconds = systime.wMilliseconds;
  fxtime.wMinute = systime.wMinute;
  fxtime.wMonth = systime.wMonth;
  fxtime.wSecond = systime.wSecond;
  fxtime.wYear = systime.wYear;
  return fxtime;
}

void CPDFSDK_FormFillEnvironment::OnChange() {
  if (m_pInfo && m_pInfo->FFI_OnChange)
    m_pInfo->FFI_OnChange(m_pInfo);
}

FPDF_PAGE CPDFSDK_FormFillEnvironment::GetCurrentPage(
    UnderlyingDocumentType* document) {
  if (m_pInfo && m_pInfo->FFI_GetCurrentPage)
    return m_pInfo->FFI_GetCurrentPage(m_pInfo, document);
  return nullptr;
}

void CPDFSDK_FormFillEnvironment::ExecuteNamedAction(const char* namedAction) {
  if (m_pInfo && m_pInfo->FFI_ExecuteNamedAction)
    m_pInfo->FFI_ExecuteNamedAction(m_pInfo, namedAction);
}

void CPDFSDK_FormFillEnvironment::OnSetFieldInputFocus(
    FPDF_WIDESTRING focusText,
    FPDF_DWORD nTextLen,
    bool bFocus) {
  if (m_pInfo && m_pInfo->FFI_SetTextFieldFocus)
    m_pInfo->FFI_SetTextFieldFocus(m_pInfo, focusText, nTextLen, bFocus);
}

void CPDFSDK_FormFillEnvironment::DoURIAction(const char* bsURI) {
  if (m_pInfo && m_pInfo->FFI_DoURIAction)
    m_pInfo->FFI_DoURIAction(m_pInfo, bsURI);
}

void CPDFSDK_FormFillEnvironment::DoGoToAction(int nPageIndex,
                                               int zoomMode,
                                               float* fPosArray,
                                               int sizeOfArray) {
  if (m_pInfo && m_pInfo->FFI_DoGoToAction) {
    m_pInfo->FFI_DoGoToAction(m_pInfo, nPageIndex, zoomMode, fPosArray,
                              sizeOfArray);
  }
}

#ifdef PDF_ENABLE_XFA
void CPDFSDK_FormFillEnvironment::DisplayCaret(CPDFXFA_Page* page,
                                               FPDF_BOOL bVisible,
                                               double left,
                                               double top,
                                               double right,
                                               double bottom) {
  if (m_pInfo && m_pInfo->FFI_DisplayCaret) {
    m_pInfo->FFI_DisplayCaret(m_pInfo, page, bVisible, left, top, right,
                              bottom);
  }
}

int CPDFSDK_FormFillEnvironment::GetCurrentPageIndex(
    CPDFXFA_Context* document) {
  if (!m_pInfo || !m_pInfo->FFI_GetCurrentPageIndex)
    return -1;
  return m_pInfo->FFI_GetCurrentPageIndex(m_pInfo, document);
}

void CPDFSDK_FormFillEnvironment::SetCurrentPage(CPDFXFA_Context* document,
                                                 int iCurPage) {
  if (m_pInfo && m_pInfo->FFI_SetCurrentPage)
    m_pInfo->FFI_SetCurrentPage(m_pInfo, document, iCurPage);
}

WideString CPDFSDK_FormFillEnvironment::GetPlatform() {
  if (!m_pInfo || !m_pInfo->FFI_GetPlatform)
    return WideString();

  int nRequiredLen = m_pInfo->FFI_GetPlatform(m_pInfo, nullptr, 0);
  if (nRequiredLen <= 0)
    return WideString();

  std::vector<uint8_t> pBuff(nRequiredLen);
  int nActualLen =
      m_pInfo->FFI_GetPlatform(m_pInfo, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return WideString();

  return WideString::FromUTF16LE(reinterpret_cast<uint16_t*>(pBuff.data()),
                                 nActualLen / sizeof(uint16_t));
}

void CPDFSDK_FormFillEnvironment::GotoURL(CPDFXFA_Context* document,
                                          const WideStringView& wsURL) {
  if (!m_pInfo || !m_pInfo->FFI_GotoURL)
    return;

  ByteString bsTo = WideString(wsURL).UTF16LE_Encode();
  FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(wsURL.GetLength());
  m_pInfo->FFI_GotoURL(m_pInfo, document, pTo);
  bsTo.ReleaseBuffer(bsTo.GetStringLength());
}

void CPDFSDK_FormFillEnvironment::GetPageViewRect(CPDFXFA_Page* page,
                                                  FS_RECTF& dstRect) {
  if (!m_pInfo || !m_pInfo->FFI_GetPageViewRect)
    return;

  double left;
  double top;
  double right;
  double bottom;
  m_pInfo->FFI_GetPageViewRect(m_pInfo, page, &left, &top, &right, &bottom);
  if (top < bottom)
    std::swap(top, bottom);

  dstRect.left = static_cast<float>(left);
  dstRect.top = static_cast<float>(top);
  dstRect.bottom = static_cast<float>(bottom);
  dstRect.right = static_cast<float>(right);
}

bool CPDFSDK_FormFillEnvironment::PopupMenu(CPDFXFA_Page* page,
                                            FPDF_WIDGET hWidget,
                                            int menuFlag,
                                            CFX_PointF pt) {
  return m_pInfo && m_pInfo->FFI_PopupMenu &&
         m_pInfo->FFI_PopupMenu(m_pInfo, page, hWidget, menuFlag, pt.x, pt.y);
}

void CPDFSDK_FormFillEnvironment::Alert(FPDF_WIDESTRING Msg,
                                        FPDF_WIDESTRING Title,
                                        int Type,
                                        int Icon) {
  if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_alert) {
    m_pInfo->m_pJsPlatform->app_alert(m_pInfo->m_pJsPlatform, Msg, Title, Type,
                                      Icon);
  }
}

void CPDFSDK_FormFillEnvironment::EmailTo(FPDF_FILEHANDLER* fileHandler,
                                          FPDF_WIDESTRING pTo,
                                          FPDF_WIDESTRING pSubject,
                                          FPDF_WIDESTRING pCC,
                                          FPDF_WIDESTRING pBcc,
                                          FPDF_WIDESTRING pMsg) {
  if (m_pInfo && m_pInfo->FFI_EmailTo)
    m_pInfo->FFI_EmailTo(m_pInfo, fileHandler, pTo, pSubject, pCC, pBcc, pMsg);
}

void CPDFSDK_FormFillEnvironment::UploadTo(FPDF_FILEHANDLER* fileHandler,
                                           int fileFlag,
                                           FPDF_WIDESTRING uploadTo) {
  if (m_pInfo && m_pInfo->FFI_UploadTo)
    m_pInfo->FFI_UploadTo(m_pInfo, fileHandler, fileFlag, uploadTo);
}

FPDF_FILEHANDLER* CPDFSDK_FormFillEnvironment::OpenFile(int fileType,
                                                        FPDF_WIDESTRING wsURL,
                                                        const char* mode) {
  if (m_pInfo && m_pInfo->FFI_OpenFile)
    return m_pInfo->FFI_OpenFile(m_pInfo, fileType, wsURL, mode);
  return nullptr;
}

RetainPtr<IFX_SeekableReadStream> CPDFSDK_FormFillEnvironment::DownloadFromURL(
    const wchar_t* url) {
  if (!m_pInfo || !m_pInfo->FFI_DownloadFromURL)
    return nullptr;

  ByteString bstrURL = WideString(url).UTF16LE_Encode();
  FPDF_WIDESTRING wsURL =
      (FPDF_WIDESTRING)bstrURL.GetBuffer(bstrURL.GetLength());

  FPDF_LPFILEHANDLER fileHandler = m_pInfo->FFI_DownloadFromURL(m_pInfo, wsURL);
  return MakeSeekableStream(fileHandler);
}

WideString CPDFSDK_FormFillEnvironment::PostRequestURL(
    const wchar_t* wsURL,
    const wchar_t* wsData,
    const wchar_t* wsContentType,
    const wchar_t* wsEncode,
    const wchar_t* wsHeader) {
  if (!m_pInfo || !m_pInfo->FFI_PostRequestURL)
    return L"";

  ByteString bsURL = WideString(wsURL).UTF16LE_Encode();
  FPDF_WIDESTRING URL = (FPDF_WIDESTRING)bsURL.GetBuffer(bsURL.GetLength());

  ByteString bsData = WideString(wsData).UTF16LE_Encode();
  FPDF_WIDESTRING data = (FPDF_WIDESTRING)bsData.GetBuffer(bsData.GetLength());

  ByteString bsContentType = WideString(wsContentType).UTF16LE_Encode();
  FPDF_WIDESTRING contentType =
      (FPDF_WIDESTRING)bsContentType.GetBuffer(bsContentType.GetLength());

  ByteString bsEncode = WideString(wsEncode).UTF16LE_Encode();
  FPDF_WIDESTRING encode =
      (FPDF_WIDESTRING)bsEncode.GetBuffer(bsEncode.GetLength());

  ByteString bsHeader = WideString(wsHeader).UTF16LE_Encode();
  FPDF_WIDESTRING header =
      (FPDF_WIDESTRING)bsHeader.GetBuffer(bsHeader.GetLength());

  FPDF_BSTR response;
  FPDF_BStr_Init(&response);
  m_pInfo->FFI_PostRequestURL(m_pInfo, URL, data, contentType, encode, header,
                              &response);

  WideString wsRet = WideString::FromUTF16LE(
      (FPDF_WIDESTRING)response.str, response.len / sizeof(FPDF_WIDESTRING));
  FPDF_BStr_Clear(&response);

  return wsRet;
}

FPDF_BOOL CPDFSDK_FormFillEnvironment::PutRequestURL(const wchar_t* wsURL,
                                                     const wchar_t* wsData,
                                                     const wchar_t* wsEncode) {
  if (!m_pInfo || !m_pInfo->FFI_PutRequestURL)
    return false;

  ByteString bsURL = WideString(wsURL).UTF16LE_Encode();
  FPDF_WIDESTRING URL = (FPDF_WIDESTRING)bsURL.GetBuffer(bsURL.GetLength());

  ByteString bsData = WideString(wsData).UTF16LE_Encode();
  FPDF_WIDESTRING data = (FPDF_WIDESTRING)bsData.GetBuffer(bsData.GetLength());

  ByteString bsEncode = WideString(wsEncode).UTF16LE_Encode();
  FPDF_WIDESTRING encode =
      (FPDF_WIDESTRING)bsEncode.GetBuffer(bsEncode.GetLength());

  return m_pInfo->FFI_PutRequestURL(m_pInfo, URL, data, encode);
}

WideString CPDFSDK_FormFillEnvironment::GetLanguage() {
  if (!m_pInfo || !m_pInfo->FFI_GetLanguage)
    return WideString();

  int nRequiredLen = m_pInfo->FFI_GetLanguage(m_pInfo, nullptr, 0);
  if (nRequiredLen <= 0)
    return WideString();

  std::vector<uint8_t> pBuff(nRequiredLen);
  int nActualLen =
      m_pInfo->FFI_GetLanguage(m_pInfo, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return WideString();

  return WideString::FromUTF16LE(reinterpret_cast<uint16_t*>(pBuff.data()),
                                 nActualLen / sizeof(uint16_t));
}

void CPDFSDK_FormFillEnvironment::PageEvent(int iPageCount,
                                            uint32_t dwEventType) const {
  if (m_pInfo && m_pInfo->FFI_PageEvent)
    m_pInfo->FFI_PageEvent(m_pInfo, iPageCount, dwEventType);
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_FormFillEnvironment::ClearAllFocusedAnnots() {
  for (auto& it : m_PageMap) {
    if (it.second->IsValidSDKAnnot(GetFocusAnnot()))
      KillFocusAnnot(0);
  }
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetPageView(
    UnderlyingPageType* pUnderlyingPage,
    bool renew) {
  auto it = m_PageMap.find(pUnderlyingPage);
  if (it != m_PageMap.end())
    return it->second.get();

  if (!renew)
    return nullptr;

  auto pNew = pdfium::MakeUnique<CPDFSDK_PageView>(this, pUnderlyingPage);
  CPDFSDK_PageView* pPageView = pNew.get();
  m_PageMap[pUnderlyingPage] = std::move(pNew);

  // Delay to load all the annotations, to avoid endless loop.
  pPageView->LoadFXAnnots();
  return pPageView;
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetCurrentView() {
  UnderlyingPageType* pPage =
      UnderlyingFromFPDFPage(GetCurrentPage(m_pUnderlyingDoc.Get()));
  return pPage ? GetPageView(pPage, true) : nullptr;
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetPageView(int nIndex) {
  UnderlyingPageType* pTempPage = GetPage(nIndex);
  if (!pTempPage)
    return nullptr;

  auto it = m_PageMap.find(pTempPage);
  return it != m_PageMap.end() ? it->second.get() : nullptr;
}

void CPDFSDK_FormFillEnvironment::ProcJavascriptFun() {
  CPDF_Document* pPDFDoc = GetPDFDocument();
  CPDF_DocJSActions docJS(pPDFDoc);
  int iCount = docJS.CountJSActions();
  for (int i = 0; i < iCount; i++) {
    WideString csJSName;
    CPDF_Action jsAction = docJS.GetJSActionAndName(i, &csJSName);
    GetActionHandler()->DoAction_JavaScript(jsAction, csJSName, this);
  }
}

bool CPDFSDK_FormFillEnvironment::ProcOpenAction() {
  if (!m_pUnderlyingDoc)
    return false;

  const CPDF_Dictionary* pRoot = GetPDFDocument()->GetRoot();
  if (!pRoot)
    return false;

  CPDF_Object* pOpenAction = pRoot->GetDictFor("OpenAction");
  if (!pOpenAction)
    pOpenAction = pRoot->GetArrayFor("OpenAction");
  if (!pOpenAction)
    return false;

  if (pOpenAction->IsArray())
    return true;

  CPDF_Dictionary* pDict = pOpenAction->AsDictionary();
  if (!pDict)
    return false;

  CPDF_Action action(pDict);
  GetActionHandler()->DoAction_DocOpen(action, this);
  return true;
}

void CPDFSDK_FormFillEnvironment::RemovePageView(
    UnderlyingPageType* pUnderlyingPage) {
  auto it = m_PageMap.find(pUnderlyingPage);
  if (it == m_PageMap.end())
    return;

  CPDFSDK_PageView* pPageView = it->second.get();
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
  m_PageMap.erase(it);
}

UnderlyingPageType* CPDFSDK_FormFillEnvironment::GetPage(int nIndex) {
  if (!m_pInfo || !m_pInfo->FFI_GetPage)
    return nullptr;
  return UnderlyingFromFPDFPage(
      m_pInfo->FFI_GetPage(m_pInfo, m_pUnderlyingDoc.Get(), nIndex));
}

CPDFSDK_InterForm* CPDFSDK_FormFillEnvironment::GetInterForm() {
  if (!m_pInterForm)
    m_pInterForm = pdfium::MakeUnique<CPDFSDK_InterForm>(this);
  return m_pInterForm.get();
}

void CPDFSDK_FormFillEnvironment::UpdateAllViews(CPDFSDK_PageView* pSender,
                                                 CPDFSDK_Annot* pAnnot) {
  for (const auto& it : m_PageMap) {
    CPDFSDK_PageView* pPageView = it.second.get();
    if (pPageView != pSender)
      pPageView->UpdateView(pAnnot);
  }
}

bool CPDFSDK_FormFillEnvironment::SetFocusAnnot(
    CPDFSDK_Annot::ObservedPtr* pAnnot) {
  if (m_bBeingDestroyed)
    return false;
  if (m_pFocusAnnot == *pAnnot)
    return true;
  if (m_pFocusAnnot && !KillFocusAnnot(0))
    return false;
  if (!*pAnnot)
    return false;

  CPDFSDK_PageView* pPageView = (*pAnnot)->GetPageView();
  if (!pPageView || !pPageView->IsValid())
    return false;

  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = GetAnnotHandlerMgr();
  if (m_pFocusAnnot)
    return false;

#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot::ObservedPtr pLastFocusAnnot(m_pFocusAnnot.Get());
  if (!pAnnotHandler->Annot_OnChangeFocus(pAnnot, &pLastFocusAnnot))
    return false;
#endif  // PDF_ENABLE_XFA
  if (!pAnnotHandler->Annot_OnSetFocus(pAnnot, 0))
    return false;
  if (m_pFocusAnnot)
    return false;

  m_pFocusAnnot.Reset(pAnnot->Get());
  return true;
}

bool CPDFSDK_FormFillEnvironment::KillFocusAnnot(uint32_t nFlag) {
  if (!m_pFocusAnnot)
    return false;

  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = GetAnnotHandlerMgr();
  CPDFSDK_Annot::ObservedPtr pFocusAnnot(m_pFocusAnnot.Get());
  m_pFocusAnnot.Reset();

#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot::ObservedPtr pNull;
  if (!pAnnotHandler->Annot_OnChangeFocus(&pNull, &pFocusAnnot))
    return false;
#endif  // PDF_ENABLE_XFA

  if (!pAnnotHandler->Annot_OnKillFocus(&pFocusAnnot, nFlag)) {
    m_pFocusAnnot.Reset(pFocusAnnot.Get());
    return false;
  }

  if (pFocusAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET) {
    CPDFSDK_Widget* pWidget = static_cast<CPDFSDK_Widget*>(pFocusAnnot.Get());
    FormFieldType fieldType = pWidget->GetFieldType();
    if (fieldType == FormFieldType::kTextField ||
        fieldType == FormFieldType::kComboBox) {
      OnSetFieldInputFocus(nullptr, 0, false);
    }
  }
  return !m_pFocusAnnot;
}

bool CPDFSDK_FormFillEnvironment::GetPermissions(int nFlag) const {
  return !!(GetPDFDocument()->GetUserPermissions() & nFlag);
}
