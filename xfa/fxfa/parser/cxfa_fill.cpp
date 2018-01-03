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
  CXFA_Color* pNode =
      JSObject()->GetProperty<CXFA_Color>(0, XFA_Element::Color, true);
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  pNode->JSObject()->SetCData(XFA_Attribute::Value,
                              WideString::Format(L"%d,%d,%d", r, g, b), false,
                              false);
}

FX_ARGB CXFA_Fill::GetColor(bool bText) {
  if (CXFA_Color* pNode = GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pNode->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  if (bText)
    return 0xFF000000;
  return 0xFFFFFFFF;
}

XFA_Element CXFA_Fill::GetFillType() const {
  CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_Element::Solid;
}

XFA_AttributeEnum CXFA_Fill::GetPatternType() {
  return GetPattern()->JSObject()->GetEnum(XFA_Attribute::Type);
}

FX_ARGB CXFA_Fill::GetPatternColor() {
  if (CXFA_Color* pColor =
          GetPattern()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

int32_t CXFA_Fill::GetStippleRate() {
  return GetStipple()
      ->JSObject()
      ->TryInteger(XFA_Attribute::Rate, true)
      .value_or(50);
}

FX_ARGB CXFA_Fill::GetStippleColor() {
  if (CXFA_Color* pColor =
          GetStipple()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

XFA_AttributeEnum CXFA_Fill::GetLinearType() {
  return GetLinear()
      ->JSObject()
      ->TryEnum(XFA_Attribute::Type, true)
      .value_or(XFA_AttributeEnum::ToRight);
}

FX_ARGB CXFA_Fill::GetLinearColor() {
  if (CXFA_Color* pColor =
          GetLinear()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

bool CXFA_Fill::IsRadialToEdge() {
  return GetRadial()
             ->JSObject()
             ->TryEnum(XFA_Attribute::Type, true)
             .value_or(XFA_AttributeEnum::ToEdge) == XFA_AttributeEnum::ToEdge;
}

FX_ARGB CXFA_Fill::GetRadialColor() {
  if (CXFA_Color* pColor =
          GetRadial()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

CXFA_Stipple* CXFA_Fill::GetStipple() {
  return JSObject()->GetProperty<CXFA_Stipple>(0, XFA_Element::Stipple, true);
}

CXFA_Radial* CXFA_Fill::GetRadial() {
  return JSObject()->GetProperty<CXFA_Radial>(0, XFA_Element::Radial, true);
}

CXFA_Linear* CXFA_Fill::GetLinear() {
  return JSObject()->GetProperty<CXFA_Linear>(0, XFA_Element::Linear, true);
}

CXFA_Pattern* CXFA_Fill::GetPattern() {
  return JSObject()->GetProperty<CXFA_Pattern>(0, XFA_Element::Pattern, true);
}
