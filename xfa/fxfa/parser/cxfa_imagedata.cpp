// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_imagedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ImageData::CXFA_ImageData(CXFA_Node* pNode, bool bDefValue)
    : CXFA_DataData(pNode), m_bDefValue(bDefValue) {}

int32_t CXFA_ImageData::GetAspect() {
  return m_pNode->JSNode()->GetEnum(XFA_Attribute::Aspect);
}

bool CXFA_ImageData::GetContentType(WideString& wsContentType) {
  pdfium::Optional<WideString> content =
      m_pNode->JSNode()->TryCData(XFA_Attribute::ContentType, true);
  if (!content)
    return false;

  wsContentType = *content;
  return true;
}

bool CXFA_ImageData::GetHref(WideString& wsHref) {
  if (m_bDefValue) {
    pdfium::Optional<WideString> ret =
        m_pNode->JSNode()->TryCData(XFA_Attribute::Href, true);
    if (!ret)
      return false;

    wsHref = *ret;
    return true;
  }
  return m_pNode->JSNode()->GetAttribute(XFA_Attribute::Href, wsHref, true);
}

XFA_ATTRIBUTEENUM CXFA_ImageData::GetTransferEncoding() {
  if (m_bDefValue) {
    return static_cast<XFA_ATTRIBUTEENUM>(
        m_pNode->JSNode()->GetEnum(XFA_Attribute::TransferEncoding));
  }
  return XFA_ATTRIBUTEENUM_Base64;
}

bool CXFA_ImageData::GetContent(WideString& wsText) {
  pdfium::Optional<WideString> ret = m_pNode->JSNode()->TryContent(false, true);
  if (!ret)
    return false;

  wsText = *ret;
  return true;
}

bool CXFA_ImageData::SetContentType(const WideString& wsContentType) {
  return m_pNode->JSNode()->SetCData(XFA_Attribute::ContentType, wsContentType,
                                     false, false);
}

bool CXFA_ImageData::SetHref(const WideString& wsHref) {
  if (m_bDefValue)
    return m_pNode->JSNode()->SetCData(XFA_Attribute::Href, wsHref, false,
                                       false);
  return m_pNode->JSNode()->SetAttribute(XFA_Attribute::Href,
                                         wsHref.AsStringView(), false);
}

bool CXFA_ImageData::SetTransferEncoding(XFA_ATTRIBUTEENUM iTransferEncoding) {
  if (m_bDefValue) {
    return m_pNode->JSNode()->SetEnum(XFA_Attribute::TransferEncoding,
                                      iTransferEncoding, false);
  }
  return true;
}
