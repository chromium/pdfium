// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_margin.h"

CXFA_Margin::CXFA_Margin(CXFA_Node* pNode) : CXFA_Data(pNode) {}

bool CXFA_Margin::GetLeftInset(float& fInset, float fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_LeftInset, fInset);
}

bool CXFA_Margin::GetTopInset(float& fInset, float fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_TopInset, fInset);
}

bool CXFA_Margin::GetRightInset(float& fInset, float fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_RightInset, fInset);
}

bool CXFA_Margin::GetBottomInset(float& fInset, float fDefInset) const {
  fInset = fDefInset;
  return TryMeasure(XFA_ATTRIBUTE_BottomInset, fInset);
}
