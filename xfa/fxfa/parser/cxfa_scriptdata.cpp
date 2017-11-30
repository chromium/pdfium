// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_scriptdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ScriptData::CXFA_ScriptData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

CXFA_ScriptData::Type CXFA_ScriptData::GetContentType() const {
  pdfium::Optional<WideString> cData =
      m_pNode->JSNode()->TryCData(XFA_Attribute::ContentType, false);
  if (!cData || *cData == L"application/x-formcalc")
    return Type::Formcalc;
  if (*cData == L"application/x-javascript")
    return Type::Javascript;
  return Type::Unknown;
}

XFA_AttributeEnum CXFA_ScriptData::GetRunAt() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::RunAt);
}

WideString CXFA_ScriptData::GetExpression() const {
  return m_pNode->JSNode()->GetContent(false);
}
