// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/cpdfsdk_environment.h"

#include "fpdfsdk/formfiller/cffl_iformfiller.h"
#include "fpdfsdk/include/cpdfsdk_annothandlermgr.h"
#include "fpdfsdk/include/fsdk_actionhandler.h"
#include "fpdfsdk/javascript/ijs_runtime.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/include/fpdfxfa_app.h"
#endif  // PDF_ENABLE_XFA

namespace {

// NOTE: |bsUTF16LE| must outlive the use of the result. Care must be taken
// since modifying the result would impact |bsUTF16LE|.
FPDF_WIDESTRING AsFPDFWideString(CFX_ByteString* bsUTF16LE) {
  return reinterpret_cast<FPDF_WIDESTRING>(
      bsUTF16LE->GetBuffer(bsUTF16LE->GetLength()));
}

}  // namespace

CPDFSDK_Environment::CPDFSDK_Environment(UnderlyingDocumentType* pDoc,
                                         FPDF_FORMFILLINFO* pFFinfo)
    : m_pInfo(pFFinfo), m_pSDKDoc(nullptr), m_pUnderlyingDoc(pDoc) {
  m_pSysHandler.reset(new CFX_SystemHandler(this));
}

CPDFSDK_Environment::~CPDFSDK_Environment() {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_App* pProvider = CPDFXFA_App::GetInstance();
  if (pProvider->m_pEnvList.GetSize() == 0)
    pProvider->SetJavaScriptInitialized(FALSE);
#endif  // PDF_ENABLE_XFA
  if (m_pInfo && m_pInfo->Release)
    m_pInfo->Release(m_pInfo);
}

int CPDFSDK_Environment::JS_appAlert(const FX_WCHAR* Msg,
                                     const FX_WCHAR* Title,
                                     uint32_t Type,
                                     uint32_t Icon) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_alert) {
    return -1;
  }
  CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
  CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
  return m_pInfo->m_pJsPlatform->app_alert(
      m_pInfo->m_pJsPlatform, AsFPDFWideString(&bsMsg),
      AsFPDFWideString(&bsTitle), Type, Icon);
}

int CPDFSDK_Environment::JS_appResponse(const FX_WCHAR* Question,
                                        const FX_WCHAR* Title,
                                        const FX_WCHAR* Default,
                                        const FX_WCHAR* cLabel,
                                        FPDF_BOOL bPassword,
                                        void* response,
                                        int length) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_response) {
    return -1;
  }
  CFX_ByteString bsQuestion = CFX_WideString(Question).UTF16LE_Encode();
  CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
  CFX_ByteString bsDefault = CFX_WideString(Default).UTF16LE_Encode();
  CFX_ByteString bsLabel = CFX_WideString(cLabel).UTF16LE_Encode();
  return m_pInfo->m_pJsPlatform->app_response(
      m_pInfo->m_pJsPlatform, AsFPDFWideString(&bsQuestion),
      AsFPDFWideString(&bsTitle), AsFPDFWideString(&bsDefault),
      AsFPDFWideString(&bsLabel), bPassword, response, length);
}

void CPDFSDK_Environment::JS_appBeep(int nType) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->app_beep) {
    return;
  }
  m_pInfo->m_pJsPlatform->app_beep(m_pInfo->m_pJsPlatform, nType);
}

CFX_WideString CPDFSDK_Environment::JS_fieldBrowse() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Field_browse) {
    return CFX_WideString();
  }
  const int nRequiredLen =
      m_pInfo->m_pJsPlatform->Field_browse(m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return CFX_WideString();

  std::unique_ptr<char[]> pBuff(new char[nRequiredLen]);
  memset(pBuff.get(), 0, nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Field_browse(
      m_pInfo->m_pJsPlatform, pBuff.get(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return CFX_WideString();

  return CFX_WideString::FromLocal(CFX_ByteStringC(pBuff.get(), nActualLen));
}

CFX_WideString CPDFSDK_Environment::JS_docGetFilePath() {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_getFilePath) {
    return CFX_WideString();
  }
  const int nRequiredLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, nullptr, 0);
  if (nRequiredLen <= 0)
    return CFX_WideString();

  std::unique_ptr<char[]> pBuff(new char[nRequiredLen]);
  memset(pBuff.get(), 0, nRequiredLen);
  const int nActualLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(
      m_pInfo->m_pJsPlatform, pBuff.get(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen)
    return CFX_WideString();

  return CFX_WideString::FromLocal(CFX_ByteStringC(pBuff.get(), nActualLen));
}

void CPDFSDK_Environment::JS_docSubmitForm(void* formData,
                                           int length,
                                           const FX_WCHAR* URL) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_submitForm) {
    return;
  }
  CFX_ByteString bsDestination = CFX_WideString(URL).UTF16LE_Encode();
  m_pInfo->m_pJsPlatform->Doc_submitForm(m_pInfo->m_pJsPlatform, formData,
                                         length,
                                         AsFPDFWideString(&bsDestination));
}

void CPDFSDK_Environment::JS_docmailForm(void* mailData,
                                         int length,
                                         FPDF_BOOL bUI,
                                         const FX_WCHAR* To,
                                         const FX_WCHAR* Subject,
                                         const FX_WCHAR* CC,
                                         const FX_WCHAR* BCC,
                                         const FX_WCHAR* Msg) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_mail) {
    return;
  }
  CFX_ByteString bsTo = CFX_WideString(To).UTF16LE_Encode();
  CFX_ByteString bsSubject = CFX_WideString(Subject).UTF16LE_Encode();
  CFX_ByteString bsCC = CFX_WideString(CC).UTF16LE_Encode();
  CFX_ByteString bsBcc = CFX_WideString(BCC).UTF16LE_Encode();
  CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
  m_pInfo->m_pJsPlatform->Doc_mail(
      m_pInfo->m_pJsPlatform, mailData, length, bUI, AsFPDFWideString(&bsTo),
      AsFPDFWideString(&bsSubject), AsFPDFWideString(&bsCC),
      AsFPDFWideString(&bsBcc), AsFPDFWideString(&bsMsg));
}

void CPDFSDK_Environment::JS_docprint(FPDF_BOOL bUI,
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

void CPDFSDK_Environment::JS_docgotoPage(int nPageNum) {
  if (!m_pInfo || !m_pInfo->m_pJsPlatform ||
      !m_pInfo->m_pJsPlatform->Doc_gotoPage) {
    return;
  }
  m_pInfo->m_pJsPlatform->Doc_gotoPage(m_pInfo->m_pJsPlatform, nPageNum);
}

IJS_Runtime* CPDFSDK_Environment::GetJSRuntime() {
  if (!IsJSInitiated())
    return nullptr;
  if (!m_pJSRuntime)
    m_pJSRuntime.reset(IJS_Runtime::Create(this));
  return m_pJSRuntime.get();
}

CPDFSDK_AnnotHandlerMgr* CPDFSDK_Environment::GetAnnotHandlerMgr() {
  if (!m_pAnnotHandlerMgr)
    m_pAnnotHandlerMgr.reset(new CPDFSDK_AnnotHandlerMgr(this));
  return m_pAnnotHandlerMgr.get();
}

CPDFSDK_ActionHandler* CPDFSDK_Environment::GetActionHander() {
  if (!m_pActionHandler)
    m_pActionHandler.reset(new CPDFSDK_ActionHandler());
  return m_pActionHandler.get();
}

CFFL_IFormFiller* CPDFSDK_Environment::GetIFormFiller() {
  if (!m_pIFormFiller)
    m_pIFormFiller.reset(new CFFL_IFormFiller(this));
  return m_pIFormFiller.get();
}
