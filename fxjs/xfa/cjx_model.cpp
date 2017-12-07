// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_model.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_delta.h"
#include "xfa/fxfa/parser/cxfa_document.h"

const CJX_MethodSpec CJX_Model::MethodSpecs[] = {
    {"clearErrorList", clearErrorList_static},
    {"createNode", createNode_static},
    {"isCompatibleNS", isCompatibleNS_static},
    {"", nullptr}};

CJX_Model::CJX_Model(CXFA_Node* node) : CJX_Node(node) {
  DefineMethods(MethodSpecs);
}

CJX_Model::~CJX_Model() {}

void CJX_Model::clearErrorList(CFXJSE_Arguments* pArguments) {}

void CJX_Model::createNode(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc <= 0 || argc >= 4) {
    ThrowParamCountMismatchException(L"createNode");
    return;
  }

  WideString strName;
  WideString strNameSpace;
  if (argc > 1) {
    ByteString bsName = pArguments->GetUTF8String(1);
    strName = WideString::FromUTF8(bsName.AsStringView());
    if (argc == 3) {
      ByteString bsNameSpace = pArguments->GetUTF8String(2);
      strNameSpace = WideString::FromUTF8(bsNameSpace.AsStringView());
    }
  }

  ByteString bsTagName = pArguments->GetUTF8String(0);
  WideString strTagName = WideString::FromUTF8(bsTagName.AsStringView());
  XFA_Element eType = CXFA_Node::NameToElement(strTagName);
  CXFA_Node* pNewNode = GetXFANode()->CreateSamePacketNode(eType);
  if (!pNewNode) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  if (strName.IsEmpty()) {
    pArguments->GetReturnValue()->Assign(
        GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewNode));
    return;
  }

  if (!pNewNode->HasAttribute(XFA_Attribute::Name)) {
    ThrowMissingPropertyException(strTagName, L"name");
    return;
  }

  pNewNode->JSNode()->SetAttribute(XFA_Attribute::Name, strName.AsStringView(),
                                   true);
  if (pNewNode->GetPacketType() == XFA_PacketType::Datasets)
    pNewNode->CreateXMLMappingNode();

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewNode));
}

void CJX_Model::isCompatibleNS(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1) {
    ThrowParamCountMismatchException(L"isCompatibleNS");
    return;
  }

  WideString wsNameSpace;
  if (iLength >= 1) {
    ByteString bsNameSpace = pArguments->GetUTF8String(0);
    wsNameSpace = WideString::FromUTF8(bsNameSpace.AsStringView());
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pValue)
    return;

  pValue->SetBoolean(TryNamespace().value_or(WideString()) == wsNameSpace);
}
