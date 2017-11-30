// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_linedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

XFA_AttributeEnum CXFA_LineData::GetHand() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Hand);
}

bool CXFA_LineData::GetSlope() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Slope) ==
         XFA_AttributeEnum::Slash;
}

CXFA_EdgeData CXFA_LineData::GetEdgeData() const {
  return CXFA_EdgeData(m_pNode->GetChild(0, XFA_Element::Edge, false));
}
