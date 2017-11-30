// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_strokedata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

bool CXFA_StrokeData::IsVisible() const {
  if (!m_pNode)
    return false;

  XFA_AttributeEnum presence = m_pNode->JSNode()
                                   ->TryEnum(XFA_Attribute::Presence, true)
                                   .value_or(XFA_AttributeEnum::Visible);
  return presence == XFA_AttributeEnum::Visible;
}

XFA_AttributeEnum CXFA_StrokeData::GetCapType() const {
  if (!m_pNode)
    return XFA_AttributeEnum::Square;
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Cap);
}

XFA_AttributeEnum CXFA_StrokeData::GetStrokeType() const {
  return m_pNode ? m_pNode->JSNode()->GetEnum(XFA_Attribute::Stroke)
                 : XFA_AttributeEnum::Solid;
}

float CXFA_StrokeData::GetThickness() const {
  return GetMSThickness().ToUnit(XFA_Unit::Pt);
}

CXFA_Measurement CXFA_StrokeData::GetMSThickness() const {
  return m_pNode ? m_pNode->JSNode()->GetMeasure(XFA_Attribute::Thickness)
                 : CXFA_Measurement(0.5, XFA_Unit::Pt);
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

  return CXFA_DataData::ToColor(
      pNode->JSNode()->GetCData(XFA_Attribute::Value).AsStringView());
}

void CXFA_StrokeData::SetColor(FX_ARGB argb) {
  if (!m_pNode)
    return;

  CXFA_Node* pNode =
      m_pNode->JSNode()->GetProperty(0, XFA_Element::Color, true);
  int a;
  int r;
  int g;
  int b;
  std::tie(a, r, g, b) = ArgbDecode(argb);
  pNode->JSNode()->SetCData(XFA_Attribute::Value,
                            WideString::Format(L"%d,%d,%d", r, g, b), false,
                            false);
}

XFA_AttributeEnum CXFA_StrokeData::GetJoinType() const {
  return m_pNode ? m_pNode->JSNode()->GetEnum(XFA_Attribute::Join)
                 : XFA_AttributeEnum::Square;
}

bool CXFA_StrokeData::IsInverted() const {
  return m_pNode ? m_pNode->JSNode()->GetBoolean(XFA_Attribute::Inverted)
                 : false;
}

float CXFA_StrokeData::GetRadius() const {
  return m_pNode ? m_pNode->JSNode()
                       ->TryMeasure(XFA_Attribute::Radius, true)
                       .value_or(CXFA_Measurement(0, XFA_Unit::In))
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
