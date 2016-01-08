// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_script_signaturepseudomodel.h"
CScript_SignaturePseudoModel::CScript_SignaturePseudoModel(
    CXFA_Document* pDocument)
    : CXFA_OrdinaryObject(pDocument, XFA_ELEMENT_SignaturePseudoModel) {
  m_uScriptHash = XFA_HASHCODE_Signature;
}
CScript_SignaturePseudoModel::~CScript_SignaturePseudoModel() {}
void CScript_SignaturePseudoModel::Script_SignaturePseudoModel_Verify(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 4) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"verify");
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  IXFA_Doc* hDoc = pNotify->GetHDOC();
  CXFA_Node* pNode = NULL;
  if (iLength >= 1) {
    pNode = (CXFA_Node*)pArguments->GetObject(0);
  }
  int32_t bVerify = pNotify->GetDocProvider()->Verify(hDoc, pNode);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetInteger(hValue, bVerify);
  }
}
void CScript_SignaturePseudoModel::Script_SignaturePseudoModel_Sign(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 3 || iLength > 7) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"sign");
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  IXFA_Doc* hDoc = pNotify->GetHDOC();
  CXFA_NodeList* pNodeList = NULL;
  CFX_WideString wsExpression;
  CFX_WideString wsXMLIdent;
  if (iLength >= 1) {
    pNodeList = (CXFA_NodeList*)pArguments->GetObject(0);
  }
  if (iLength >= 2) {
    CFX_ByteString bsExpression = pArguments->GetUTF8String(1);
    wsExpression =
        CFX_WideString::FromUTF8(bsExpression, bsExpression.GetLength());
  }
  if (iLength >= 3) {
    CFX_ByteString bsXMLIdent = pArguments->GetUTF8String(2);
    wsXMLIdent = CFX_WideString::FromUTF8(bsXMLIdent, bsXMLIdent.GetLength());
  }
  FX_BOOL bSign = pNotify->GetDocProvider()->Sign(hDoc, pNodeList, wsExpression,
                                                  wsXMLIdent);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, bSign);
  }
}
void CScript_SignaturePseudoModel::Script_SignaturePseudoModel_Enumerate(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 0) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"enumerate");
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  IXFA_Doc* hDoc = pNotify->GetHDOC();
  CXFA_NodeList* pList = pNotify->GetDocProvider()->Enumerate(hDoc);
  FXJSE_Value_Set(pArguments->GetReturnValue(),
                  m_pDocument->GetScriptContext()->GetJSValueFromMap(pList));
}
void CScript_SignaturePseudoModel::Script_SignaturePseudoModel_Clear(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD, L"clear");
    return;
  }
  IXFA_Notify* pNotify = m_pDocument->GetParser()->GetNotify();
  if (!pNotify) {
    return;
  }
  IXFA_Doc* hDoc = pNotify->GetHDOC();
  CXFA_Node* pNode = NULL;
  FX_BOOL bClear = TRUE;
  if (iLength >= 1) {
    pNode = (CXFA_Node*)pArguments->GetObject(0);
  }
  if (iLength >= 2) {
    bClear = pArguments->GetInt32(1) == 0 ? FALSE : TRUE;
  }
  FX_BOOL bFlag = pNotify->GetDocProvider()->Clear(hDoc, pNode, bClear);
  FXJSE_HVALUE hValue = pArguments->GetReturnValue();
  if (hValue) {
    FXJSE_Value_SetBoolean(hValue, bFlag);
  }
}
