// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_filldata.h"

#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_linear.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_pattern.h"
#include "xfa/fxfa/parser/cxfa_radial.h"
#include "xfa/fxfa/parser/cxfa_stipple.h"

CXFA_FillData::CXFA_FillData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

CXFA_FillData::~CXFA_FillData() {}

bool CXFA_FillData::IsVisible() const {
  return m_pNode->JSObject()
             ->TryEnum(XFA_Attribute::Presence, true)
             .value_or(XFA_AttributeEnum::Visible) ==
         XFA_AttributeEnum::Visible;
}

void CXFA_FillData::SetColor(FX_ARGB color) {
  CXFA_Color* pNode =
      m_pNode->JSObject()->GetProperty<CXFA_Color>(0, XFA_Element::Color, true);
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  pNode->JSObject()->SetCData(XFA_Attribute::Value,
                              WideString::Format(L"%d,%d,%d", r, g, b), false,
                              false);
}

FX_ARGB CXFA_FillData::GetColor(bool bText) const {
  if (CXFA_Color* pNode =
          m_pNode->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pNode->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  if (bText)
    return 0xFF000000;
  return 0xFFFFFFFF;
}

XFA_Element CXFA_FillData::GetFillType() const {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_Element::Solid;
}

XFA_AttributeEnum CXFA_FillData::GetPatternType() const {
  return GetPattern()->JSObject()->GetEnum(XFA_Attribute::Type);
}

FX_ARGB CXFA_FillData::GetPatternColor() const {
  if (CXFA_Color* pColor =
          GetPattern()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

int32_t CXFA_FillData::GetStippleRate() const {
  return GetStipple()
      ->JSObject()
      ->TryInteger(XFA_Attribute::Rate, true)
      .value_or(50);
}

FX_ARGB CXFA_FillData::GetStippleColor() const {
  if (CXFA_Color* pColor =
          GetStipple()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

XFA_AttributeEnum CXFA_FillData::GetLinearType() const {
  return GetLinear()
      ->JSObject()
      ->TryEnum(XFA_Attribute::Type, true)
      .value_or(XFA_AttributeEnum::ToRight);
}

FX_ARGB CXFA_FillData::GetLinearColor() const {
  if (CXFA_Color* pColor =
          GetLinear()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

bool CXFA_FillData::IsRadialToEdge() const {
  return GetRadial()
             ->JSObject()
             ->TryEnum(XFA_Attribute::Type, true)
             .value_or(XFA_AttributeEnum::ToEdge) == XFA_AttributeEnum::ToEdge;
}

FX_ARGB CXFA_FillData::GetRadialColor() const {
  if (CXFA_Color* pColor =
          GetRadial()->GetChild<CXFA_Color>(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSObject()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return 0xFF000000;
}

CXFA_Stipple* CXFA_FillData::GetStipple() const {
  return m_pNode->JSObject()->GetProperty<CXFA_Stipple>(0, XFA_Element::Stipple,
                                                        true);
}

CXFA_Radial* CXFA_FillData::GetRadial() const {
  return m_pNode->JSObject()->GetProperty<CXFA_Radial>(0, XFA_Element::Radial,
                                                       true);
}

CXFA_Linear* CXFA_FillData::GetLinear() const {
  return m_pNode->JSObject()->GetProperty<CXFA_Linear>(0, XFA_Element::Linear,
                                                       true);
}

CXFA_Pattern* CXFA_FillData::GetPattern() const {
  return m_pNode->JSObject()->GetProperty<CXFA_Pattern>(0, XFA_Element::Pattern,
                                                        true);
}
