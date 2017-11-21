// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_textdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_TextData::CXFA_TextData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

WideString CXFA_TextData::GetContent() const {
  return m_pNode->JSNode()->GetContent(false);
}
