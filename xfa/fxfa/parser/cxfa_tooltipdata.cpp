// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_tooltipdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ToolTipData::CXFA_ToolTipData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

bool CXFA_ToolTipData::GetTip(WideString& wsTip) {
  return m_pNode->JSNode()->TryContent(wsTip, false, true);
}
