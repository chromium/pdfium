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
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::LineHeight)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginLeft() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::MarginLeft)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginRight() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::MarginRight)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceAbove() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::SpaceAbove)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceBelow() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::SpaceBelow)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetTextIndent() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::TextIndent)
      .ToUnit(XFA_Unit::Pt);
}
