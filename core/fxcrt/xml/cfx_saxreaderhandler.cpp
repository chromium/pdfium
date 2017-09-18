// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_saxreaderhandler.h"

#include <string>

#include "core/fxcrt/cfx_checksumcontext.h"

CFX_SAXReaderHandler::CFX_SAXReaderHandler(CFX_ChecksumContext* pContext)
    : m_pContext(pContext) {
  ASSERT(m_pContext);
}

CFX_SAXReaderHandler::~CFX_SAXReaderHandler() {}

CFX_SAXContext* CFX_SAXReaderHandler::OnTagEnter(
    const ByteStringView& bsTagName,
    CFX_SAXItem::Type eType,
    uint32_t dwStartPos) {
  UpdateChecksum(true);
  if (eType != CFX_SAXItem::Type::Tag &&
      eType != CFX_SAXItem::Type::Instruction) {
    return nullptr;
  }

  m_SAXContext.m_eNode = eType;
  m_SAXContext.m_TextBuf << "<";
  if (eType == CFX_SAXItem::Type::Instruction)
    m_SAXContext.m_TextBuf << "?";

  m_SAXContext.m_TextBuf << bsTagName;
  m_SAXContext.m_bsTagName = bsTagName;
  return &m_SAXContext;
}

void CFX_SAXReaderHandler::OnTagAttribute(CFX_SAXContext* pTag,
                                          const ByteStringView& bsAttri,
                                          const ByteStringView& bsValue) {
  if (!pTag)
    return;
  pTag->m_TextBuf << " " << bsAttri << "=\"" << bsValue << "\"";
}

void CFX_SAXReaderHandler::OnTagBreak(CFX_SAXContext* pTag) {
  if (!pTag)
    return;

  pTag->m_TextBuf << ">";
  UpdateChecksum(false);
}

void CFX_SAXReaderHandler::OnTagData(CFX_SAXContext* pTag,
                                     CFX_SAXItem::Type eType,
                                     const ByteStringView& bsData,
                                     uint32_t dwStartPos) {
  if (!pTag)
    return;

  if (eType == CFX_SAXItem::Type::CharData)
    pTag->m_TextBuf << "<![CDATA[";

  pTag->m_TextBuf << bsData;
  if (eType == CFX_SAXItem::Type::CharData)
    pTag->m_TextBuf << "]]>";
}

void CFX_SAXReaderHandler::OnTagClose(CFX_SAXContext* pTag, uint32_t dwEndPos) {
  if (!pTag)
    return;

  if (pTag->m_eNode == CFX_SAXItem::Type::Instruction)
    pTag->m_TextBuf << "?>";
  else if (pTag->m_eNode == CFX_SAXItem::Type::Tag)
    pTag->m_TextBuf << "></" << pTag->m_bsTagName.AsStringView() << ">";

  UpdateChecksum(false);
}

void CFX_SAXReaderHandler::OnTagEnd(CFX_SAXContext* pTag,
                                    const ByteStringView& bsTagName,
                                    uint32_t dwEndPos) {
  if (!pTag)
    return;

  pTag->m_TextBuf << "</" << bsTagName << ">";
  UpdateChecksum(false);
}

void CFX_SAXReaderHandler::OnTargetData(CFX_SAXContext* pTag,
                                        CFX_SAXItem::Type eType,
                                        const ByteStringView& bsData,
                                        uint32_t dwStartPos) {
  if (!pTag && eType != CFX_SAXItem::Type::Comment)
    return;

  if (eType == CFX_SAXItem::Type::Comment) {
    m_SAXContext.m_TextBuf << "<!--" << bsData << "-->";
    UpdateChecksum(false);
  } else {
    pTag->m_TextBuf << " " << bsData;
  }
}

void CFX_SAXReaderHandler::UpdateChecksum(bool bCheckSpace) {
  int32_t iLength = m_SAXContext.m_TextBuf.tellp();
  if (iLength < 1)
    return;

  std::string sBuffer = m_SAXContext.m_TextBuf.str();
  const uint8_t* pBuffer = reinterpret_cast<const uint8_t*>(sBuffer.c_str());
  bool bUpdata = true;
  if (bCheckSpace) {
    bUpdata = false;
    for (int32_t i = 0; i < iLength; i++) {
      bUpdata = (pBuffer[i] > 0x20);
      if (bUpdata)
        break;
    }
  }
  if (bUpdata)
    m_pContext->Update(ByteStringView(pBuffer, iLength));

  m_SAXContext.m_TextBuf.str("");
}
