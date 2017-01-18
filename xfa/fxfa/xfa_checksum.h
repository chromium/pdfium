// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_XFA_CHECKSUM_H_
#define XFA_FXFA_XFA_CHECKSUM_H_

#include <memory>

#include "core/fdrm/crypto/fx_crypt.h"
#include "xfa/fde/xml/cfx_saxreader.h"
#include "xfa/fxfa/fxfa.h"

class CXFA_SAXReaderHandler;
class CXFA_ChecksumContext;

class CXFA_SAXContext {
 public:
  CXFA_SAXContext() : m_eNode(CFX_SAXItem::Type::Unknown) {}

  CFX_ByteTextBuf m_TextBuf;
  CFX_ByteString m_bsTagName;
  CFX_SAXItem::Type m_eNode;
};

class CXFA_SAXReaderHandler {
 public:
  explicit CXFA_SAXReaderHandler(CXFA_ChecksumContext* pContext);
  ~CXFA_SAXReaderHandler();

  CXFA_SAXContext* OnTagEnter(const CFX_ByteStringC& bsTagName,
                              CFX_SAXItem::Type eType,
                              uint32_t dwStartPos);
  void OnTagAttribute(CXFA_SAXContext* pTag,
                      const CFX_ByteStringC& bsAttri,
                      const CFX_ByteStringC& bsValue);
  void OnTagBreak(CXFA_SAXContext* pTag);
  void OnTagData(CXFA_SAXContext* pTag,
                 CFX_SAXItem::Type eType,
                 const CFX_ByteStringC& bsData,
                 uint32_t dwStartPos);
  void OnTagClose(CXFA_SAXContext* pTag, uint32_t dwEndPos);
  void OnTagEnd(CXFA_SAXContext* pTag,
                const CFX_ByteStringC& bsTagName,
                uint32_t dwEndPos);

  void OnTargetData(CXFA_SAXContext* pTag,
                    CFX_SAXItem::Type eType,
                    const CFX_ByteStringC& bsData,
                    uint32_t dwStartPos);

 protected:
  void UpdateChecksum(bool bCheckSpace);

  CXFA_ChecksumContext* m_pContext;
  CXFA_SAXContext m_SAXContext;
};

class CXFA_ChecksumContext {
 public:
  CXFA_ChecksumContext();
  ~CXFA_ChecksumContext();

  void StartChecksum();
  void Update(const CFX_ByteStringC& bsText);
  bool UpdateChecksum(const CFX_RetainPtr<IFX_SeekableReadStream>& pSrcFile,
                      FX_FILESIZE offset = 0,
                      size_t size = 0);
  void FinishChecksum();
  CFX_ByteString GetChecksum() const;

 protected:
  std::unique_ptr<CFX_SAXReader> m_pSAXReader;
  std::unique_ptr<CRYPT_sha1_context> m_pByteContext;
  CFX_ByteString m_bsChecksum;
};

#endif  // XFA_FXFA_XFA_CHECKSUM_H_
