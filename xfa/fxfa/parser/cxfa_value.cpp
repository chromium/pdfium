// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_value.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Arc, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Text, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Time, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::DateTime, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Image, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Decimal, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Boolean, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Integer, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::ExData, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Rectangle, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Date, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Float, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Line, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Relevant, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Override, XFA_AttributeType::Boolean, (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"value";

}  // namespace

CXFA_Value::CXFA_Value(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Value,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Value::~CXFA_Value() {}
