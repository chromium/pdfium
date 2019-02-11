// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_para.h"

#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"

namespace {

const CXFA_Node::PropertyData kParaPropertyData[] = {
    {XFA_Element::Hyphenation, 1, 0},
};

const CXFA_Node::AttributeData kParaAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::HAlign, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Left},
    {XFA_Attribute::TextIndent, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Widows, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::MarginRight, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::MarginLeft, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::RadixOffset, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::Preserve, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::SpaceBelow, XFA_AttributeType::Measure, (void*)L"0in"},
    {XFA_Attribute::VAlign, XFA_AttributeType::Enum,
     (void*)XFA_AttributeValue::Top},
    {XFA_Attribute::TabDefault, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::TabStops, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Orphans, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::LineHeight, XFA_AttributeType::Measure, (void*)L"0pt"},
    {XFA_Attribute::SpaceAbove, XFA_AttributeType::Measure, (void*)L"0in"},
};

}  // namespace

CXFA_Para::CXFA_Para(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Para,
                kParaPropertyData,
                kParaAttributeData,
                pdfium::MakeUnique<CJX_Node>(this)) {}

CXFA_Para::~CXFA_Para() = default;

XFA_AttributeValue CXFA_Para::GetHorizontalAlign() {
  return JSObject()
      ->TryEnum(XFA_Attribute::HAlign, true)
      .value_or(XFA_AttributeValue::Left);
}

XFA_AttributeValue CXFA_Para::GetVerticalAlign() {
  return JSObject()
      ->TryEnum(XFA_Attribute::VAlign, true)
      .value_or(XFA_AttributeValue::Top);
}

float CXFA_Para::GetLineHeight() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::LineHeight, XFA_Unit::Pt);
}

float CXFA_Para::GetMarginLeft() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::MarginLeft, XFA_Unit::Pt);
}

float CXFA_Para::GetMarginRight() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::MarginRight, XFA_Unit::Pt);
}

float CXFA_Para::GetSpaceAbove() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::SpaceAbove, XFA_Unit::Pt);
}

float CXFA_Para::GetSpaceBelow() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::SpaceBelow, XFA_Unit::Pt);
}

float CXFA_Para::GetTextIndent() {
  return JSObject()->GetMeasureInUnit(XFA_Attribute::TextIndent, XFA_Unit::Pt);
}
