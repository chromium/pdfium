// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_SAXREADERHANDLER_H_
#define CORE_FXCRT_XML_CFX_SAXREADERHANDLER_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_saxcontext.h"
#include "core/fxcrt/xml/cfx_saxreader.h"

class CFX_ChecksumContext;

class CFX_SAXReaderHandler {
 public:
  explicit CFX_SAXReaderHandler(CFX_ChecksumContext* pContext);
  ~CFX_SAXReaderHandler();

  CFX_SAXContext* OnTagEnter(const CFX_ByteStringC& bsTagName,
                             CFX_SAXItem::Type eType,
                             uint32_t dwStartPos);
  void OnTagAttribute(CFX_SAXContext* pTag,
                      const CFX_ByteStringC& bsAttri,
                      const CFX_ByteStringC& bsValue);
  void OnTagBreak(CFX_SAXContext* pTag);
  void OnTagData(CFX_SAXContext* pTag,
                 CFX_SAXItem::Type eType,
                 const CFX_ByteStringC& bsData,
                 uint32_t dwStartPos);
  void OnTagClose(CFX_SAXContext* pTag, uint32_t dwEndPos);
  void OnTagEnd(CFX_SAXContext* pTag,
                const CFX_ByteStringC& bsTagName,
                uint32_t dwEndPos);

  void OnTargetData(CFX_SAXContext* pTag,
                    CFX_SAXItem::Type eType,
                    const CFX_ByteStringC& bsData,
                    uint32_t dwStartPos);

 private:
  void UpdateChecksum(bool bCheckSpace);

  CFX_ChecksumContext* m_pContext;
  CFX_SAXContext m_SAXContext;
};

#endif  // CORE_FXCRT_XML_CFX_SAXREADERHANDLER_H_
