// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_imagedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ImageData::CXFA_ImageData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

XFA_AttributeEnum CXFA_ImageData::GetAspect() const {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Aspect);
}

WideString CXFA_ImageData::GetContentType() const {
  return m_pNode->JSNode()
      ->TryCData(XFA_Attribute::ContentType, true)
      .value_or(L"");
}

WideString CXFA_ImageData::GetHref() const {
  return m_pNode->JSNode()->TryCData(XFA_Attribute::Href, true).value_or(L"");
}

XFA_AttributeEnum CXFA_ImageData::GetTransferEncoding() const {
  return static_cast<XFA_AttributeEnum>(
      m_pNode->JSNode()->GetEnum(XFA_Attribute::TransferEncoding));
}

WideString CXFA_ImageData::GetContent() const {
  return m_pNode->JSNode()->TryContent(false, true).value_or(L"");
}

void CXFA_ImageData::SetContentType(const WideString& wsContentType) {
  m_pNode->JSNode()->SetCData(XFA_Attribute::ContentType, wsContentType, false,
                              false);
}

void CXFA_ImageData::SetHref(const WideString& wsHref) {
  m_pNode->JSNode()->SetCData(XFA_Attribute::Href, wsHref, false, false);
}

void CXFA_ImageData::SetTransferEncoding(XFA_AttributeEnum iTransferEncoding) {
  m_pNode->JSNode()->SetEnum(XFA_Attribute::TransferEncoding, iTransferEncoding,
                             false);
}
