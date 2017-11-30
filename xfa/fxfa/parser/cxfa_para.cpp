// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {
    {XFA_Element::Hyphenation, 1, 0},
    {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::HAlign, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Left},
    {XFA_Attribute::TextIndent, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Widows, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::MarginRight, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::MarginLeft, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::RadixOffset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Preserve, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::SpaceBelow, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::VAlign, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Top},
    {XFA_Attribute::TabDefault, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::TabStops, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Orphans, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LineHeight, XFA_AttributeType::Measure, (void*)L"0pt"},
    {XFA_Attribute::SpaceAbove, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"para";

}  // namespace

CXFA_Para::CXFA_Para(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Para,
                kPropertyData,
                kAttributeData,
                kName) {}

CXFA_Para::~CXFA_Para() {}
