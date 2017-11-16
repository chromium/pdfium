// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_filldata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_FillData::CXFA_FillData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

CXFA_FillData::~CXFA_FillData() {}

int32_t CXFA_FillData::GetPresence() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Presence);
}

void CXFA_FillData::SetColor(FX_ARGB color) {
  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Color, true);
  WideString wsColor;
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pNode->JSNode()->SetCData(XFA_Attribute::Value, wsColor, false, false);
}

FX_ARGB CXFA_FillData::GetColor(bool bText) {
  if (CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pNode->JSNode()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      return CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  if (bText)
    return 0xFF000000;
  return 0xFFFFFFFF;
}

XFA_Element CXFA_FillData::GetFillType() {
  CXFA_Node* pChild = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pChild) {
    XFA_Element eType = pChild->GetElementType();
    if (eType != XFA_Element::Color && eType != XFA_Element::Extras)
      return eType;

    pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return XFA_Element::Solid;
}

int32_t CXFA_FillData::GetPattern(FX_ARGB& foreColor) {
  foreColor = 0xFF000000;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Pattern, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSNode()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      foreColor = CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return pNode->JSNode()->GetEnum(XFA_Attribute::Type);
}

int32_t CXFA_FillData::GetStipple(FX_ARGB& stippleColor) {
  stippleColor = 0xFF000000;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Stipple, true);
  int32_t eAttr = 50;
  pNode->JSNode()->TryInteger(XFA_Attribute::Rate, eAttr, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSNode()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      stippleColor = CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return eAttr;
}

int32_t CXFA_FillData::GetLinear(FX_ARGB& endColor) {
  endColor = 0xFF000000;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Linear, true);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToRight;
  pNode->JSNode()->TryEnum(XFA_Attribute::Type, eAttr, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSNode()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      endColor = CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return eAttr;
}

int32_t CXFA_FillData::GetRadial(FX_ARGB& endColor) {
  endColor = 0xFF000000;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Radial, true);
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_ToEdge;
  pNode->JSNode()->TryEnum(XFA_Attribute::Type, eAttr, true);
  if (CXFA_Node* pColor = pNode->GetChild(0, XFA_Element::Color, false)) {
    pdfium::Optional<WideString> wsColor =
        pColor->JSNode()->TryCData(XFA_Attribute::Value, false);
    if (wsColor)
      endColor = CXFA_DataData::ToColor(wsColor->AsStringView());
  }
  return eAttr;
}
