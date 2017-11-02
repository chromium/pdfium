// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_caption.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_Caption::CXFA_Caption(CXFA_Node* pNode) : CXFA_Data(pNode) {}

int32_t CXFA_Caption::GetPresence() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Visible;
  m_pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Presence, eAttr, true);
  return eAttr;
}

int32_t CXFA_Caption::GetPlacementType() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Left;
  m_pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Placement, eAttr, true);
  return eAttr;
}

float CXFA_Caption::GetReserve() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_ATTRIBUTE_Reserve, ms, true);
  return ms.ToUnit(XFA_UNIT_Pt);
}

CXFA_Margin CXFA_Caption::GetMargin() {
  return CXFA_Margin(m_pNode ? m_pNode->GetChild(0, XFA_Element::Margin)
                             : nullptr);
}

CXFA_Font CXFA_Caption::GetFont() {
  return CXFA_Font(m_pNode ? m_pNode->GetChild(0, XFA_Element::Font) : nullptr);
}

CXFA_Value CXFA_Caption::GetValue() {
  return CXFA_Value(m_pNode ? m_pNode->GetChild(0, XFA_Element::Value)
                            : nullptr);
}
