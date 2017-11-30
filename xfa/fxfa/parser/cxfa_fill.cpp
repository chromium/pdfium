// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fill.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Pattern, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Solid, 1,
     XFA_PROPERTYFLAG_OneOf | XFA_PROPERTYFLAG_DefaultOneOf},
    {XFA_Element::Stipple, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Color, 1, 0},
    {XFA_Element::Linear, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Extras, 1, 0},
    {XFA_Element::Radial, 1, XFA_PROPERTYFLAG_OneOf},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Presence, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Visible},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"fill";

}  // namespace

CXFA_Fill::CXFA_Fill(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Fill,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Fill::~CXFA_Fill() {}
