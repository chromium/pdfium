// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datadata.h"

#include "core/fxcrt/fx_extension.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"

// Static.
FX_ARGB CXFA_DataData::ToColor(const WideStringView& wsValue) {
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

CXFA_DataData::CXFA_DataData(CXFA_Node* pNode) : m_pNode(pNode) {}

CXFA_DataData::~CXFA_DataData() {}

XFA_Element CXFA_DataData::GetElementType() const {
  return m_pNode ? m_pNode->GetElementType() : XFA_Element::Unknown;
}

pdfium::Optional<float> CXFA_DataData::TryMeasureAsFloat(
    XFA_Attribute attr) const {
  pdfium::Optional<CXFA_Measurement> measure =
      m_pNode->JSNode()->TryMeasure(attr, false);
  if (measure)
    return {measure->ToUnit(XFA_Unit::Pt)};
  return {};
}
