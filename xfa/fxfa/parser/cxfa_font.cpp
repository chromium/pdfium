// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_font.h"

#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_filldata.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_Font::CXFA_Font(CXFA_Node* pNode) : CXFA_Data(pNode) {}

float CXFA_Font::GetBaselineShift() {
  return m_pNode->JSNode()
      ->GetMeasure(XFA_ATTRIBUTE_BaselineShift)
      .ToUnit(XFA_UNIT_Pt);
}

float CXFA_Font::GetHorizontalScale() {
  WideString wsValue;
  m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_FontHorizontalScale, wsValue, true);
  int32_t iScale = FXSYS_wtoi(wsValue.c_str());
  return iScale > 0 ? (float)iScale : 100.0f;
}

float CXFA_Font::GetVerticalScale() {
  WideString wsValue;
  m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_FontVerticalScale, wsValue, true);
  int32_t iScale = FXSYS_wtoi(wsValue.c_str());
  return iScale > 0 ? (float)iScale : 100.0f;
}

float CXFA_Font::GetLetterSpacing() {
  WideStringView wsValue;
  if (!m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_LetterSpacing, wsValue, true))
    return 0;

  CXFA_Measurement ms(wsValue);
  if (ms.GetUnit() == XFA_UNIT_Em)
    return ms.GetValue() * GetFontSize();
  return ms.ToUnit(XFA_UNIT_Pt);
}

int32_t CXFA_Font::GetLineThrough() {
  int32_t iValue = 0;
  m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_LineThrough, iValue, true);
  return iValue;
}

int32_t CXFA_Font::GetUnderline() {
  int32_t iValue = 0;
  m_pNode->JSNode()->TryInteger(XFA_ATTRIBUTE_Underline, iValue, true);
  return iValue;
}

int32_t CXFA_Font::GetUnderlinePeriod() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_All;
  m_pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_UnderlinePeriod, eAttr, true);
  return eAttr;
}

float CXFA_Font::GetFontSize() {
  CXFA_Measurement ms;
  m_pNode->JSNode()->TryMeasure(XFA_ATTRIBUTE_Size, ms, true);
  return ms.ToUnit(XFA_UNIT_Pt);
}

void CXFA_Font::GetTypeface(WideStringView& wsTypeFace) {
  m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_Typeface, wsTypeFace, true);
}

bool CXFA_Font::IsBold() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Normal;
  m_pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Weight, eAttr, true);
  return eAttr == XFA_ATTRIBUTEENUM_Bold;
}

bool CXFA_Font::IsItalic() {
  XFA_ATTRIBUTEENUM eAttr = XFA_ATTRIBUTEENUM_Normal;
  m_pNode->JSNode()->TryEnum(XFA_ATTRIBUTE_Posture, eAttr, true);
  return eAttr == XFA_ATTRIBUTEENUM_Italic;
}

void CXFA_Font::SetColor(FX_ARGB color) {
  CXFA_FillData(m_pNode->JSNode()->GetProperty(0, XFA_Element::Fill, true))
      .SetColor(color);
}

FX_ARGB CXFA_Font::GetColor() {
  CXFA_FillData fillData(m_pNode->GetChild(0, XFA_Element::Fill, false));
  return fillData ? fillData.GetColor(true) : 0xFF000000;
}
