// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_mgr.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_doc.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"
#include "fpdfsdk/include/javascript/IJavaScript.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_app.h"
#include "public/fpdf_formfill.h"

CPDFXFA_App* CPDFXFA_App::g_pApp = NULL;

CPDFXFA_App* CPDFXFA_App::GetInstance() {
  if (!g_pApp) {
    g_pApp = new CPDFXFA_App();
  }
  return g_pApp;
}

void CPDFXFA_App::ReleaseInstance() {
  delete g_pApp;
  g_pApp = NULL;
}

CPDFXFA_App::CPDFXFA_App()
    : m_bJavaScriptInitialized(FALSE),
      m_pXFAApp(NULL),
      m_pFontMgr(NULL),
      m_hJSERuntime(NULL),
      m_csAppType(JS_STR_VIEWERTYPE_STANDARD) {
  m_pEnvList.RemoveAll();
}

CPDFXFA_App::~CPDFXFA_App() {
  delete m_pFontMgr;
  m_pFontMgr = NULL;

  delete m_pXFAApp;
  m_pXFAApp = NULL;

#ifdef PDF_ENABLE_XFA
  FXJSE_Runtime_Release(m_hJSERuntime);
  m_hJSERuntime = NULL;

  FXJSE_Finalize();
  BC_Library_Destory();
#endif
}

FX_BOOL CPDFXFA_App::Initialize() {
#ifdef PDF_ENABLE_XFA
  BC_Library_Init();
  FXJSE_Initialize();

  m_hJSERuntime = FXJSE_Runtime_Create();
  if (!m_hJSERuntime)
    return FALSE;

  m_pXFAApp = IXFA_App::Create(this);
  if (!m_pXFAApp)
    return FALSE;

  m_pFontMgr = IXFA_FontMgr::CreateDefault();
  if (!m_pFontMgr)
    return FALSE;

  m_pXFAApp->SetDefaultFontMgr(m_pFontMgr);
#endif
  return TRUE;
}

FX_BOOL CPDFXFA_App::AddFormFillEnv(CPDFDoc_Environment* pEnv) {
  if (!pEnv)
    return FALSE;

  m_pEnvList.Add(pEnv);
  return TRUE;
}

FX_BOOL CPDFXFA_App::RemoveFormFillEnv(CPDFDoc_Environment* pEnv) {
  if (!pEnv)
    return FALSE;

  int nFind = m_pEnvList.Find(pEnv);
  if (nFind != -1) {
    m_pEnvList.RemoveAt(nFind);
    return TRUE;
  }

  return FALSE;
}

void CPDFXFA_App::GetAppType(CFX_WideString& wsAppType) {
  wsAppType = m_csAppType;
}

void CPDFXFA_App::GetAppName(CFX_WideString& wsName) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    wsName = pEnv->FFI_GetAppName();
  }
}

void CPDFXFA_App::SetAppType(const CFX_WideStringC& wsAppType) {
  m_csAppType = wsAppType;
}

void CPDFXFA_App::GetLanguage(CFX_WideString& wsLanguage) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    wsLanguage = pEnv->FFI_GetLanguage();
  }
}

void CPDFXFA_App::GetPlatform(CFX_WideString& wsPlatform) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    wsPlatform = pEnv->FFI_GetPlatform();
  }
}

void CPDFXFA_App::GetVariation(CFX_WideString& wsVariation) {
  wsVariation = JS_STR_VIEWERVARIATION;
}

void CPDFXFA_App::GetVersion(CFX_WideString& wsVersion) {
  wsVersion = JS_STR_VIEWERVERSION_XFA;
}

void CPDFXFA_App::Beep(FX_DWORD dwType) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    pEnv->JS_appBeep(dwType);
  }
}

int32_t CPDFXFA_App::MsgBox(const CFX_WideStringC& wsMessage,
                            const CFX_WideStringC& wsTitle,
                            FX_DWORD dwIconType,
                            FX_DWORD dwButtonType) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (!pEnv)
    return -1;

  FX_DWORD iconType = 0;
  int iButtonType = 0;
  switch (dwIconType) {
    case XFA_MBICON_Error:
      iconType |= 0;
      break;
    case XFA_MBICON_Warning:
      iconType |= 1;
      break;
    case XFA_MBICON_Question:
      iconType |= 2;
      break;
    case XFA_MBICON_Status:
      iconType |= 3;
      break;
  }
  switch (dwButtonType) {
    case XFA_MB_OK:
      iButtonType |= 0;
      break;
    case XFA_MB_OKCancel:
      iButtonType |= 1;
      break;
    case XFA_MB_YesNo:
      iButtonType |= 2;
      break;
    case XFA_MB_YesNoCancel:
      iButtonType |= 3;
      break;
  }
  int32_t iRet = pEnv->JS_appAlert(wsMessage.GetPtr(), wsTitle.GetPtr(),
                                   iButtonType, iconType);
  switch (iRet) {
    case 1:
      return XFA_IDOK;
    case 2:
      return XFA_IDCancel;
    case 3:
      return XFA_IDNo;
    case 4:
      return XFA_IDYes;
  }
  return XFA_IDYes;
}

void CPDFXFA_App::Response(CFX_WideString& wsAnswer,
                           const CFX_WideStringC& wsQuestion,
                           const CFX_WideStringC& wsTitle,
                           const CFX_WideStringC& wsDefaultAnswer,
                           FX_BOOL bMark) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    int nLength = 2048;
    char* pBuff = new char[nLength];
    nLength = pEnv->JS_appResponse(wsQuestion.GetPtr(), wsTitle.GetPtr(),
                                   wsDefaultAnswer.GetPtr(), NULL, bMark, pBuff,
                                   nLength);
    if (nLength > 0) {
      nLength = nLength > 2046 ? 2046 : nLength;
      pBuff[nLength] = 0;
      pBuff[nLength + 1] = 0;
      wsAnswer = CFX_WideString::FromUTF16LE(
          reinterpret_cast<const unsigned short*>(pBuff),
          nLength / sizeof(unsigned short));
    }
    delete[] pBuff;
  }
}

int32_t CPDFXFA_App::GetCurDocumentInBatch() {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    return pEnv->FFI_GetCurDocument();
  }
  return 0;
}

int32_t CPDFXFA_App::GetDocumentCountInBatch() {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    return pEnv->FFI_GetDocumentCount();
  }

  return 0;
}

IFX_FileRead* CPDFXFA_App::DownloadURL(const CFX_WideStringC& wsURL) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    return pEnv->FFI_DownloadFromURL(wsURL.GetPtr());
  }
  return NULL;
}

FX_BOOL CPDFXFA_App::PostRequestURL(const CFX_WideStringC& wsURL,
                                    const CFX_WideStringC& wsData,
                                    const CFX_WideStringC& wsContentType,
                                    const CFX_WideStringC& wsEncode,
                                    const CFX_WideStringC& wsHeader,
                                    CFX_WideString& wsResponse) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    wsResponse = pEnv->FFI_PostRequestURL(wsURL.GetPtr(), wsData.GetPtr(),
                                          wsContentType.GetPtr(),
                                          wsEncode.GetPtr(), wsHeader.GetPtr());
    return TRUE;
  }
  return FALSE;
}

FX_BOOL CPDFXFA_App::PutRequestURL(const CFX_WideStringC& wsURL,
                                   const CFX_WideStringC& wsData,
                                   const CFX_WideStringC& wsEncode) {
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv) {
    return pEnv->FFI_PutRequestURL(wsURL.GetPtr(), wsData.GetPtr(),
                                   wsEncode.GetPtr());
  }
  return FALSE;
}

void CPDFXFA_App::LoadString(int32_t iStringID, CFX_WideString& wsString) {
  switch (iStringID) {
    case XFA_IDS_ValidateFailed:
      wsString = L"%s validate failed";
      return;
    case XFA_IDS_CalcOverride:
      wsString = L"Calculate Override";
      return;
    case XFA_IDS_ModifyField:
      wsString = L"Are you sure you want to modify this field?";
      return;
    case XFA_IDS_NotModifyField:
      wsString = L"You are not allowed to modify this field.";
      return;
    case XFA_IDS_AppName:
      wsString = L"Foxit";
      return;
    case XFA_IDS_ImageFilter:
      wsString =
          L"Image "
          L"Files(*.bmp;*.jpg;*.png;*.gif;*.tif)|*.bmp;*.jpg;*.png;*.gif;*.tif|"
          L"All Files(*.*)|*.*||";
      return;
    case XFA_IDS_UNKNOW_CATCHED:
      wsString = L"unknown error is catched!";
      return;
    case XFA_IDS_Unable_TO_SET:
      wsString = L"Unable to set ";
      return;
    case XFA_IDS_VALUE_EXCALMATORY:
      wsString = L" value!";
      return;
    case XFA_IDS_INVALID_ENUM_VALUE:
      wsString = L"Invalid enumerated value: ";
      return;
    case XFA_IDS_UNSUPPORT_METHOD:
      wsString = L"unsupport %s method.";
      return;
    case XFA_IDS_UNSUPPORT_PROP:
      wsString = L"unsupport %s property.";
      return;
    case XFA_IDS_INVAlID_PROP_SET:
      wsString = L"Invalid property set operation;";
      return;
    case XFA_IDS_NOT_DEFAUL_VALUE:
      wsString = L" doesn't have a default property";
      return;
    case XFA_IDS_UNABLE_SET_LANGUAGE:
      wsString = L"Unable to set language value!";
      return;
    case XFA_IDS_UNABLE_SET_NUMPAGES:
      wsString = L"Unable to set numPages value!";
      return;
    case XFA_IDS_UNABLE_SET_PLATFORM:
      wsString = L"Unable to set platform value!";
      return;
    case XFA_IDS_UNABLE_SET_VALIDATIONENABLE:
      wsString = L"Unable to set validationsEnabled value!";
      return;
    case XFA_IDS_UNABLE_SET_VARIATION:
      wsString = L"Unable to set variation value!";
      return;
    case XFA_IDS_UNABLE_SET_VERSION:
      wsString = L"Unable to set version value!";
      return;
    case XFA_IDS_UNABLE_SET_READY:
      wsString = L"Unable to set ready value!";
      return;
    case XFA_IDS_NUMBER_OF_OCCUR:
      wsString =
          L"The element [%s] has violated its allowable number of occurrences";
      return;
    case XFA_IDS_UNABLE_SET_CLASS_NAME:
      wsString = L"Unable to set className value!";
      return;
    case XFA_IDS_UNABLE_SET_LENGTH_VALUE:
      wsString = L"Unable to set length value!";
      return;
    case XFA_IDS_UNSUPPORT_CHAR:
      wsString = L"unsupported char '%c'";
      return;
    case XFA_IDS_BAD_SUFFIX:
      wsString = L"bad suffix on number";
      return;
    case XFA_IDS_EXPECTED_IDENT:
      wsString = L"expected identifier instead of '%s'";
      return;
    case XFA_IDS_EXPECTED_STRING:
      wsString = L"expected '%s' instead of '%s'";
      return;
    case XFA_IDS_INVALIDATE_CHAR:
      wsString = L"invalidate char '%c'";
      return;
    case XFA_IDS_REDEFINITION:
      wsString = L"'%s' redefinition ";
      return;
    case XFA_IDS_INVALIDATE_TOKEN:
      wsString = L"invalidate token '%s'";
      return;
    case XFA_IDS_INVALIDATE_EXPRESSION:
      wsString = L"invalidate expression '%s'";
      return;
    case XFA_IDS_UNDEFINE_IDENTIFIER:
      wsString = L"undefined identifier '%s'";
      return;
    case XFA_IDS_INVALIDATE_LEFTVALUE:
      wsString = L"invalidate left-value '%s'";
      return;
    case XFA_IDS_COMPILER_ERROR:
      wsString = L"compiler error";
      return;
    case XFA_IDS_CANNOT_MODIFY_VALUE:
      wsString = L"can't modify the '%s' value";
      return;
    case XFA_IDS_ERROR_PARAMETERS:
      wsString = L"function '%s' has not %d parameters";
      return;
    case XFA_IDS_EXPECT_ENDIF:
      wsString = L"expected 'endif' instead of '%s'";
      return;
    case XFA_IDS_UNEXPECTED_EXPRESSION:
      wsString = L"unexpected expression '%s'";
      return;
    case XFA_IDS_CONDITION_IS_NULL:
      wsString = L"condition is null";
      return;
    case XFA_IDS_ILLEGALBREAK:
      wsString = L"illegal break";
      return;
    case XFA_IDS_ILLEGALCONTINUE:
      wsString = L"illegal continue";
      return;
    case XFA_IDS_EXPECTED_OPERATOR:
      wsString = L"expected operator '%s' instead of '%s'";
      return;
    case XFA_IDS_DIVIDE_ZERO:
      wsString = L"divide by zero";
      return;
    case XFA_IDS_CANNOT_COVERT_OBJECT:
      wsString = L"%s.%s can not covert to object";
      return;
    case XFA_IDS_NOT_FOUND_CONTAINER:
      wsString = L"can not found container '%s'";
      return;
    case XFA_IDS_NOT_FOUND_PROPERTY:
      wsString = L"can not found property '%s'";
      return;
    case XFA_IDS_NOT_FOUND_METHOD:
      wsString = L"can not found method '%s'";
      return;
    case XFA_IDS_NOT_FOUND_CONST:
      wsString = L"can not found const '%s'";
      return;
    case XFA_IDS_NOT_ASSIGN_OBJECT:
      wsString = L"can not direct assign value to object";
      return;
    case XFA_IDS_IVALIDATE_INSTRUCTION:
      wsString = L"invalidate instruction";
      return;
    case XFA_IDS_EXPECT_NUMBER:
      wsString = L"expected number instead of '%s'";
      return;
    case XFA_IDS_VALIDATE_OUT_ARRAY:
      wsString = L"validate access index '%s' out of array";
      return;
    case XFA_IDS_CANNOT_ASSIGN_IDENT:
      wsString = L"can not assign to %s";
      return;
    case XFA_IDS_NOT_FOUNT_FUNCTION:
      wsString = L"can not found '%s' function";
      return;
    case XFA_IDS_NOT_ARRAY:
      wsString = L"'%s' doesn't an array";
      return;
    case XFA_IDS_OUT_ARRAY:
      wsString = L"out of range of '%s' array";
      return;
    case XFA_IDS_NOT_SUPPORT_CALC:
      wsString = L"'%s' operator can not support array calculate";
      return;
    case XFA_IDS_ARGUMENT_NOT_ARRAY:
      wsString = L"'%s' function's %d argument can not be array";
      return;
    case XFA_IDS_ARGUMENT_EXPECT_CONTAINER:
      wsString = L"'%s' argument expected a container";
      return;
    case XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT:
      wsString =
          L"an attempt was made to reference property '%s' of a non-object in "
          L"SOM expression %s";
      return;
    case XFA_IDS_FUNCTION_IS_BUILDIN:
      wsString = L"function '%s' is buildin";
      return;
    case XFA_IDS_ERROR_MSG:
      wsString = L"%s : %s";
      return;
    case XFA_IDS_INDEX_OUT_OF_BOUNDS:
      wsString = L"Index value is out of bounds";
      return;
    case XFA_IDS_INCORRECT_NUMBER_OF_METHOD:
      wsString = L"Incorrect number of parameters calling method '%s'";
      return;
    case XFA_IDS_ARGUMENT_MISMATCH:
      wsString = L"Argument mismatch in property or function argument";
      return;
    case XFA_IDS_INVALID_ENUMERATE:
      wsString = L"Invalid enumerated value: %s";
      return;
    case XFA_IDS_INVALID_APPEND:
      wsString =
          L"Invalid append operation: %s cannot have a child element of %s";
      return;
    case XFA_IDS_SOM_EXPECTED_LIST:
      wsString =
          L"SOM expression returned list when single result was expected";
      return;
    case XFA_IDS_NOT_HAVE_PROPERTY:
      wsString = L"'%s' doesn't have property '%s'";
      return;
    case XFA_IDS_INVALID_NODE_TYPE:
      wsString = L"Invalid node type : '%s'";
      return;
    case XFA_IDS_VIOLATE_BOUNDARY:
      wsString =
          L"The element [%s] has violated its allowable number of occurrences";
      return;
    case XFA_IDS_SERVER_DENY:
      wsString = L"Server does not permit";
      return;
    case XFA_IDS_ValidateLimit:
      wsString = FX_WSTRC(
          L"Message limit exceeded. Remaining %d validation errors not "
          L"reported.");
      return;
    case XFA_IDS_ValidateNullWarning:
      wsString = FX_WSTRC(
          L"%s cannot be left blank. To ignore validations for %s, click "
          L"Ignore.");
      return;
    case XFA_IDS_ValidateNullError:
      wsString = FX_WSTRC(L"%s cannot be left blank.");
      return;
    case XFA_IDS_ValidateWarning:
      wsString = FX_WSTRC(
          L"The value you entered for %s is invalid. To ignore validations for "
          L"%s, click Ignore.");
      return;
    case XFA_IDS_ValidateError:
      wsString = FX_WSTRC(L"The value you entered for %s is invalid.");
      return;
  }
}

FX_BOOL CPDFXFA_App::ShowFileDialog(const CFX_WideStringC& wsTitle,
                                    const CFX_WideStringC& wsFilter,
                                    CFX_WideStringArray& wsPathArr,
                                    FX_BOOL bOpen) {
  return FALSE;
}

IFWL_AdapterTimerMgr* CPDFXFA_App::GetTimerMgr() {
  CXFA_FWLAdapterTimerMgr* pAdapter = NULL;
  CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
  if (pEnv)
    pAdapter = new CXFA_FWLAdapterTimerMgr(pEnv);
  return pAdapter;
}
