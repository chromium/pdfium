// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margindata.h"

CXFA_MarginData::CXFA_MarginData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

float CXFA_MarginData::GetLeftInset() const {
  return TryLeftInset().value_or(0);
}

float CXFA_MarginData::GetTopInset() const {
  return TryTopInset().value_or(0);
}

float CXFA_MarginData::GetRightInset() const {
  return TryRightInset().value_or(0);
}

float CXFA_MarginData::GetBottomInset() const {
  return TryBottomInset().value_or(0);
}

pdfium::Optional<float> CXFA_MarginData::TryLeftInset() const {
  float value = 0;
  if (TryMeasure(XFA_Attribute::LeftInset, value))
    return {value};
  return {};
}

pdfium::Optional<float> CXFA_MarginData::TryTopInset() const {
  float value = 0;
  if (TryMeasure(XFA_Attribute::TopInset, value))
    return {value};
  return {};
}

pdfium::Optional<float> CXFA_MarginData::TryRightInset() const {
  float value = 0;
  if (TryMeasure(XFA_Attribute::RightInset, value))
    return {value};
  return {};
}

pdfium::Optional<float> CXFA_MarginData::TryBottomInset() const {
  float value = 0;
  if (TryMeasure(XFA_Attribute::BottomInset, value))
    return {value};
  return {};
}
