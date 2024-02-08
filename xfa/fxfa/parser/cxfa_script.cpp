// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_script.h"

#include "fxjs/xfa/cjx_script.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kScriptPropertyData[] = {
    {XFA_Element::Exclude, 1, {}},
    {XFA_Element::CurrentPage, 1, {}},
    {XFA_Element::RunScripts, 1, {}},
};

const CXFA_Node::AttributeData kScriptAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ContentType, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::RunAt, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Client},
    {XFA_Attribute::Binding, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
};

}  // namespace

// static
CXFA_Script* CXFA_Script::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::Script
             ? static_cast<CXFA_Script*>(pNode)
             : nullptr;
}

CXFA_Script::CXFA_Script(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kConfig, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::Script,
                kScriptPropertyData,
                kScriptAttributeData,
                cppgc::MakeGarbageCollected<CJX_Script>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Script::~CXFA_Script() = default;

CXFA_Script::Type CXFA_Script::GetContentType() {
  std::optional<WideString> cData =
      JSObject()->TryCData(XFA_Attribute::ContentType, false);
  if (!cData.has_value())
    return Type::Formcalc;
  if (cData.value().EqualsASCII("application/x-formcalc"))
    return Type::Formcalc;
  if (cData.value().EqualsASCII("application/x-javascript"))
    return Type::Javascript;
  return Type::Unknown;
}

XFA_AttributeValue CXFA_Script::GetRunAt() {
  return JSObject()->GetEnum(XFA_Attribute::RunAt);
}

WideString CXFA_Script::GetExpression() {
  return JSObject()->GetContent(false);
}
