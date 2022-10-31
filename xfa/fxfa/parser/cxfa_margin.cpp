// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margin.h"

#include "fxjs/xfa/cjx_node.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kMarginPropertyData[] = {
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kMarginAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LeftInset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::BottomInset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::TopInset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::RightInset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Margin::CXFA_Margin(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Margin,
                kMarginPropertyData,
                kMarginAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Margin::~CXFA_Margin() = default;

float CXFA_Margin::GetLeftInset() const {
  return TryLeftInset().value_or(0);
}

float CXFA_Margin::GetTopInset() const {
  return TryTopInset().value_or(0);
}

float CXFA_Margin::GetRightInset() const {
  return TryRightInset().value_or(0);
}

float CXFA_Margin::GetBottomInset() const {
  return TryBottomInset().value_or(0);
}

absl::optional<float> CXFA_Margin::TryLeftInset() const {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::LeftInset);
}

absl::optional<float> CXFA_Margin::TryTopInset() const {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::TopInset);
}

absl::optional<float> CXFA_Margin::TryRightInset() const {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::RightInset);
}

absl::optional<float> CXFA_Margin::TryBottomInset() const {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::BottomInset);
}
