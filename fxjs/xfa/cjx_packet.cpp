// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_packet.h"

#include <utility>
#include <vector>

#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_packet.h"

const CJX_MethodSpec CJX_Packet::MethodSpecs[] = {
    {"getAttribute", getAttribute_static},
    {"removeAttribute", removeAttribute_static},
    {"setAttribute", setAttribute_static}};

CJX_Packet::CJX_Packet(CXFA_Packet* packet) : CJX_Node(packet) {
  DefineMethods(MethodSpecs);
}

CJX_Packet::~CJX_Packet() = default;

bool CJX_Packet::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Packet::getAttribute(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString attributeValue;
  CFX_XMLElement* element = ToXMLElement(GetXFANode()->GetXMLMappingNode());
  if (element)
    attributeValue = element->GetAttribute(runtime->ToWideString(params[0]));

  return CJS_Result::Success(
      runtime->NewString(attributeValue.ToUTF8().AsStringView()));
}

CJS_Result CJX_Packet::setAttribute(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  CFX_XMLElement* element = ToXMLElement(GetXFANode()->GetXMLMappingNode());
  if (element) {
    element->SetAttribute(runtime->ToWideString(params[1]),
                          runtime->ToWideString(params[0]));
  }
  return CJS_Result::Success(runtime->NewNull());
}

CJS_Result CJX_Packet::removeAttribute(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CFX_XMLElement* pElement = ToXMLElement(GetXFANode()->GetXMLMappingNode());
  if (pElement)
    pElement->RemoveAttribute(runtime->ToWideString(params[0]));

  return CJS_Result::Success(runtime->NewNull());
}

void CJX_Packet::content(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute eAttribute) {
  CFX_XMLElement* element = ToXMLElement(GetXFANode()->GetXMLMappingNode());
  if (bSetting) {
    if (element) {
      element->AppendLastChild(
          GetXFANode()
              ->GetDocument()
              ->GetNotify()
              ->GetFFDoc()
              ->GetXMLDocument()
              ->CreateNode<CFX_XMLText>(
                  fxv8::ReentrantToWideStringHelper(pIsolate, *pValue)));
    }
    return;
  }

  WideString wsTextData;
  if (element)
    wsTextData = element->GetTextData();

  *pValue = fxv8::NewStringHelper(pIsolate, wsTextData.ToUTF8().AsStringView());
}
