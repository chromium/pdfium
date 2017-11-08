// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_captiondata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_CaptionData::CXFA_CaptionData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

int32_t CXFA_CaptionData::GetPresence() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Visible;
  m_pNode->JSNode()->TryEnum(XFA_Attribute::Presence, eAttr, true);
  return eAttr;
}

int32_t CXFA_CaptionData::GetPlacementType() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Left;
  m_pNode->JSNode()->TryEnum(XFA_Attribute::Placement, eAttr, true);
  return eAttr;
}

float CXFA_CaptionData::GetReserve() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::Reserve, ms, true);
  return ms.ToUnit(XFA_UNIT_Pt);
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
