// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_packet.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_packet.h"

const CJX_MethodSpec CJX_Packet::MethodSpecs[] = {
    {"getAttribute", getAttribute_static},
    {"removeAttribute", removeAttribute_static},
    {"setAttribute", setAttribute_static},
    {"", nullptr}};

CJX_Packet::CJX_Packet(CXFA_Packet* packet) : CJX_Node(packet) {
  DefineMethods(MethodSpecs);
}

CJX_Packet::~CJX_Packet() {}

void CJX_Packet::getAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"getAttribute");
    return;
  }

  WideString wsAttributeValue;
  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsAttributeName = pArguments->GetUTF8String(0);
    wsAttributeValue = static_cast<CFX_XMLElement*>(pXMLNode)->GetString(
        WideString::FromUTF8(bsAttributeName.AsStringView()).c_str());
  }

  pArguments->GetReturnValue()->SetString(
      wsAttributeValue.UTF8Encode().AsStringView());
}

void CJX_Packet::setAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    ThrowParamCountMismatchException(L"setAttribute");
    return;
  }

  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsValue = pArguments->GetUTF8String(0);
    ByteString bsName = pArguments->GetUTF8String(1);
    static_cast<CFX_XMLElement*>(pXMLNode)->SetString(
        WideString::FromUTF8(bsName.AsStringView()),
        WideString::FromUTF8(bsValue.AsStringView()));
  }
  pArguments->GetReturnValue()->SetNull();
}

void CJX_Packet::removeAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"removeAttribute");
    return;
  }

  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsName = pArguments->GetUTF8String(0);
    WideString wsName = WideString::FromUTF8(bsName.AsStringView());
    CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
    if (pXMLElement->HasAttribute(wsName.c_str()))
      pXMLElement->RemoveAttribute(wsName.c_str());
  }
  pArguments->GetReturnValue()->SetNull();
}
