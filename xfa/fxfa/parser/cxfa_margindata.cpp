// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margindata.h"

CXFA_MarginData::CXFA_MarginData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

float CXFA_MarginData::GetLeftInset() const {
  float left = 0;
  TryLeftInset(left);
  return left;
}

float CXFA_MarginData::GetTopInset() const {
  float top = 0;
  TryTopInset(top);
  return top;
}

float CXFA_MarginData::GetRightInset() const {
  float right = 0;
  TryRightInset(right);
  return right;
}

float CXFA_MarginData::GetBottomInset() const {
  float bottom = 0;
  TryBottomInset(bottom);
  return bottom;
}

bool CXFA_MarginData::TryLeftInset(float& fInset) const {
  return TryMeasure(XFA_Attribute::LeftInset, fInset);
}

bool CXFA_MarginData::TryTopInset(float& fInset) const {
  return TryMeasure(XFA_Attribute::TopInset, fInset);
}

bool CXFA_MarginData::TryRightInset(float& fInset) const {
  return TryMeasure(XFA_Attribute::RightInset, fInset);
}

bool CXFA_MarginData::TryBottomInset(float& fInset) const {
  return TryMeasure(XFA_Attribute::BottomInset, fInset);
}
