// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_captiondata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_CaptionData::CXFA_CaptionData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

bool CXFA_CaptionData::IsVisible() const {
  return m_pNode->JSNode()
             ->TryEnum(XFA_Attribute::Presence, true)
             .value_or(XFA_AttributeEnum::Visible) ==
         XFA_AttributeEnum::Visible;
}

bool CXFA_CaptionData::IsHidden() const {
  return m_pNode->JSNode()
             ->TryEnum(XFA_Attribute::Presence, true)
             .value_or(XFA_AttributeEnum::Visible) == XFA_AttributeEnum::Hidden;
}

XFA_AttributeEnum CXFA_CaptionData::GetPlacementType() const {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::Placement, true)
      .value_or(XFA_AttributeEnum::Left);
}

float CXFA_CaptionData::GetReserve() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::Reserve)
      .ToUnit(XFA_Unit::Pt);
}

CXFA_MarginData CXFA_CaptionData::GetMarginData() const {
  return CXFA_MarginData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Margin, false) : nullptr);
}

CXFA_FontData CXFA_CaptionData::GetFontData() const {
  return CXFA_FontData(m_pNode ? m_pNode->GetChild(0, XFA_Element::Font, false)
                               : nullptr);
}

CXFA_ValueData CXFA_CaptionData::GetValueData() const {
  return CXFA_ValueData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Value, false) : nullptr);
}
