// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_fill.h"

#include "fxjs/xfa/cjx_fill.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_linear.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_pattern.h"
#include "xfa/fxfa/parser/cxfa_radial.h"
#include "xfa/fxfa/parser/cxfa_stipple.h"

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
                kName,
                pdfium::MakeUnique<CJX_Fill>(this)) {}

CXFA_Fill::~CXFA_Fill() {}

bool CXFA_Fill::IsVisible() {
  return JSObject()
             ->TryEnum(XFA_Attribute::Presence, true)
             .value_or(XFA_AttributeEnum::Visible) ==
         XFA_AttributeEnum::Visible;
}

void CXFA_Fill::SetColor(FX_ARGB color) {
  CXFA_Color* pColor =
      JSObject()->GetOrCreateProperty<CXFA_Color>(0, XFA_Element::Color);
  if (!pColor)
    return;

  pColor->SetValue(color);
}

FX_ARGB CXFA_Fill::GetColor(bool bText) {
  CXFA_Color* pColor = GetChild<CXFA_Color>(0, XFA_Element::Color, false);
  if (!pColor)
    return bText ? 0xFF000000 : 0xFFFFFFFF;
  return pColor->GetValueOrDefault(bText ? 0xFF000000 : 0xFFFFFFFF);
}

XFA_Element CXFA_Fill::GetFillType() const {
  CXFA_Node* pChild = GetFirstChild();
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNextSibling();
  }
  return XFA_Element::Solid;
}

XFA_AttributeEnum CXFA_Fill::GetPatternType() {
  CXFA_Pattern* pattern = GetPatternIfExists();
  return pattern ? pattern->GetType() : CXFA_Pattern::kDefaultType;
}

FX_ARGB CXFA_Fill::GetPatternColor() {
  CXFA_Pattern* pattern = GetPatternIfExists();
  if (!pattern)
    return CXFA_Color::kBlackColor;

  CXFA_Color* pColor = pattern->GetColorIfExists();
  return pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
}

int32_t CXFA_Fill::GetStippleRate() {
  CXFA_Stipple* stipple = GetStippleIfExists();
  if (!stipple)
    return CXFA_Stipple::GetDefaultRate();
  return stipple->GetRate();
}

FX_ARGB CXFA_Fill::GetStippleColor() {
  CXFA_Stipple* stipple = GetStippleIfExists();
  if (!stipple)
    return CXFA_Color::kBlackColor;

  CXFA_Color* pColor = stipple->GetColorIfExists();
  return pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
}

XFA_AttributeEnum CXFA_Fill::GetLinearType() {
  CXFA_Linear* linear = GetLinearIfExists();
  return linear ? linear->GetType() : CXFA_Linear::kDefaultType;
}

FX_ARGB CXFA_Fill::GetLinearColor() {
  CXFA_Linear* linear = GetLinearIfExists();
  if (!linear)
    return CXFA_Color::kBlackColor;

  CXFA_Color* pColor = linear->GetColorIfExists();
  return pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
}

bool CXFA_Fill::IsRadialToEdge() {
  CXFA_Radial* radial = GetRadialIfExists();
  return radial ? radial->IsToEdge() : false;
}

FX_ARGB CXFA_Fill::GetRadialColor() {
  CXFA_Radial* radial = GetRadialIfExists();
  if (!radial)
    return CXFA_Color::kBlackColor;

  CXFA_Color* pColor = radial->GetColorIfExists();
  return pColor ? pColor->GetValue() : CXFA_Color::kBlackColor;
}

CXFA_Stipple* CXFA_Fill::GetStippleIfExists() {
  return JSObject()->GetOrCreateProperty<CXFA_Stipple>(0, XFA_Element::Stipple);
}

CXFA_Radial* CXFA_Fill::GetRadialIfExists() {
  return JSObject()->GetOrCreateProperty<CXFA_Radial>(0, XFA_Element::Radial);
}

CXFA_Linear* CXFA_Fill::GetLinearIfExists() {
  return JSObject()->GetOrCreateProperty<CXFA_Linear>(0, XFA_Element::Linear);
}

CXFA_Pattern* CXFA_Fill::GetPatternIfExists() {
  return JSObject()->GetOrCreateProperty<CXFA_Pattern>(0, XFA_Element::Pattern);
}
