// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_paradata.h"

#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ParaData::CXFA_ParaData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

XFA_AttributeEnum CXFA_ParaData::GetHorizontalAlign() const {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::HAlign, true)
      .value_or(XFA_AttributeEnum::Left);
}

XFA_AttributeEnum CXFA_ParaData::GetVerticalAlign() const {
  return m_pNode->JSNode()
      ->TryEnum(XFA_Attribute::VAlign, true)
      .value_or(XFA_AttributeEnum::Top);
}

float CXFA_ParaData::GetLineHeight() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::LineHeight)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginLeft() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::MarginLeft)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetMarginRight() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::MarginRight)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceAbove() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::SpaceAbove)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetSpaceBelow() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::SpaceBelow)
      .ToUnit(XFA_Unit::Pt);
}

float CXFA_ParaData::GetTextIndent() const {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_Attribute::TextIndent)
      .ToUnit(XFA_Unit::Pt);
}
