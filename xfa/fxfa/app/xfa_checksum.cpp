// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/include/xfa_checksum.h"

#include "core/fdrm/crypto/include/fx_crypt.h"
#include "xfa/fgas/crt/fgas_algorithm.h"

CXFA_SAXReaderHandler::CXFA_SAXReaderHandler(CXFA_ChecksumContext* pContext)
    : m_pContext(pContext) {
  ASSERT(m_pContext);
}
CXFA_SAXReaderHandler::~CXFA_SAXReaderHandler() {}
void* CXFA_SAXReaderHandler::OnTagEnter(const CFX_ByteStringC& bsTagName,
                                        FX_SAXNODE eType,
                                        uint32_t dwStartPos) {
  UpdateChecksum(TRUE);
  if (eType != FX_SAXNODE_Tag && eType != FX_SAXNODE_Instruction) {
    return NULL;
  }
  m_SAXContext.m_eNode = eType;
  CFX_ByteTextBuf& textBuf = m_SAXContext.m_TextBuf;
  textBuf << "<";
  if (eType == FX_SAXNODE_Instruction) {
    textBuf << "?";
  }
  textBuf << bsTagName;
  m_SAXContext.m_bsTagName = bsTagName;
  return &m_SAXContext;
}
void CXFA_SAXReaderHandler::OnTagAttribute(void* pTag,
                                           const CFX_ByteStringC& bsAttri,
                                           const CFX_ByteStringC& bsValue) {
  if (pTag == NULL) {
    return;
  }
  CFX_ByteTextBuf& textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
  textBuf << " " << bsAttri << "=\"" << bsValue << "\"";
}
void CXFA_SAXReaderHandler::OnTagBreak(void* pTag) {
  if (pTag == NULL) {
    return;
  }
  CFX_ByteTextBuf& textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
  textBuf << ">";
  UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTagData(void* pTag,
                                      FX_SAXNODE eType,
                                      const CFX_ByteStringC& bsData,
                                      uint32_t dwStartPos) {
  if (pTag == NULL) {
    return;
  }
  CFX_ByteTextBuf& textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
  if (eType == FX_SAXNODE_CharData) {
    textBuf << "<![CDATA[";
  }
  textBuf << bsData;
  if (eType == FX_SAXNODE_CharData) {
    textBuf << "]]>";
  }
}
void CXFA_SAXReaderHandler::OnTagClose(void* pTag, uint32_t dwEndPos) {
  if (pTag == NULL) {
    return;
  }
  CXFA_SAXContext* pSAXContext = (CXFA_SAXContext*)pTag;
  CFX_ByteTextBuf& textBuf = pSAXContext->m_TextBuf;
  if (pSAXContext->m_eNode == FX_SAXNODE_Instruction) {
    textBuf << "?>";
  } else if (pSAXContext->m_eNode == FX_SAXNODE_Tag) {
    textBuf << "></" << pSAXContext->m_bsTagName.AsStringC() << ">";
  }
  UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTagEnd(void* pTag,
                                     const CFX_ByteStringC& bsTagName,
                                     uint32_t dwEndPos) {
  if (pTag == NULL) {
    return;
  }
  CFX_ByteTextBuf& textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
  textBuf << "</" << bsTagName << ">";
  UpdateChecksum(FALSE);
}
void CXFA_SAXReaderHandler::OnTargetData(void* pTag,
                                         FX_SAXNODE eType,
                                         const CFX_ByteStringC& bsData,
                                         uint32_t dwStartPos) {
  if (pTag == NULL && eType != FX_SAXNODE_Comment) {
    return;
  }
  if (eType == FX_SAXNODE_Comment) {
    CFX_ByteTextBuf& textBuf = m_SAXContext.m_TextBuf;
    textBuf << "<!--" << bsData << "-->";
    UpdateChecksum(FALSE);
  } else {
    CFX_ByteTextBuf& textBuf = ((CXFA_SAXContext*)pTag)->m_TextBuf;
    textBuf << " " << bsData;
  }
}
void CXFA_SAXReaderHandler::UpdateChecksum(FX_BOOL bCheckSpace) {
  int32_t iLength = m_SAXContext.m_TextBuf.GetLength();
  if (iLength < 1) {
    return;
  }
  uint8_t* pBuffer = m_SAXContext.m_TextBuf.GetBuffer();
  FX_BOOL bUpdata = TRUE;
  if (bCheckSpace) {
    bUpdata = FALSE;
    for (int32_t i = 0; i < iLength; i++) {
      bUpdata = (pBuffer[i] > 0x20);
      if (bUpdata) {
        break;
      }
    }
  }
  if (bUpdata) {
    m_pContext->Update(CFX_ByteStringC(pBuffer, iLength));
  }
  m_SAXContext.m_TextBuf.Clear();
}

CXFA_ChecksumContext::CXFA_ChecksumContext()
    : m_pSAXReader(nullptr), m_pByteContext(nullptr) {}

CXFA_ChecksumContext::~CXFA_ChecksumContext() {
  FinishChecksum();
}

void CXFA_ChecksumContext::StartChecksum() {
  FinishChecksum();
  m_pByteContext = FX_Alloc(uint8_t, 128);
  CRYPT_SHA1Start(m_pByteContext);
  m_bsChecksum.clear();
  m_pSAXReader = new CFX_SAXReader;
}

FX_BOOL CXFA_ChecksumContext::UpdateChecksum(IFX_FileRead* pSrcFile,
                                             FX_FILESIZE offset,
                                             size_t size) {
  if (!m_pSAXReader || !pSrcFile)
    return FALSE;
  if (size < 1)
    size = pSrcFile->GetSize();

  CXFA_SAXReaderHandler handler(this);
  m_pSAXReader->SetHandler(&handler);
  if (m_pSAXReader->StartParse(
          pSrcFile, (uint32_t)offset, (uint32_t)size,
          FX_SAXPARSEMODE_NotSkipSpace | FX_SAXPARSEMODE_NotConvert_amp |
              FX_SAXPARSEMODE_NotConvert_lt | FX_SAXPARSEMODE_NotConvert_gt |
              FX_SAXPARSEMODE_NotConvert_sharp) < 0) {
    return FALSE;
  }
  return m_pSAXReader->ContinueParse(NULL) > 99;
}

void CXFA_ChecksumContext::FinishChecksum() {
  if (m_pSAXReader) {
    m_pSAXReader->Release();
    m_pSAXReader = NULL;
  }
  if (m_pByteContext) {
    uint8_t digest[20];
    FXSYS_memset(digest, 0, 20);
    CRYPT_SHA1Finish(m_pByteContext, digest);
    int32_t nLen = FX_Base64EncodeA(digest, 20, NULL);
    FX_CHAR* pBuffer = m_bsChecksum.GetBuffer(nLen);
    FX_Base64EncodeA(digest, 20, pBuffer);
    m_bsChecksum.ReleaseBuffer(nLen);
    FX_Free(m_pByteContext);
    m_pByteContext = NULL;
  }
}
void CXFA_ChecksumContext::GetChecksum(CFX_ByteString& bsChecksum) {
  bsChecksum = m_bsChecksum;
}
void CXFA_ChecksumContext::Update(const CFX_ByteStringC& bsText) {
  if (m_pByteContext) {
    CRYPT_SHA1Update(m_pByteContext, bsText.raw_str(), bsText.GetLength());
  }
}
