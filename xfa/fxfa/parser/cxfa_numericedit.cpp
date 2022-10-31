// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_numericedit.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kNumericEditPropertyData[] = {
    {XFA_Element::Margin, 1, {}},
    {XFA_Element::Border, 1, {}},
    {XFA_Element::Comb, 1, {}},
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kNumericEditAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::HScrollPolicy, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Auto},
};

}  // namespace

CXFA_NumericEdit::CXFA_NumericEdit(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::NumericEdit,
                kNumericEditPropertyData,
                kNumericEditAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_NumericEdit::~CXFA_NumericEdit() = default;

XFA_Element CXFA_NumericEdit::GetValueNodeType() const {
  return XFA_Element::Float;
}

XFA_FFWidgetType CXFA_NumericEdit::GetDefaultFFWidgetType() const {
  return XFA_FFWidgetType::kNumericEdit;
}
