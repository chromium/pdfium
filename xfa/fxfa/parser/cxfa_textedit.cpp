// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_textedit.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kTextEditPropertyData[] = {
    {XFA_Element::Margin, 1, {}},
    {XFA_Element::Border, 1, {}},
    {XFA_Element::Comb, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kTextEditAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::VScrollPolicy, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Auto},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::AllowRichText, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::MultiLine, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::HScrollPolicy, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Auto},
};

}  // namespace

CXFA_TextEdit::CXFA_TextEdit(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::TextEdit,
                kTextEditPropertyData,
                kTextEditAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_TextEdit::~CXFA_TextEdit() = default;

XFA_FFWidgetType CXFA_TextEdit::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kTextEdit;
}
