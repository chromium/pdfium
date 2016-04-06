// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_data.h"

#include "xfa/fxfa/parser/xfa_object.h"

// Static.
FX_ARGB CXFA_Data::ToColor(const CFX_WideStringC& wsValue) {
  uint8_t r = 0, g = 0, b = 0;
  if (wsValue.GetLength() == 0)
    return 0xff000000;

  int cc = 0;
  const FX_WCHAR* str = wsValue.c_str();
  int len = wsValue.GetLength();
  while (XFA_IsSpace(str[cc]) && cc < len)
    cc++;

  if (cc >= len)
    return 0xff000000;

  while (cc < len) {
    if (str[cc] == ',' || !XFA_IsDigit(str[cc]))
      break;

    r = r * 10 + str[cc] - '0';
    cc++;
  }
  if (cc < len && str[cc] == ',') {
    cc++;
    while (XFA_IsSpace(str[cc]) && cc < len)
      cc++;

    while (cc < len) {
      if (str[cc] == ',' || !XFA_IsDigit(str[cc]))
        break;

      g = g * 10 + str[cc] - '0';
      cc++;
    }
    if (cc < len && str[cc] == ',') {
      cc++;
      while (XFA_IsSpace(str[cc]) && cc < len)
        cc++;

      while (cc < len) {
        if (str[cc] == ',' || !XFA_IsDigit(str[cc]))
          break;

        b = b * 10 + str[cc] - '0';
        cc++;
      }
    }
  }
  return (0xff << 24) | (r << 16) | (g << 8) | b;
}

XFA_ELEMENT CXFA_Data::GetClassID() const {
  return m_pNode ? m_pNode->GetClassID() : XFA_ELEMENT_UNKNOWN;
}

FX_BOOL CXFA_Data::TryMeasure(XFA_ATTRIBUTE eAttr,
                              FX_FLOAT& fValue,
                              FX_BOOL bUseDefault) const {
  CXFA_Measurement ms;
  if (m_pNode->TryMeasure(eAttr, ms, bUseDefault)) {
    fValue = ms.ToUnit(XFA_UNIT_Pt);
    return TRUE;
  }
  return FALSE;
}

FX_BOOL CXFA_Data::SetMeasure(XFA_ATTRIBUTE eAttr, FX_FLOAT fValue) {
  CXFA_Measurement ms(fValue, XFA_UNIT_Pt);
  return m_pNode->SetMeasure(eAttr, ms);
}
