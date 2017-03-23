// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_SAXREADERHANDLER_H_
#define XFA_FXFA_CXFA_SAXREADERHANDLER_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfx_saxreader.h"
#include "xfa/fxfa/cxfa_saxcontext.h"

class CXFA_ChecksumContext;

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

 private:
  void UpdateChecksum(bool bCheckSpace);

  CXFA_ChecksumContext* m_pContext;
  CXFA_SAXContext m_SAXContext;
};

#endif  // XFA_FXFA_CXFA_SAXREADERHANDLER_H_
