// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_image.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_Image::CXFA_Image(CXFA_Node* pNode, bool bDefValue)
    : CXFA_Data(pNode), m_bDefValue(bDefValue) {}

int32_t CXFA_Image::GetAspect() {
  return m_pNode->GetEnum(XFA_ATTRIBUTE_Aspect);
}

bool CXFA_Image::GetContentType(WideString& wsContentType) {
  return m_pNode->TryCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}

bool CXFA_Image::GetHref(WideString& wsHref) {
  if (m_bDefValue)
    return m_pNode->TryCData(XFA_ATTRIBUTE_Href, wsHref);
  return m_pNode->GetAttribute(L"href", wsHref);
}

int32_t CXFA_Image::GetTransferEncoding() {
  if (m_bDefValue)
    return m_pNode->GetEnum(XFA_ATTRIBUTE_TransferEncoding);
  return XFA_ATTRIBUTEENUM_Base64;
}

bool CXFA_Image::GetContent(WideString& wsText) {
  return m_pNode->TryContent(wsText);
}

bool CXFA_Image::SetContentType(const WideString& wsContentType) {
  return m_pNode->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType);
}

bool CXFA_Image::SetHref(const WideString& wsHref) {
  if (m_bDefValue)
    return m_pNode->SetCData(XFA_ATTRIBUTE_Href, wsHref);
  return m_pNode->SetAttribute(XFA_ATTRIBUTE_Href, wsHref.AsStringView());
}

bool CXFA_Image::SetTransferEncoding(int32_t iTransferEncoding) {
  if (m_bDefValue) {
    return m_pNode->SetEnum(XFA_ATTRIBUTE_TransferEncoding,
                            (XFA_ATTRIBUTEENUM)iTransferEncoding);
  }
  return true;
}
