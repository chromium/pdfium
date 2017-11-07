// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_assistdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_AssistData::CXFA_AssistData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

CXFA_ToolTip CXFA_AssistData::GetToolTip() {
  return CXFA_ToolTip(m_pNode->GetChild(0, XFA_Element::ToolTip, false));
}
