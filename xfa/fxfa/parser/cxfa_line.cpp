// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_line.h"

#include "xfa/fxfa/parser/cxfa_node.h"

int32_t CXFA_Line::GetHand() {
  return m_pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_Hand);
}

bool CXFA_Line::GetSlope() {
  return m_pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_Slope) ==
         XFA_ATTRIBUTEENUM_Slash;
}

CXFA_EdgeData CXFA_Line::GetEdgeData() {
  return CXFA_EdgeData(m_pNode->GetChild(0, XFA_Element::Edge, false));
}
