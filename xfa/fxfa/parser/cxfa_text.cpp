// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_text.h"

#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::AttributeData kTextAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Name, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Rid, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::MaxChars, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

// static
CXFA_Text* CXFA_Text::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::Text
             ? static_cast<CXFA_Text*>(pNode)
             : nullptr;
}

CXFA_Text::CXFA_Text(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kSourceSet, XFA_XDPPACKET::kTemplate,
                 XFA_XDPPACKET::kForm},
                XFA_ObjectType::ContentNode,
                XFA_Element::Text,
                {},
                kTextAttributeData,
                cppgc::MakeGarbageCollected<CJX_Object>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Text::~CXFA_Text() = default;

WideString CXFA_Text::GetContent() const {
  return JSObject()->GetContent(false);
}
