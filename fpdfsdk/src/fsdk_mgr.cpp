// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>
#include <memory>

#include "fpdfsdk/include/fsdk_mgr.h"

#include "fpdfsdk/include/formfiller/FFL_FormFiller.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/javascript/IJavaScript.h"
#include "public/fpdf_ext.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_app.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_doc.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_page.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"
#endif  // PDF_ENABLE_XFA

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

class CFX_SystemHandler : public IFX_SystemHandler {
 public:
  explicit CFX_SystemHandler(CPDFDoc_Environment* pEnv)
      : m_pEnv(pEnv), m_nCharSet(-1) {}
  ~CFX_SystemHandler() override {}

 public:
  // IFX_SystemHandler
  void InvalidateRect(FX_HWND hWnd, FX_RECT rect) override;
  void OutputSelectedRect(void* pFormFiller, CPDF_Rect& rect) override;
  FX_BOOL IsSelectionImplemented() override;
  CFX_WideString GetClipboardText(FX_HWND hWnd) override { return L""; }
  FX_BOOL SetClipboardText(FX_HWND hWnd, CFX_WideString string) override {
    return FALSE;
  }
  void ClientToScreen(FX_HWND hWnd, int32_t& x, int32_t& y) override {}
  void ScreenToClient(FX_HWND hWnd, int32_t& x, int32_t& y) override {}
  void SetCursor(int32_t nCursorType) override;
  FX_HMENU CreatePopupMenu() override { return NULL; }
  FX_BOOL AppendMenuItem(FX_HMENU hMenu,
                         int32_t nIDNewItem,
                         CFX_WideString string) override {
    return FALSE;
  }
  FX_BOOL EnableMenuItem(FX_HMENU hMenu,
                         int32_t nIDItem,
                         FX_BOOL bEnabled) override {
    return FALSE;
  }
  int32_t TrackPopupMenu(FX_HMENU hMenu,
                         int32_t x,
                         int32_t y,
                         FX_HWND hParent) override {
    return -1;
  }
  void DestroyMenu(FX_HMENU hMenu) override {}
  CFX_ByteString GetNativeTrueTypeFont(int32_t nCharset) override;
  FX_BOOL FindNativeTrueTypeFont(int32_t nCharset,
                                 CFX_ByteString sFontFaceName) override;
  CPDF_Font* AddNativeTrueTypeFontToPDF(CPDF_Document* pDoc,
                                        CFX_ByteString sFontFaceName,
                                        uint8_t nCharset) override;
  int32_t SetTimer(int32_t uElapse, TimerCallback lpTimerFunc) override;
  void KillTimer(int32_t nID) override;
  FX_BOOL IsSHIFTKeyDown(FX_DWORD nFlag) override {
    return m_pEnv->FFI_IsSHIFTKeyDown(nFlag);
  }
  FX_BOOL IsCTRLKeyDown(FX_DWORD nFlag) override {
    return m_pEnv->FFI_IsCTRLKeyDown(nFlag);
  }
  FX_BOOL IsALTKeyDown(FX_DWORD nFlag) override {
    return m_pEnv->FFI_IsALTKeyDown(nFlag);
  }
  FX_BOOL IsINSERTKeyDown(FX_DWORD nFlag) override {
    return m_pEnv->FFI_IsINSERTKeyDown(nFlag);
  }
  FX_SYSTEMTIME GetLocalTime() override;
  int32_t GetCharSet() override { return m_nCharSet; }
  void SetCharSet(int32_t nCharSet) override { m_nCharSet = nCharSet; }

 private:
  CPDFDoc_Environment* m_pEnv;
  int m_nCharSet;
};

void CFX_SystemHandler::SetCursor(int32_t nCursorType) {
  m_pEnv->FFI_SetCursor(nCursorType);
}

void CFX_SystemHandler::InvalidateRect(FX_HWND hWnd, FX_RECT rect) {
  CPDFSDK_Annot* pSDKAnnot = (CPDFSDK_Annot*)hWnd;
  CPDFSDK_PageView* pPageView = pSDKAnnot->GetPageView();
  UnderlyingPageType* pPage = pSDKAnnot->GetUnderlyingPage();
  if (!pPage || !pPageView)
    return;
  CFX_Matrix page2device;
  pPageView->GetCurrentMatrix(page2device);
  CFX_Matrix device2page;
  device2page.SetReverse(page2device);
  FX_FLOAT left, top, right, bottom;
  device2page.Transform((FX_FLOAT)rect.left, (FX_FLOAT)rect.top, left, top);
  device2page.Transform((FX_FLOAT)rect.right, (FX_FLOAT)rect.bottom, right,
                        bottom);
  CPDF_Rect rcPDF(left, bottom, right, top);
  rcPDF.Normalize();

  m_pEnv->FFI_Invalidate(pPage, rcPDF.left, rcPDF.top, rcPDF.right,
                         rcPDF.bottom);
}
void CFX_SystemHandler::OutputSelectedRect(void* pFormFiller, CPDF_Rect& rect) {
  CFFL_FormFiller* pFFL = (CFFL_FormFiller*)pFormFiller;
  if (pFFL) {
    CPDF_Point leftbottom = CPDF_Point(rect.left, rect.bottom);
    CPDF_Point righttop = CPDF_Point(rect.right, rect.top);
    CPDF_Point ptA = pFFL->PWLtoFFL(leftbottom);
    CPDF_Point ptB = pFFL->PWLtoFFL(righttop);
    CPDFSDK_Annot* pAnnot = pFFL->GetSDKAnnot();
    UnderlyingPageType* pPage = pAnnot->GetUnderlyingPage();
    ASSERT(pPage);
    m_pEnv->FFI_OutputSelectedRect(pPage, ptA.x, ptB.y, ptB.x, ptA.y);
  }
}

FX_BOOL CFX_SystemHandler::IsSelectionImplemented() {
  if (m_pEnv) {
    FPDF_FORMFILLINFO* pInfo = m_pEnv->GetFormFillInfo();
    if (pInfo && pInfo->FFI_OutputSelectedRect)
      return TRUE;
  }
  return FALSE;
}

CFX_ByteString CFX_SystemHandler::GetNativeTrueTypeFont(int32_t nCharset) {
  return "";
}

FX_BOOL CFX_SystemHandler::FindNativeTrueTypeFont(
    int32_t nCharset,
    CFX_ByteString sFontFaceName) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  if (!pFontMgr)
    return FALSE;

  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  if (!pFontMapper)
    return FALSE;

  int nSize = pFontMapper->m_InstalledTTFonts.GetSize();
  if (nSize == 0) {
    pFontMapper->LoadInstalledFonts();
    nSize = pFontMapper->m_InstalledTTFonts.GetSize();
  }

  for (int i = 0; i < nSize; ++i) {
    if (pFontMapper->m_InstalledTTFonts[i].Compare(sFontFaceName))
      return TRUE;
  }

  return FALSE;
}

static int CharSet2CP(int charset) {
  if (charset == 128)
    return 932;
  if (charset == 134)
    return 936;
  if (charset == 129)
    return 949;
  if (charset == 136)
    return 950;
  return 0;
}
CPDF_Font* CFX_SystemHandler::AddNativeTrueTypeFontToPDF(
    CPDF_Document* pDoc,
    CFX_ByteString sFontFaceName,
    uint8_t nCharset) {
  if (pDoc) {
    CFX_Font* pFXFont = new CFX_Font();
    pFXFont->LoadSubst(sFontFaceName, TRUE, 0, 0, 0, CharSet2CP(nCharset),
                       FALSE);
    CPDF_Font* pFont = pDoc->AddFont(pFXFont, nCharset, FALSE);
    delete pFXFont;
    return pFont;
  }

  return NULL;
}

int32_t CFX_SystemHandler::SetTimer(int32_t uElapse,
                                    TimerCallback lpTimerFunc) {
  return m_pEnv->FFI_SetTimer(uElapse, lpTimerFunc);
}
void CFX_SystemHandler::KillTimer(int32_t nID) {
  m_pEnv->FFI_KillTimer(nID);
}

FX_SYSTEMTIME CFX_SystemHandler::GetLocalTime() {
  return m_pEnv->FFI_GetLocalTime();
}

CPDFDoc_Environment::CPDFDoc_Environment(UnderlyingDocumentType* pDoc,
                                         FPDF_FORMFILLINFO* pFFinfo)
    : m_pInfo(pFFinfo), m_pSDKDoc(NULL), m_pUnderlyingDoc(pDoc) {
  m_pSysHandler.reset(new CFX_SystemHandler(this));
}

CPDFDoc_Environment::~CPDFDoc_Environment() {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_App* pProvider = CPDFXFA_App::GetInstance();
  if (pProvider->m_pEnvList.GetSize() == 0)
    pProvider->SetJavaScriptInitialized(FALSE);
#endif  // PDF_ENABLE_XFA
}

int CPDFDoc_Environment::JS_appAlert(const FX_WCHAR* Msg,
                                     const FX_WCHAR* Title,
                                     FX_UINT Type,
                                     FX_UINT Icon) {
  if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_alert) {
    CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
    CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
    FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());
    FPDF_WIDESTRING pTitle =
        (FPDF_WIDESTRING)bsTitle.GetBuffer(bsTitle.GetLength());
    int ret = m_pInfo->m_pJsPlatform->app_alert(m_pInfo->m_pJsPlatform, pMsg,
                                                pTitle, Type, Icon);
    bsMsg.ReleaseBuffer();
    bsTitle.ReleaseBuffer();
    return ret;
  }
  return -1;
}

int CPDFDoc_Environment::JS_appResponse(const FX_WCHAR* Question,
                                        const FX_WCHAR* Title,
                                        const FX_WCHAR* Default,
                                        const FX_WCHAR* cLabel,
                                        FPDF_BOOL bPassword,
                                        void* response,
                                        int length) {
  if (m_pInfo && m_pInfo->m_pJsPlatform &&
      m_pInfo->m_pJsPlatform->app_response) {
    CFX_ByteString bsQuestion = CFX_WideString(Question).UTF16LE_Encode();
    CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
    CFX_ByteString bsDefault = CFX_WideString(Default).UTF16LE_Encode();
    CFX_ByteString bsLabel = CFX_WideString(cLabel).UTF16LE_Encode();
    FPDF_WIDESTRING pQuestion =
        (FPDF_WIDESTRING)bsQuestion.GetBuffer(bsQuestion.GetLength());
    FPDF_WIDESTRING pTitle =
        (FPDF_WIDESTRING)bsTitle.GetBuffer(bsTitle.GetLength());
    FPDF_WIDESTRING pDefault =
        (FPDF_WIDESTRING)bsDefault.GetBuffer(bsDefault.GetLength());
    FPDF_WIDESTRING pLabel =
        (FPDF_WIDESTRING)bsLabel.GetBuffer(bsLabel.GetLength());
    int ret = m_pInfo->m_pJsPlatform->app_response(
        m_pInfo->m_pJsPlatform, pQuestion, pTitle, pDefault, pLabel, bPassword,
        response, length);
    bsQuestion.ReleaseBuffer();
    bsTitle.ReleaseBuffer();
    bsDefault.ReleaseBuffer();
    bsLabel.ReleaseBuffer();
    return ret;
  }
  return -1;
}

CFX_WideString CPDFDoc_Environment::JS_fieldBrowse() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Field_browse) {
    return L"";
  }

  const int nRequiredLen =
      m_pInfo->m_pJsPlatform->Field_browse(m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return L"";

  std::unique_ptr<char[]> pBuff(new char[nRequiredLen]);
  memset(pBuff.get(), 0, nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Field_browse(
      m_pInfo->m_pJsPlatform, pBuff.get(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return L"";

  CFX_ByteString bsRet = CFX_ByteString(pBuff.get(), nActualLen);
  CFX_WideString wsRet = CFX_WideString::FromLocal(bsRet);
  return wsRet;
}

CFX_WideString CPDFDoc_Environment::JS_docGetFilePath() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_getFilePath) {
    return L"";
  }

  const int nRequiredLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return L"";

  std::unique_ptr<char[]> pBuff(new char[nRequiredLen]);
  memset(pBuff.get(), 0, nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, pBuff.get(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return L"";

  CFX_ByteString bsRet = CFX_ByteString(pBuff.get(), nActualLen);
  CFX_WideString wsRet = CFX_WideString::FromLocal(bsRet);
  return wsRet;
}

void CPDFDoc_Environment::JS_docSubmitForm(void* formData,
                                           int length,
                                           const FX_WCHAR* URL) {
  if (m_pInfo && m_pInfo->m_pJsPlatform &&
      m_pInfo->m_pJsPlatform->Doc_submitForm) {
    CFX_ByteString bsDestination = CFX_WideString(URL).UTF16LE_Encode();
    FPDF_WIDESTRING pDestination =
        (FPDF_WIDESTRING)bsDestination.GetBuffer(bsDestination.GetLength());
    m_pInfo->m_pJsPlatform->Doc_submitForm(m_pInfo->m_pJsPlatform, formData,
                                           length, pDestination);
    bsDestination.ReleaseBuffer();
  }
}

void CPDFDoc_Environment::JS_docmailForm(void* mailData,
                                         int length,
                                         FPDF_BOOL bUI,
                                         const FX_WCHAR* To,
                                         const FX_WCHAR* Subject,
                                         const FX_WCHAR* CC,
                                         const FX_WCHAR* BCC,
                                         const FX_WCHAR* Msg) {
  if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_mail) {
    CFX_ByteString bsTo = CFX_WideString(To).UTF16LE_Encode();
    CFX_ByteString bsCC = CFX_WideString(Subject).UTF16LE_Encode();
    CFX_ByteString bsBcc = CFX_WideString(BCC).UTF16LE_Encode();
    CFX_ByteString bsSubject = CFX_WideString(Subject).UTF16LE_Encode();
    CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
    FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(bsTo.GetLength());
    FPDF_WIDESTRING pCC = (FPDF_WIDESTRING)bsCC.GetBuffer(bsCC.GetLength());
    FPDF_WIDESTRING pBcc = (FPDF_WIDESTRING)bsBcc.GetBuffer(bsBcc.GetLength());
    FPDF_WIDESTRING pSubject =
        (FPDF_WIDESTRING)bsSubject.GetBuffer(bsSubject.GetLength());
    FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());
    m_pInfo->m_pJsPlatform->Doc_mail(m_pInfo->m_pJsPlatform, mailData, length,
                                     bUI, pTo, pSubject, pCC, pBcc, pMsg);
    bsTo.ReleaseBuffer();
    bsCC.ReleaseBuffer();
    bsBcc.ReleaseBuffer();
    bsSubject.ReleaseBuffer();
    bsMsg.ReleaseBuffer();
  }
}

IJS_Runtime* CPDFDoc_Environment::GetJSRuntime() {
  if (!IsJSInitiated())
    return NULL;
  if (!m_pJSRuntime)
    m_pJSRuntime.reset(IJS_Runtime::Create(this));
  return m_pJSRuntime.get();
}

CPDFSDK_AnnotHandlerMgr* CPDFDoc_Environment::GetAnnotHandlerMgr() {
  if (!m_pAnnotHandlerMgr)
    m_pAnnotHandlerMgr.reset(new CPDFSDK_AnnotHandlerMgr(this));
  return m_pAnnotHandlerMgr.get();
}

CPDFSDK_ActionHandler* CPDFDoc_Environment::GetActionHander() {
  if (!m_pActionHandler)
    m_pActionHandler.reset(new CPDFSDK_ActionHandler());
  return m_pActionHandler.get();
}

CFFL_IFormFiller* CPDFDoc_Environment::GetIFormFiller() {
  if (!m_pIFormFiller)
    m_pIFormFiller.reset(new CFFL_IFormFiller(this));
  return m_pIFormFiller.get();
}

CPDFSDK_Document::CPDFSDK_Document(UnderlyingDocumentType* pDoc,
                                   CPDFDoc_Environment* pEnv)
    : m_pDoc(pDoc),
      m_pFocusAnnot(nullptr),
      m_pEnv(pEnv),
      m_bChangeMask(FALSE),
      m_bBeingDestroyed(FALSE) {}

CPDFSDK_Document::~CPDFSDK_Document() {
  m_bBeingDestroyed = TRUE;

  for (auto& it : m_pageMap)
    it.second->KillFocusAnnotIfNeeded();

  for (auto& it : m_pageMap)
    delete it.second;
  m_pageMap.clear();
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(
    UnderlyingPageType* pUnderlyingPage,
    FX_BOOL ReNew) {
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
      UnderlyingFromFPDFPage(m_pEnv->FFI_GetCurrentPage(m_pDoc));
  return pPage ? GetPageView(pPage, TRUE) : nullptr;
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(int nIndex) {
  UnderlyingPageType* pTempPage =
      UnderlyingFromFPDFPage(m_pEnv->FFI_GetPage(m_pDoc, nIndex));
  if (!pTempPage)
    return nullptr;

  auto it = m_pageMap.find(pTempPage);
  return it->second;
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
          jsAction, CFX_WideString::FromLocal(csJSName), this);
  }
}

FX_BOOL CPDFSDK_Document::ProcOpenAction() {
  if (!m_pDoc)
    return FALSE;

  CPDF_Dictionary* pRoot = GetPDFDocument()->GetRoot();
  if (!pRoot)
    return FALSE;

  CPDF_Object* pOpenAction = pRoot->GetDict("OpenAction");
  if (!pOpenAction)
    pOpenAction = pRoot->GetArray("OpenAction");

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

CPDF_OCContext* CPDFSDK_Document::GetOCContext() {
  if (!m_pOccontent)
    m_pOccontent.reset(new CPDF_OCContext(GetPDFDocument()));
  return m_pOccontent.get();
}

void CPDFSDK_Document::RemovePageView(UnderlyingPageType* pUnderlyingPage) {
  auto it = m_pageMap.find(pUnderlyingPage);
  if (it == m_pageMap.end())
    return;

  CPDFSDK_PageView* pPageView = it->second;
  if (pPageView->IsLocked())
    return;

  pPageView->KillFocusAnnotIfNeeded();
  delete pPageView;
  m_pageMap.erase(it);
}

UnderlyingPageType* CPDFSDK_Document::GetPage(int nIndex) {
  return UnderlyingFromFPDFPage(m_pEnv->FFI_GetPage(m_pDoc, nIndex));
}

CPDFSDK_InterForm* CPDFSDK_Document::GetInterForm() {
  if (!m_pInterForm)
    m_pInterForm.reset(new CPDFSDK_InterForm(this));
  return m_pInterForm.get();
}

void CPDFSDK_Document::UpdateAllViews(CPDFSDK_PageView* pSender,
                                      CPDFSDK_Annot* pAnnot) {
  for (const auto& it : m_pageMap) {
    CPDFSDK_PageView* pPageView = it.second;
    if (pPageView != pSender) {
      pPageView->UpdateView(pAnnot);
    }
  }
}

CPDFSDK_Annot* CPDFSDK_Document::GetFocusAnnot() {
  return m_pFocusAnnot;
}

FX_BOOL CPDFSDK_Document::SetFocusAnnot(CPDFSDK_Annot* pAnnot, FX_UINT nFlag) {
  if (m_bBeingDestroyed)
    return FALSE;

  if (m_pFocusAnnot == pAnnot)
    return TRUE;

  if (m_pFocusAnnot) {
    if (!KillFocusAnnot(nFlag))
      return FALSE;
  }

  if (!pAnnot)
    return FALSE;

#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot* pLastFocusAnnot = m_pFocusAnnot;
#endif  // PDF_ENABLE_XFA
  CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
  if (pPageView && pPageView->IsValid()) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandler = m_pEnv->GetAnnotHandlerMgr();
    if (!m_pFocusAnnot) {
#ifdef PDF_ENABLE_XFA
      if (!pAnnotHandler->Annot_OnChangeFocus(pAnnot, pLastFocusAnnot))
        return FALSE;
#endif  // PDF_ENABLE_XFA
      if (!pAnnotHandler->Annot_OnSetFocus(pAnnot, nFlag))
        return FALSE;
      if (!m_pFocusAnnot) {
        m_pFocusAnnot = pAnnot;
        return TRUE;
      }
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_Document::KillFocusAnnot(FX_UINT nFlag) {
  if (m_pFocusAnnot) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandler = m_pEnv->GetAnnotHandlerMgr();
    CPDFSDK_Annot* pFocusAnnot = m_pFocusAnnot;
    m_pFocusAnnot = nullptr;

#ifdef PDF_ENABLE_XFA
    if (!pAnnotHandler->Annot_OnChangeFocus(nullptr, pFocusAnnot))
      return FALSE;
#endif  // PDF_ENABLE_XFA

    if (pAnnotHandler->Annot_OnKillFocus(pFocusAnnot, nFlag)) {
      if (pFocusAnnot->GetType() == "Widget") {
        CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pFocusAnnot;
        int nFieldType = pWidget->GetFieldType();
        if (FIELDTYPE_TEXTFIELD == nFieldType ||
            FIELDTYPE_COMBOBOX == nFieldType) {
          m_pEnv->FFI_OnSetFieldInputFocus(NULL, NULL, 0, FALSE);
        }
      }

      if (!m_pFocusAnnot)
        return TRUE;
    } else {
      m_pFocusAnnot = pFocusAnnot;
    }
  }
  return FALSE;
}

void CPDFSDK_Document::OnCloseDocument() {
  KillFocusAnnot();
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

CPDFSDK_PageView::CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc,
                                   UnderlyingPageType* page)
    : m_page(page),
      m_pSDKDoc(pSDKDoc),
      m_CaptureWidget(nullptr),
#ifndef PDF_ENABLE_XFA
      m_bTakeOverPage(FALSE),
#endif  // PDF_ENABLE_XFA
      m_bEnterWidget(FALSE),
      m_bExitWidget(FALSE),
      m_bOnWidget(FALSE),
      m_bValid(FALSE),
      m_bLocked(FALSE) {
  CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm();
  if (pInterForm) {
    CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
#ifdef PDF_ENABLE_XFA
    if (page->GetPDFPage())
      pPDFInterForm->FixPageFields(page->GetPDFPage());
#else   // PDF_ENABLE_XFA
    pPDFInterForm->FixPageFields(page);
#endif  // PDF_ENABLE_XFA
  }
#ifndef PDF_ENABLE_XFA
  m_page->SetPrivateData((void*)m_page, (void*)this, nullptr);
#endif  // PDF_ENABLE_XFA
}

CPDFSDK_PageView::~CPDFSDK_PageView() {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray)
    pAnnotHandlerMgr->ReleaseAnnot(pAnnot);

  m_fxAnnotArray.clear();
  m_pAnnotList.reset();
#ifndef PDF_ENABLE_XFA
  m_page->RemovePrivateData((void*)m_page);
  if (m_bTakeOverPage) {
    delete m_page;
  }
#endif  // PDF_ENABLE_XFA
}

void CPDFSDK_PageView::PageView_OnDraw(CFX_RenderDevice* pDevice,
                                       CFX_Matrix* pUser2Device,
#ifdef PDF_ENABLE_XFA
                                       CPDF_RenderOptions* pOptions,
                                       const FX_RECT& pClip) {
#else
                                       CPDF_RenderOptions* pOptions) {
#endif  // PDF_ENABLE_XFA
  m_curMatrix = *pUser2Device;
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pPage = GetPDFXFAPage();
  if (!pPage)
    return;

  if (pPage->GetDocument()->GetDocType() == DOCTYPE_DYNAMIC_XFA) {
    CFX_Graphics gs;
    gs.Create(pDevice);
    CFX_RectF rectClip;
    rectClip.Set(static_cast<FX_FLOAT>(pClip.left),
                 static_cast<FX_FLOAT>(pClip.top),
                 static_cast<FX_FLOAT>(pClip.Width()),
                 static_cast<FX_FLOAT>(pClip.Height()));
    gs.SetClipRect(rectClip);
    IXFA_RenderContext* pRenderContext = XFA_RenderContext_Create();
    if (!pRenderContext)
      return;
    CXFA_RenderOptions renderOptions;
    renderOptions.m_bHighlight = TRUE;
    IXFA_PageView* xfaView = pPage->GetXFAPageView();
    pRenderContext->StartRender(xfaView, &gs, *pUser2Device, renderOptions);
    pRenderContext->DoRender();
    pRenderContext->StopRender();
    pRenderContext->Release();
    IXFA_DocView* docView = xfaView->GetDocView();
    if (!docView)
      return;
    CPDFSDK_Annot* annot = GetFocusAnnot();
    if (!annot)
      return;
    // Render the focus widget
    docView->GetWidgetHandler()->RenderWidget(annot->GetXFAWidget(), &gs,
                                              pUser2Device, FALSE);
    return;
  }
#endif  // PDF_ENABLE_XFA

  // for pdf/static xfa.
  CPDFSDK_AnnotIterator annotIterator(this, true);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    pAnnotHandlerMgr->Annot_OnDraw(this, pSDKAnnot, pDevice, pUser2Device, 0);
  }
}

const CPDF_Annot* CPDFSDK_PageView::GetPDFAnnotAtPoint(FX_FLOAT pageX,
                                                       FX_FLOAT pageY) {
  for (const CPDF_Annot* pAnnot : m_pAnnotList->All()) {
    CFX_FloatRect annotRect;
    pAnnot->GetRect(annotRect);
    if (annotRect.Contains(pageX, pageY))
      return pAnnot;
  }
  return nullptr;
}

const CPDF_Annot* CPDFSDK_PageView::GetPDFWidgetAtPoint(FX_FLOAT pageX,
                                                        FX_FLOAT pageY) {
  for (const CPDF_Annot* pAnnot : m_pAnnotList->All()) {
    if (pAnnot->GetSubType() == "Widget") {
      CFX_FloatRect annotRect;
      pAnnot->GetRect(annotRect);
      if (annotRect.Contains(pageX, pageY))
        return pAnnot;
    }
  }
  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXAnnotAtPoint(FX_FLOAT pageX,
                                                   FX_FLOAT pageY) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_AnnotIterator annotIterator(this, false);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    CPDF_Rect rc = pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
    if (rc.Contains(pageX, pageY))
      return pSDKAnnot;
  }

  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXWidgetAtPoint(FX_FLOAT pageX,
                                                    FX_FLOAT pageY) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_AnnotIterator annotIterator(this, false);
  while (CPDFSDK_Annot* pSDKAnnot = annotIterator.Next()) {
    bool bHitTest = pSDKAnnot->GetType() == "Widget";
#ifdef PDF_ENABLE_XFA
    bHitTest = bHitTest || pSDKAnnot->GetType() == FSDK_XFAWIDGET_TYPENAME;
#endif  // PDF_ENABLE_XFA
    if (bHitTest) {
      pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
      CPDF_Point point(pageX, pageY);
      if (pAnnotMgr->Annot_OnHitTest(this, pSDKAnnot, point))
        return pSDKAnnot;
    }
  }

  return nullptr;
}

void CPDFSDK_PageView::KillFocusAnnotIfNeeded() {
  // if there is a focused annot on the page, we should kill the focus first.
  if (CPDFSDK_Annot* focusedAnnot = m_pSDKDoc->GetFocusAnnot()) {
    if (pdfium::ContainsValue(m_fxAnnotArray, focusedAnnot))
      KillFocusAnnot();
  }
}

FX_BOOL CPDFSDK_PageView::Annot_HasAppearance(CPDF_Annot* pAnnot) {
  CPDF_Dictionary* pAnnotDic = pAnnot->GetAnnotDict();
  if (pAnnotDic)
    return pAnnotDic->KeyExist("AS");
  return FALSE;
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CPDF_Annot* pPDFAnnot) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_Annot* pSDKAnnot = pAnnotHandler->NewAnnot(pPDFAnnot, this);
  if (!pSDKAnnot)
    return nullptr;

  m_fxAnnotArray.push_back(pSDKAnnot);
  pAnnotHandler->Annot_OnCreate(pSDKAnnot);
  return pSDKAnnot;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(IXFA_Widget* pPDFAnnot) {
  if (!pPDFAnnot)
    return nullptr;

  CPDFSDK_Annot* pSDKAnnot = GetAnnotByXFAWidget(pPDFAnnot);
  if (pSDKAnnot)
    return pSDKAnnot;

  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
  pSDKAnnot = pAnnotHandler->NewAnnot(pPDFAnnot, this);
  if (!pSDKAnnot)
    return nullptr;

  m_fxAnnotArray.push_back(pSDKAnnot);
  return pSDKAnnot;
}
#endif  // PDF_ENABLE_XFA

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CPDF_Dictionary* pDict) {
  return pDict ? AddAnnot(pDict->GetString("Subtype"), pDict) : nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(const FX_CHAR* lpSubType,
                                          CPDF_Dictionary* pDict) {
  return NULL;
}

FX_BOOL CPDFSDK_PageView::DeleteAnnot(CPDFSDK_Annot* pAnnot) {
#ifdef PDF_ENABLE_XFA
  if (!pAnnot)
    return FALSE;
  CPDFXFA_Page* pPage = pAnnot->GetPDFXFAPage();
  if (!pPage || (pPage->GetDocument()->GetDocType() != DOCTYPE_STATIC_XFA &&
                 pPage->GetDocument()->GetDocType() != DOCTYPE_DYNAMIC_XFA))
    return FALSE;

  auto it = std::find(m_fxAnnotArray.begin(), m_fxAnnotArray.end(), pAnnot);
  if (it != m_fxAnnotArray.end())
    m_fxAnnotArray.erase(it);
  if (m_CaptureWidget == pAnnot)
    m_CaptureWidget = nullptr;

  return TRUE;
#else   // PDF_ENABLE_XFA
  return FALSE;
#endif  // PDF_ENABLE_XFA
}

CPDF_Document* CPDFSDK_PageView::GetPDFDocument() {
  if (m_page) {
#ifdef PDF_ENABLE_XFA
    return m_page->GetDocument()->GetPDFDoc();
#else   // PDF_ENABLE_XFA
    return m_page->m_pDocument;
#endif  // PDF_ENABLE_XFA
  }
  return NULL;
}

#ifdef PDF_ENABLE_XFA
CPDF_Page* CPDFSDK_PageView::GetPDFPage() {
  if (m_page) {
    return m_page->GetPDFPage();
  }
  return NULL;
}
#endif  // PDF_ENABLE_XFA

size_t CPDFSDK_PageView::CountAnnots() const {
  return m_fxAnnotArray.size();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnot(size_t nIndex) {
  return nIndex < m_fxAnnotArray.size() ? m_fxAnnotArray[nIndex] : nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByDict(CPDF_Dictionary* pDict) {
  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot->GetPDFAnnot()->GetAnnotDict() == pDict)
      return pAnnot;
  }
  return nullptr;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByXFAWidget(IXFA_Widget* hWidget) {
  if (!hWidget)
    return nullptr;

  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot->GetXFAWidget() == hWidget)
      return pAnnot;
  }
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

FX_BOOL CPDFSDK_PageView::OnLButtonDown(const CPDF_Point& point,
                                        FX_UINT nFlag) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
  if (!pFXAnnot) {
    KillFocusAnnot(nFlag);
    return FALSE;
  }

  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  FX_BOOL bRet =
      pAnnotHandlerMgr->Annot_OnLButtonDown(this, pFXAnnot, nFlag, point);
  if (bRet)
    SetFocusAnnot(pFXAnnot);
  return bRet;
}

#ifdef PDF_ENABLE_XFA
FX_BOOL CPDFSDK_PageView::OnRButtonDown(const CPDF_Point& point,
                                        FX_UINT nFlag) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  ASSERT(pAnnotHandlerMgr);

  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);

  if (pFXAnnot == NULL)
    return FALSE;

  FX_BOOL bRet =
      pAnnotHandlerMgr->Annot_OnRButtonDown(this, pFXAnnot, nFlag, point);
  if (bRet) {
    SetFocusAnnot(pFXAnnot);
  }
  return TRUE;
}

FX_BOOL CPDFSDK_PageView::OnRButtonUp(const CPDF_Point& point, FX_UINT nFlag) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();

  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);

  if (pFXAnnot == NULL)
    return FALSE;

  FX_BOOL bRet =
      pAnnotHandlerMgr->Annot_OnRButtonUp(this, pFXAnnot, nFlag, point);
  if (bRet) {
    SetFocusAnnot(pFXAnnot);
  }
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

FX_BOOL CPDFSDK_PageView::OnLButtonUp(const CPDF_Point& point, FX_UINT nFlag) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  ASSERT(pEnv);
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
  CPDFSDK_Annot* pFocusAnnot = GetFocusAnnot();
  FX_BOOL bRet = FALSE;
  if (pFocusAnnot && pFocusAnnot != pFXAnnot) {
    // Last focus Annot gets a chance to handle the event.
    bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFocusAnnot, nFlag, point);
  }
  if (pFXAnnot && !bRet)
    bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFXAnnot, nFlag, point);
  return bRet;
}

FX_BOOL CPDFSDK_PageView::OnMouseMove(const CPDF_Point& point, int nFlag) {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
  if (CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y)) {
    if (m_CaptureWidget && m_CaptureWidget != pFXAnnot) {
      m_bExitWidget = TRUE;
      m_bEnterWidget = FALSE;
      pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
    }
    m_CaptureWidget = (CPDFSDK_Widget*)pFXAnnot;
    m_bOnWidget = TRUE;
    if (!m_bEnterWidget) {
      m_bEnterWidget = TRUE;
      m_bExitWidget = FALSE;
      pAnnotHandlerMgr->Annot_OnMouseEnter(this, pFXAnnot, nFlag);
    }
    pAnnotHandlerMgr->Annot_OnMouseMove(this, pFXAnnot, nFlag, point);
    return TRUE;
  }
  if (m_bOnWidget) {
    m_bOnWidget = FALSE;
    m_bExitWidget = TRUE;
    m_bEnterWidget = FALSE;
    if (m_CaptureWidget) {
      pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
      m_CaptureWidget = NULL;
    }
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnMouseWheel(double deltaX,
                                       double deltaY,
                                       const CPDF_Point& point,
                                       int nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFXWidgetAtPoint(point.x, point.y)) {
    CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnMouseWheel(this, pAnnot, nFlag,
                                                (int)deltaY, point);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnChar(int nChar, FX_UINT nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFocusAnnot()) {
    CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnChar(pAnnot, nChar, nFlag);
  }

  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyDown(int nKeyCode, int nFlag) {
  if (CPDFSDK_Annot* pAnnot = GetFocusAnnot()) {
    CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    return pAnnotHandlerMgr->Annot_OnKeyDown(pAnnot, nKeyCode, nFlag);
  }
  return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyUp(int nKeyCode, int nFlag) {
  return FALSE;
}

void CPDFSDK_PageView::LoadFXAnnots() {
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
#ifdef PDF_ENABLE_XFA
  CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
#else
  FX_BOOL enableAPUpdate = CPDF_InterForm::UpdatingAPEnabled();
  // Disable the default AP construction.
  CPDF_InterForm::EnableUpdateAP(FALSE);
  m_pAnnotList.reset(new CPDF_AnnotList(m_page));
  CPDF_InterForm::EnableUpdateAP(enableAPUpdate);
  const size_t nCount = m_pAnnotList->Count();
#endif  // PDF_ENABLE_XFA

  SetLock(TRUE);

#ifdef PDF_ENABLE_XFA
  m_page->AddRef();
  if (m_pSDKDoc->GetXFADocument()->GetDocType() == DOCTYPE_DYNAMIC_XFA) {
    IXFA_PageView* pageView = NULL;
    pageView = m_page->GetXFAPageView();
    ASSERT(pageView != NULL);

    IXFA_WidgetIterator* pWidgetHander = pageView->CreateWidgetIterator(
        XFA_TRAVERSEWAY_Form, XFA_WIDGETFILTER_Visible |
                                  XFA_WIDGETFILTER_Viewable |
                                  XFA_WIDGETFILTER_AllType);
    if (!pWidgetHander) {
      m_page->Release();
      SetLock(FALSE);
      return;
    }

    while (IXFA_Widget* pXFAAnnot = pWidgetHander->MoveToNext()) {
      CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pXFAAnnot, this);
      if (!pAnnot)
        continue;

      m_fxAnnotArray.push_back(pAnnot);
      pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
    }
    pWidgetHander->Release();
  } else {
    CPDF_Page* pPage = m_page->GetPDFPage();
    ASSERT(pPage != NULL);
    FX_BOOL enableAPUpdate = CPDF_InterForm::UpdatingAPEnabled();
    // Disable the default AP construction.
    CPDF_InterForm::EnableUpdateAP(FALSE);
    m_pAnnotList.reset(new CPDF_AnnotList(pPage));
    CPDF_InterForm::EnableUpdateAP(enableAPUpdate);

    const size_t nCount = m_pAnnotList->Count();
    for (size_t i = 0; i < nCount; ++i) {
      CPDF_Annot* pPDFAnnot = m_pAnnotList->GetAt(i);
      CheckUnSupportAnnot(GetPDFDocument(), pPDFAnnot);

      CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pPDFAnnot, this);
      if (!pAnnot)
        continue;
      m_fxAnnotArray.push_back(pAnnot);
      pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
    }
  }
  m_page->Release();
#else   // PDF_ENABLE_XFA
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_Annot* pPDFAnnot = m_pAnnotList->GetAt(i);
    CPDF_Document* pDoc = GetPDFDocument();
    CheckUnSupportAnnot(pDoc, pPDFAnnot);
    CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
    CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pPDFAnnot, this);
    if (!pAnnot)
      continue;
    m_fxAnnotArray.push_back(pAnnot);
    pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
  }
#endif  // PDF_ENABLE_XFA

  SetLock(FALSE);
}

void CPDFSDK_PageView::UpdateRects(CFX_RectArray& rects) {
  for (int i = 0; i < rects.GetSize(); i++) {
    CPDF_Rect rc = rects.GetAt(i);
    CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
    pEnv->FFI_Invalidate(m_page, rc.left, rc.top, rc.right, rc.bottom);
  }
}

void CPDFSDK_PageView::UpdateView(CPDFSDK_Annot* pAnnot) {
  CPDF_Rect rcWindow = pAnnot->GetRect();
  CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
  pEnv->FFI_Invalidate(m_page, rcWindow.left, rcWindow.top, rcWindow.right,
                       rcWindow.bottom);
}

int CPDFSDK_PageView::GetPageIndex() {
  if (m_page) {
#ifdef PDF_ENABLE_XFA
    CPDF_Dictionary* pDic = m_page->GetPDFPage()->m_pFormDict;
#else   // PDF_ENABLE_XFA
    CPDF_Dictionary* pDic = m_page->m_pFormDict;
#endif  // PDF_ENABLE_XFA
    CPDF_Document* pDoc = m_pSDKDoc->GetPDFDocument();
    if (pDoc && pDic) {
      return pDoc->GetPageIndex(pDic->GetObjNum());
    }
  }
  return -1;
}

bool CPDFSDK_PageView::IsValidAnnot(const CPDF_Annot* p) const {
  if (!p)
    return false;

  const auto& annots = m_pAnnotList->All();
  return pdfium::ContainsValue(annots, p);
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFocusAnnot() {
  CPDFSDK_Annot* pFocusAnnot = m_pSDKDoc->GetFocusAnnot();
  if (!pFocusAnnot)
    return nullptr;

  for (CPDFSDK_Annot* pAnnot : m_fxAnnotArray) {
    if (pAnnot == pFocusAnnot)
      return pAnnot;
  }
  return nullptr;
}
