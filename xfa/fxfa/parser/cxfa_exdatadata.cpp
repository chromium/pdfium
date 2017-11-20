// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exdatadata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ExDataData::CXFA_ExDataData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

void CXFA_ExDataData::SetContentType(const WideString& wsContentType) {
  m_pNode->JSNode()->SetCData(XFA_Attribute::ContentType, wsContentType, false,
                              false);
}
