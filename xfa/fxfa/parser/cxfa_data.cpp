// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_data.h"

#include "core/fxcrt/fx_extension.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

// Static.
FX_ARGB CXFA_Data::ToColor(const CFX_WideStringC& wsValue) {
  uint8_t r = 0, g = 0, b = 0;
  if (wsValue.GetLength() == 0)
    return 0xff000000;

  int cc = 0;
  const wchar_t* str = wsValue.unterminated_c_str();
  int len = wsValue.GetLength();
  while (FXSYS_iswspace(str[cc]) && cc < len)
    cc++;

  if (cc >= len)
    return 0xff000000;

  while (cc < len) {
    if (str[cc] == ',' || !FXSYS_isDecimalDigit(str[cc]))
      break;

    r = r * 10 + str[cc] - '0';
    cc++;
  }
  if (cc < len && str[cc] == ',') {
    cc++;
    while (FXSYS_iswspace(str[cc]) && cc < len)
      cc++;

    while (cc < len) {
      if (str[cc] == ',' || !FXSYS_isDecimalDigit(str[cc]))
        break;

      g = g * 10 + str[cc] - '0';
      cc++;
    }
    if (cc < len && str[cc] == ',') {
      cc++;
      while (FXSYS_iswspace(str[cc]) && cc < len)
        cc++;

      while (cc < len) {
        if (str[cc] == ',' || !FXSYS_isDecimalDigit(str[cc]))
          break;

        b = b * 10 + str[cc] - '0';
        cc++;
      }
    }
  }
  return (0xff << 24) | (r << 16) | (g << 8) | b;
}

XFA_Element CXFA_Data::GetElementType() const {
  return m_pNode ? m_pNode->GetElementType() : XFA_Element::Unknown;
}

bool CXFA_Data::TryMeasure(XFA_ATTRIBUTE eAttr,
                           float& fValue,
                           bool bUseDefault) const {
  CXFA_Measurement ms;
  if (m_pNode->TryMeasure(eAttr, ms, bUseDefault)) {
    fValue = ms.ToUnit(XFA_UNIT_Pt);
    return true;
  }
  return false;
}

bool CXFA_Data::SetMeasure(XFA_ATTRIBUTE eAttr, float fValue) {
  CXFA_Measurement ms(fValue, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(eAttr, ms);
}
