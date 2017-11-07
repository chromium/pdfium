// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_imagedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ImageData::CXFA_ImageData(CXFA_Node* pNode, bool bDefValue)
    : CXFA_Data(pNode), m_bDefValue(bDefValue) {}

int32_t CXFA_ImageData::GetAspect() {
  return m_pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_Aspect);
}

bool CXFA_ImageData::GetContentType(WideString& wsContentType) {
  return m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_ContentType, wsContentType,
                                     true);
}

bool CXFA_ImageData::GetHref(WideString& wsHref) {
  if (m_bDefValue)
    return m_pNode->JSNode()->TryCData(XFA_ATTRIBUTE_Href, wsHref, true);
  return m_pNode->JSNode()->GetAttribute(L"href", wsHref, true);
}

int32_t CXFA_ImageData::GetTransferEncoding() {
  if (m_bDefValue)
    return m_pNode->JSNode()->GetEnum(XFA_ATTRIBUTE_TransferEncoding);
  return XFA_ATTRIBUTEENUM_Base64;
}

bool CXFA_ImageData::GetContent(WideString& wsText) {
  return m_pNode->JSNode()->TryContent(wsText, false, true);
}

bool CXFA_ImageData::SetContentType(const WideString& wsContentType) {
  return m_pNode->JSNode()->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType,
                                     false, false);
}

bool CXFA_ImageData::SetHref(const WideString& wsHref) {
  if (m_bDefValue)
    return m_pNode->JSNode()->SetCData(XFA_ATTRIBUTE_Href, wsHref, false,
                                       false);
  return m_pNode->JSNode()->SetAttribute(XFA_ATTRIBUTE_Href,
                                         wsHref.AsStringView(), false);
}

bool CXFA_ImageData::SetTransferEncoding(int32_t iTransferEncoding) {
  if (m_bDefValue) {
    return m_pNode->JSNode()->SetEnum(XFA_ATTRIBUTE_TransferEncoding,
                                      (XFA_ATTRIBUTEENUM)iTransferEncoding,
                                      false);
  }
  return true;
}
