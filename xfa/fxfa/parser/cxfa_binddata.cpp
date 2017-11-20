// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_binddata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_BindData::CXFA_BindData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

WideString CXFA_BindData::GetPicture() {
  CXFA_Node* pPicture = m_pNode->GetChild(0, XFA_Element::Picture, false);
  if (!pPicture)
    return L"";
  return pPicture->JSNode()->GetContent(false);
}
