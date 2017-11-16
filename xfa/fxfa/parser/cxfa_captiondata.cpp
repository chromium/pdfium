// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_captiondata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_CaptionData::CXFA_CaptionData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_CaptionData::GetPresence() {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::Presence, true)
      .value_or(XFA_ATTRIBUTEENUM_Visible);
}

int32_t CXFA_CaptionData::GetPlacementType() {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::Placement, true)
      .value_or(XFA_ATTRIBUTEENUM_Left);
}

float CXFA_CaptionData::GetReserve() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::Reserve)
      .ToUnit(XFA_Unit::Pt);
}

CXFA_MarginData CXFA_CaptionData::GetMarginData() {
  return CXFA_MarginData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Margin, false) : nullptr);
}

CXFA_FontData CXFA_CaptionData::GetFontData() {
  return CXFA_FontData(m_pNode ? m_pNode->GetChild(0, XFA_Element::Font, false)
                               : nullptr);
}

CXFA_ValueData CXFA_CaptionData::GetValueData() {
  return CXFA_ValueData(
      m_pNode ? m_pNode->GetChild(0, XFA_Element::Value, false) : nullptr);
}
