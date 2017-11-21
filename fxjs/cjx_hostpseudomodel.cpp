// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_hostpseudomodel.h"

#include <memory>

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

namespace {

CXFA_Node* ToNode(CFXJSE_Value* pValue, CFXJSE_Class* pClass) {
  return static_cast<CXFA_Node*>(pValue->ToHostObject(pClass));
}

int32_t FilterName(const WideStringView& wsExpression,
                   int32_t nStart,
                   WideString& wsFilter) {
  ASSERT(nStart > -1);
  int32_t iLength = wsExpression.GetLength();
  if (nStart >= iLength)
    return iLength;

  wchar_t* pBuf = wsFilter.GetBuffer(iLength - nStart);
  int32_t nCount = 0;
  const wchar_t* pSrc = wsExpression.unterminated_c_str();
  wchar_t wCur;
  while (nStart < iLength) {
    wCur = pSrc[nStart++];
    if (wCur == ',')
      break;

    pBuf[nCount++] = wCur;
  }
  wsFilter.ReleaseBuffer(nCount);
  wsFilter.Trim();
  return nStart;
}

}  // namespace

CJX_HostPseudoModel::CJX_HostPseudoModel(CScript_HostPseudoModel* model)
    : CJX_Object(model) {}

CJX_HostPseudoModel::~CJX_HostPseudoModel() {}

void CJX_HostPseudoModel::AppType(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString("Exchange");
}

void CJX_HostPseudoModel::CalculationsEnabled(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  if (bSetting) {
    pNotify->GetDocEnvironment()->SetCalculationsEnabled(hDoc,
                                                         pValue->ToBoolean());
    return;
  }
  pValue->SetBoolean(pNotify->GetDocEnvironment()->IsCalculationsEnabled(hDoc));
}

void CJX_HostPseudoModel::CurrentPage(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  if (bSetting) {
    pNotify->GetDocEnvironment()->SetCurrentPage(hDoc, pValue->ToInteger());
    return;
  }
  pValue->SetInteger(pNotify->GetDocEnvironment()->GetCurrentPage(hDoc));
}

void CJX_HostPseudoModel::Language(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(L"Unable to set language value.");
    return;
  }
  pValue->SetString(
      pNotify->GetAppProvider()->GetLanguage().UTF8Encode().AsStringView());
}

void CJX_HostPseudoModel::NumPages(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  if (bSetting) {
    ThrowException(L"Unable to set numPages value.");
    return;
  }
  pValue->SetInteger(pNotify->GetDocEnvironment()->CountPages(hDoc));
}

void CJX_HostPseudoModel::Platform(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(L"Unable to set platform value.");
    return;
  }
  pValue->SetString(
      pNotify->GetAppProvider()->GetPlatform().UTF8Encode().AsStringView());
}

void CJX_HostPseudoModel::Title(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  if (bSetting) {
    pNotify->GetDocEnvironment()->SetTitle(hDoc, pValue->ToWideString());
    return;
  }

  WideString wsTitle;
  pNotify->GetDocEnvironment()->GetTitle(hDoc, wsTitle);
  pValue->SetString(wsTitle.UTF8Encode().AsStringView());
}

void CJX_HostPseudoModel::ValidationsEnabled(CFXJSE_Value* pValue,
                                             bool bSetting,
                                             XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  if (bSetting) {
    pNotify->GetDocEnvironment()->SetValidationsEnabled(hDoc,
                                                        pValue->ToBoolean());
    return;
  }

  bool bEnabled = pNotify->GetDocEnvironment()->IsValidationsEnabled(hDoc);
  pValue->SetBoolean(bEnabled);
}

void CJX_HostPseudoModel::Variation(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(L"Unable to set variation value.");
    return;
  }
  pValue->SetString("Full");
}

void CJX_HostPseudoModel::Version(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(L"Unable to set version value.");
    return;
  }
  pValue->SetString("11");
}

void CJX_HostPseudoModel::Name(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(
      pNotify->GetAppProvider()->GetAppName().UTF8Encode().AsStringView());
}

void CJX_HostPseudoModel::GotoURL(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"gotoURL");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  WideString wsURL;
  if (iLength >= 1) {
    ByteString bsURL = pArguments->GetUTF8String(0);
    wsURL = WideString::FromUTF8(bsURL.AsStringView());
  }
  pNotify->GetDocEnvironment()->GotoURL(hDoc, wsURL);
}

void CJX_HostPseudoModel::OpenList(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"openList");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pNode = nullptr;
  if (iLength >= 1) {
    std::unique_ptr<CFXJSE_Value> pValue(pArguments->GetValue(0));
    if (pValue->IsObject()) {
      pNode = ToNode(pValue.get(), nullptr);
    } else if (pValue->IsString()) {
      CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
      if (!pScriptContext)
        return;

      CXFA_Object* pObject = pScriptContext->GetThisObject();
      if (!pObject)
        return;

      uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Parent |
                        XFA_RESOLVENODE_Siblings;
      XFA_RESOLVENODE_RS resolveNodeRS;
      int32_t iRet = pScriptContext->ResolveObjects(
          pObject, pValue->ToWideString().AsStringView(), resolveNodeRS,
          dwFlag);
      if (iRet < 1 || !resolveNodeRS.objects.front()->IsNode())
        return;

      pNode = resolveNodeRS.objects.front()->AsNode();
    }
  }

  CXFA_LayoutProcessor* pDocLayout = GetDocument()->GetDocLayout();
  if (!pDocLayout)
    return;

  CXFA_FFWidget* hWidget =
      pNotify->GetHWidget(pDocLayout->GetLayoutItem(pNode));
  if (!hWidget)
    return;

  pNotify->GetDocEnvironment()->SetFocusWidget(pNotify->GetHDOC(), hWidget);
  pNotify->OpenDropDownList(hWidget);
}

void CJX_HostPseudoModel::Response(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 4) {
    ThrowParamCountMismatchException(L"response");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  WideString wsQuestion;
  WideString wsTitle;
  WideString wsDefaultAnswer;
  bool bMark = false;
  if (iLength >= 1) {
    ByteString bsQuestion = pArguments->GetUTF8String(0);
    wsQuestion = WideString::FromUTF8(bsQuestion.AsStringView());
  }
  if (iLength >= 2) {
    ByteString bsTitle = pArguments->GetUTF8String(1);
    wsTitle = WideString::FromUTF8(bsTitle.AsStringView());
  }
  if (iLength >= 3) {
    ByteString bsDefaultAnswer = pArguments->GetUTF8String(2);
    wsDefaultAnswer = WideString::FromUTF8(bsDefaultAnswer.AsStringView());
  }
  if (iLength >= 4) {
    bMark = pArguments->GetInt32(3) == 0 ? false : true;
  }

  WideString wsAnswer = pNotify->GetAppProvider()->Response(
      wsQuestion, wsTitle, wsDefaultAnswer, bMark);
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetString(wsAnswer.UTF8Encode().AsStringView());
}

void CJX_HostPseudoModel::DocumentInBatch(CFXJSE_Arguments* pArguments) {
  if (CFXJSE_Value* pValue = pArguments->GetReturnValue())
    pValue->SetInteger(0);
}

void CJX_HostPseudoModel::ResetData(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 1) {
    ThrowParamCountMismatchException(L"resetData");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  WideString wsExpression;
  if (iLength >= 1) {
    ByteString bsExpression = pArguments->GetUTF8String(0);
    wsExpression = WideString::FromUTF8(bsExpression.AsStringView());
  }
  if (wsExpression.IsEmpty()) {
    pNotify->ResetData();
    return;
  }

  int32_t iStart = 0;
  WideString wsName;
  CXFA_Node* pNode = nullptr;
  int32_t iExpLength = wsExpression.GetLength();
  while (iStart < iExpLength) {
    iStart = FilterName(wsExpression.AsStringView(), iStart, wsName);
    CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
    if (!pScriptContext)
      return;

    CXFA_Object* pObject = pScriptContext->GetThisObject();
    if (!pObject)
      return;

    uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Parent |
                      XFA_RESOLVENODE_Siblings;
    XFA_RESOLVENODE_RS resolveNodeRS;
    int32_t iRet = pScriptContext->ResolveObjects(
        pObject, wsName.AsStringView(), resolveNodeRS, dwFlag);
    if (iRet < 1 || !resolveNodeRS.objects.front()->IsNode())
      continue;

    pNode = resolveNodeRS.objects.front()->AsNode();
    pNotify->ResetData(pNode->GetWidgetData());
  }
  if (!pNode)
    pNotify->ResetData();
}

void CJX_HostPseudoModel::Beep(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 1) {
    ThrowParamCountMismatchException(L"beep");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  uint32_t dwType = 4;
  if (iLength >= 1)
    dwType = pArguments->GetInt32(0);

  pNotify->GetAppProvider()->Beep(dwType);
}

void CJX_HostPseudoModel::SetFocus(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"setFocus");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pNode = nullptr;
  if (iLength >= 1) {
    std::unique_ptr<CFXJSE_Value> pValue(pArguments->GetValue(0));
    if (pValue->IsObject()) {
      pNode = ToNode(pValue.get(), nullptr);
    } else if (pValue->IsString()) {
      CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
      if (!pScriptContext)
        return;

      CXFA_Object* pObject = pScriptContext->GetThisObject();
      if (!pObject)
        return;

      uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Parent |
                        XFA_RESOLVENODE_Siblings;
      XFA_RESOLVENODE_RS resolveNodeRS;
      int32_t iRet = pScriptContext->ResolveObjects(
          pObject, pValue->ToWideString().AsStringView(), resolveNodeRS,
          dwFlag);
      if (iRet < 1 || !resolveNodeRS.objects.front()->IsNode())
        return;

      pNode = resolveNodeRS.objects.front()->AsNode();
    }
  }
  pNotify->SetFocusWidgetNode(pNode);
}

void CJX_HostPseudoModel::GetFocus(CFXJSE_Arguments* pArguments) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pNode = pNotify->GetFocusWidgetNode();
  if (!pNode)
    return;

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNode));
}

void CJX_HostPseudoModel::MessageBox(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 4) {
    ThrowParamCountMismatchException(L"messageBox");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  WideString wsMessage;
  WideString bsTitle;
  uint32_t dwMessageType = XFA_MBICON_Error;
  uint32_t dwButtonType = XFA_MB_OK;
  if (iLength >= 1) {
    if (!ValidateArgsForMsg(pArguments, 0, wsMessage))
      return;
  }
  if (iLength >= 2) {
    if (!ValidateArgsForMsg(pArguments, 1, bsTitle))
      return;
  }
  if (iLength >= 3) {
    dwMessageType = pArguments->GetInt32(2);
    if (dwMessageType > XFA_MBICON_Status)
      dwMessageType = XFA_MBICON_Error;
  }
  if (iLength >= 4) {
    dwButtonType = pArguments->GetInt32(3);
    if (dwButtonType > XFA_MB_YesNoCancel)
      dwButtonType = XFA_MB_OK;
  }

  int32_t iValue = pNotify->GetAppProvider()->MsgBox(
      wsMessage, bsTitle, dwMessageType, dwButtonType);
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetInteger(iValue);
}

bool CJX_HostPseudoModel::ValidateArgsForMsg(CFXJSE_Arguments* pArguments,
                                             int32_t iArgIndex,
                                             WideString& wsValue) {
  if (!pArguments || iArgIndex < 0)
    return false;

  bool bIsJsType = false;
  if (GetDocument()->GetScriptContext()->GetType() ==
      CXFA_ScriptData::Type::Javascript) {
    bIsJsType = true;
  }

  std::unique_ptr<CFXJSE_Value> pValueArg(pArguments->GetValue(iArgIndex));
  if (!pValueArg->IsString() && bIsJsType) {
    ThrowArgumentMismatchException();
    return false;
  }
  wsValue = pValueArg->IsNull() ? L"" : pValueArg->ToWideString();
  return true;
}

void CJX_HostPseudoModel::DocumentCountInBatch(CFXJSE_Arguments* pArguments) {
  if (CFXJSE_Value* pValue = pArguments->GetReturnValue())
    pValue->SetInteger(0);
}

void CJX_HostPseudoModel::Print(CFXJSE_Arguments* pArguments) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  int32_t iLength = pArguments->GetLength();
  if (iLength != 8) {
    ThrowParamCountMismatchException(L"print");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  uint32_t dwOptions = 0;
  bool bShowDialog = true;
  if (iLength >= 1)
    bShowDialog = pArguments->GetInt32(0) == 0 ? false : true;
  if (bShowDialog)
    dwOptions |= XFA_PRINTOPT_ShowDialog;

  int32_t nStartPage = 0;
  if (iLength >= 2)
    nStartPage = pArguments->GetInt32(1);

  int32_t nEndPage = 0;
  if (iLength >= 3)
    nEndPage = pArguments->GetInt32(2);

  bool bCanCancel = true;
  if (iLength >= 4)
    bCanCancel = pArguments->GetInt32(3) == 0 ? false : true;
  if (bCanCancel)
    dwOptions |= XFA_PRINTOPT_CanCancel;

  bool bShrinkPage = true;
  if (iLength >= 5)
    bShrinkPage = pArguments->GetInt32(4) == 0 ? false : true;
  if (bShrinkPage)
    dwOptions |= XFA_PRINTOPT_ShrinkPage;

  bool bAsImage = true;
  if (iLength >= 6)
    bAsImage = pArguments->GetInt32(5) == 0 ? false : true;
  if (bAsImage)
    dwOptions |= XFA_PRINTOPT_AsImage;

  bool bReverseOrder = true;
  if (iLength >= 7)
    bAsImage = pArguments->GetInt32(5) == 0 ? false : true;

  bReverseOrder = pArguments->GetInt32(6) == 0 ? false : true;
  if (bReverseOrder)
    dwOptions |= XFA_PRINTOPT_ReverseOrder;

  bool bPrintAnnot = true;
  if (iLength >= 8)
    bPrintAnnot = pArguments->GetInt32(7) == 0 ? false : true;
  if (bPrintAnnot)
    dwOptions |= XFA_PRINTOPT_PrintAnnot;

  pNotify->GetDocEnvironment()->Print(hDoc, nStartPage, nEndPage, dwOptions);
}

void CJX_HostPseudoModel::ImportData(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 1) {
    ThrowParamCountMismatchException(L"importData");
    return;
  }
  // Not implemented.
}

void CJX_HostPseudoModel::ExportData(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 2) {
    ThrowParamCountMismatchException(L"exportData");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  WideString wsFilePath;
  bool bXDP = true;
  if (iLength >= 1) {
    ByteString bsFilePath = pArguments->GetUTF8String(0);
    wsFilePath = WideString::FromUTF8(bsFilePath.AsStringView());
  }
  if (iLength >= 2)
    bXDP = pArguments->GetInt32(1) == 0 ? false : true;

  pNotify->GetDocEnvironment()->ExportData(hDoc, wsFilePath, bXDP);
}

void CJX_HostPseudoModel::PageUp(CFXJSE_Arguments* pArguments) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  int32_t nCurPage = pNotify->GetDocEnvironment()->GetCurrentPage(hDoc);
  int32_t nNewPage = 0;
  if (nCurPage <= 1)
    return;

  nNewPage = nCurPage - 1;
  pNotify->GetDocEnvironment()->SetCurrentPage(hDoc, nNewPage);
}

void CJX_HostPseudoModel::PageDown(CFXJSE_Arguments* pArguments) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetHDOC();
  int32_t nCurPage = pNotify->GetDocEnvironment()->GetCurrentPage(hDoc);
  int32_t nPageCount = pNotify->GetDocEnvironment()->CountPages(hDoc);
  if (!nPageCount || nCurPage == nPageCount)
    return;

  int32_t nNewPage = 0;
  if (nCurPage >= nPageCount)
    nNewPage = nPageCount - 1;
  else
    nNewPage = nCurPage + 1;

  pNotify->GetDocEnvironment()->SetCurrentPage(hDoc, nNewPage);
}

void CJX_HostPseudoModel::CurrentDateTime(CFXJSE_Arguments* pArguments) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  WideString wsDataTime = pNotify->GetCurrentDateTime();
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetString(wsDataTime.UTF8Encode().AsStringView());
}
