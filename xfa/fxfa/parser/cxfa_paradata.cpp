// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_paradata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ParaData::CXFA_ParaData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

int32_t CXFA_ParaData::GetHorizontalAlign() {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::HAlign, true)
      .value_or(XFA_ATTRIBUTEENUM_Left);
}

int32_t CXFA_ParaData::GetVerticalAlign() {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::VAlign, true)
      .value_or(XFA_ATTRIBUTEENUM_Top);
}

float CXFA_ParaData::GetLineHeight() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::LineHeight, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginLeft() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::MarginLeft, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginRight() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::MarginRight, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceAbove() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::SpaceAbove, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceBelow() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::SpaceBelow, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetTextIndent() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_Attribute::TextIndent, ms, true);
  return ms.ToUnit(XFA_Unit::Pt);
}
