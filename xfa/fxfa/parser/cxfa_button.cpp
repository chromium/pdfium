// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_button.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kButtonPropertyData[] = {
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kButtonAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Highlight, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Inverted},
};

}  // namespace

// static
CXFA_Button* CXFA_Button::FromNode(CXFA_Node* pNode) {
  return pNode && pNode->GetElementType() == XFA_Element::Button
             ? static_cast<CXFA_Button*>(pNode)
             : nullptr;
}

CXFA_Button::CXFA_Button(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Button,
                kButtonPropertyData,
                kButtonAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Button::~CXFA_Button() = default;

XFA_FFWidgetType CXFA_Button::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kButton;
}

XFA_AttributeValue CXFA_Button::GetHighlight() {
  return JSObject()->GetEnum(XFA_Attribute::Highlight);
}
