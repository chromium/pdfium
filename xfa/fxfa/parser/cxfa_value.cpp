// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_value.h"

#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_arc.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_exdata.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_line.h"
#include "xfa/fxfa/parser/cxfa_rectangle.h"

namespace {

constexpr CXFA_Node::PropertyData kValuePropertyData[] = {
    {XFA_Element::Arc, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Text, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Time, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::DateTime, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Image, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Decimal, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Boolean, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Integer, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::ExData, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Rectangle, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Date, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Float, 1, XFA_PropertyFlag::kOneOf},
    {XFA_Element::Line, 1, XFA_PropertyFlag::kOneOf},
};

constexpr CXFA_Node::AttributeData kValueAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Override, XFA_AttributeType::Boolean, (void*)0},
};

}  // namespace

CXFA_Value::CXFA_Value(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Value,
                kValuePropertyData,
                kValueAttributeData,
                cppgc::MakeGarbageCollected<CJX_Node>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Value::~CXFA_Value() = default;

XFA_Element CXFA_Value::GetChildValueClassID() const {
  CXFA_Node* pNode = GetFirstChild();
  return pNode ? pNode->GetElementType() : XFA_Element::Unknown;
}

WideString CXFA_Value::GetChildValueContent() const {
  CXFA_Node* pNode = GetFirstChild();
  return pNode
             ? pNode->JSObject()->TryContent(false, true).value_or(WideString())
             : WideString();
}

CXFA_Arc* CXFA_Value::GetArcIfExists() const {
  return CXFA_Arc::FromNode(GetFirstChild());
}

CXFA_Line* CXFA_Value::GetLineIfExists() const {
  return CXFA_Line::FromNode(GetFirstChild());
}

CXFA_Rectangle* CXFA_Value::GetRectangleIfExists() const {
  return CXFA_Rectangle::FromNode(GetFirstChild());
}

CXFA_Text* CXFA_Value::GetTextIfExists() const {
  return CXFA_Text::FromNode(GetFirstChild());
}

CXFA_ExData* CXFA_Value::GetExDataIfExists() const {
  return CXFA_ExData::FromNode(GetFirstChild());
}

CXFA_Image* CXFA_Value::GetImageIfExists() const {
  return CXFA_Image::FromNode(GetFirstChild());
}
