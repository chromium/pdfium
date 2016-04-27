// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_INCLUDE_XFA_CHECKSUM_H_
#define XFA_FXFA_INCLUDE_XFA_CHECKSUM_H_

#include "xfa/fgas/xml/fgas_sax.h"
#include "xfa/fxfa/include/fxfa.h"

class CXFA_SAXReaderHandler;
class CXFA_ChecksumContext;

class CXFA_SAXContext {
 public:
  CXFA_SAXContext() : m_eNode(FX_SAXNODE_Unknown) {}

  CFX_ByteTextBuf m_TextBuf;
  CFX_ByteString m_bsTagName;
  FX_SAXNODE m_eNode;
};

class CXFA_SAXReaderHandler {
 public:
  CXFA_SAXReaderHandler(CXFA_ChecksumContext* pContext);
  ~CXFA_SAXReaderHandler();

  void* OnTagEnter(const CFX_ByteStringC& bsTagName,
                   FX_SAXNODE eType,
                   uint32_t dwStartPos);
  void OnTagAttribute(void* pTag,
                      const CFX_ByteStringC& bsAttri,
                      const CFX_ByteStringC& bsValue);
  void OnTagBreak(void* pTag);
  void OnTagData(void* pTag,
                 FX_SAXNODE eType,
                 const CFX_ByteStringC& bsData,
                 uint32_t dwStartPos);
  void OnTagClose(void* pTag, uint32_t dwEndPos);
  void OnTagEnd(void* pTag,
                const CFX_ByteStringC& bsTagName,
                uint32_t dwEndPos);

  void OnTargetData(void* pTag,
                    FX_SAXNODE eType,
                    const CFX_ByteStringC& bsData,
                    uint32_t dwStartPos);

 protected:
  void UpdateChecksum(FX_BOOL bCheckSpace);

  CXFA_ChecksumContext* m_pContext;
  CXFA_SAXContext m_SAXContext;
};

class CXFA_ChecksumContext {
 public:
  CXFA_ChecksumContext();
  ~CXFA_ChecksumContext();

  void Release() { delete this; }
  void StartChecksum();
  FX_BOOL UpdateChecksum(IFX_FileRead* pSrcFile,
                         FX_FILESIZE offset = 0,
                         size_t size = 0);
  void FinishChecksum();
  void GetChecksum(CFX_ByteString& bsChecksum);
  void Update(const CFX_ByteStringC& bsText);

 protected:
  CFX_SAXReader* m_pSAXReader;
  uint8_t* m_pByteContext;
  CFX_ByteString m_bsChecksum;
};

#endif  // XFA_FXFA_INCLUDE_XFA_CHECKSUM_H_
