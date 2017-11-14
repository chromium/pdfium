// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_strokedata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

int32_t CXFA_StrokeData::GetPresence() const {
  return m_pNode ? m_pNode->JSNode()->GetEnum(XFA_Attribute::Presence)
                 : XFA_ATTRIBUTEENUM_Invisible;
}

int32_t CXFA_StrokeData::GetCapType() const {
  if (!m_pNode)
    return XFA_ATTRIBUTEENUM_Square;
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Cap);
}

int32_t CXFA_StrokeData::GetStrokeType() const {
  return m_pNode ? m_pNode->JSNode()->GetEnum(XFA_Attribute::Stroke)
                 : XFA_ATTRIBUTEENUM_Solid;
}

float CXFA_StrokeData::GetThickness() const {
  return GetMSThickness().ToUnit(XFA_Unit::Pt);
}

CXFA_Measurement CXFA_StrokeData::GetMSThickness() const {
  return m_pNode ? m_pNode->JSNode()->GetMeasure(XFA_Attribute::Thickness)
                 : XFA_GetAttributeDefaultValue_Measure(
                       XFA_Element::Edge, XFA_Attribute::Thickness,
                       XFA_XDPPACKET_Form);
}

void CXFA_StrokeData::SetMSThickness(CXFA_Measurement msThinkness) {
  if (!m_pNode)
    return;

  m_pNode->JSNode()->SetMeasure(XFA_Attribute::Thickness, msThinkness, false);
}

FX_ARGB CXFA_StrokeData::GetColor() const {
  if (!m_pNode)
    return 0xFF000000;

  CXFA_Node* pNode = m_pNode->GetChild(0, XFA_Element::Color, false);
  if (!pNode)
    return 0xFF000000;

  WideStringView wsColor;
  pNode->JSNode()->TryCData(XFA_Attribute::Value, wsColor, true);
  return CXFA_DataData::ToColor(wsColor);
}

void CXFA_StrokeData::SetColor(FX_ARGB argb) {
  if (!m_pNode)
    return;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Color, true);
  WideString wsColor;
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(argb);
  wsColor.Format(L"%d,%d,%d", r, g, b);
  pNode->JSNode()->SetCData(XFA_Attribute::Value, wsColor, false, false);
}

int32_t CXFA_StrokeData::GetJoinType() const {
  return m_pNode ? m_pNode->JSNode()->GetEnum(XFA_Attribute::Join)
                 : XFA_ATTRIBUTEENUM_Square;
}

bool CXFA_StrokeData::IsInverted() const {
  return m_pNode ? m_pNode->JSNode()->GetBoolean(XFA_Attribute::Inverted)
                 : false;
}

float CXFA_StrokeData::GetRadius() const {
  return m_pNode ? m_pNode->JSNode()
                       ->GetMeasure(XFA_Attribute::Radius)
                       .ToUnit(XFA_Unit::Pt)
                 : 0;
}

bool CXFA_StrokeData::SameStyles(CXFA_StrokeData stroke,
                                 uint32_t dwFlags) const {
  if (m_pNode == stroke.GetNode())
    return true;
  if (fabs(GetThickness() - stroke.GetThickness()) >= 0.01f)
    return false;
  if ((dwFlags & XFA_STROKE_SAMESTYLE_NoPresence) == 0 &&
      IsVisible() != stroke.IsVisible()) {
    return false;
  }
  if (GetStrokeType() != stroke.GetStrokeType())
    return false;
  if (GetColor() != stroke.GetColor())
    return false;
  if ((dwFlags & XFA_STROKE_SAMESTYLE_Corner) != 0 &&
      fabs(GetRadius() - stroke.GetRadius()) >= 0.01f) {
    return false;
  }
  return true;
}
