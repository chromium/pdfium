// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_XML_FGAS_SAX_H_
#define XFA_FGAS_XML_FGAS_SAX_H_

#include "core/fxcrt/include/fx_basic.h"

#define FX_SAXPARSEMODE_NotConvert_amp 0x0001
#define FX_SAXPARSEMODE_NotConvert_lt 0x0002
#define FX_SAXPARSEMODE_NotConvert_gt 0x0004
#define FX_SAXPARSEMODE_NotConvert_apos 0x0008
#define FX_SAXPARSEMODE_NotConvert_quot 0x0010
#define FX_SAXPARSEMODE_NotConvert_sharp 0x0020
#define FX_SAXPARSEMODE_NotSkipSpace 0x0100

enum FX_SAXNODE {
  FX_SAXNODE_Unknown = 0,
  FX_SAXNODE_Instruction,
  FX_SAXNODE_Declaration,
  FX_SAXNODE_Comment,
  FX_SAXNODE_Tag,
  FX_SAXNODE_Text,
  FX_SAXNODE_CharData,
};

class IFX_SAXReaderHandler {
 public:
  virtual ~IFX_SAXReaderHandler() {}
  virtual void* OnTagEnter(const CFX_ByteStringC& bsTagName,
                           FX_SAXNODE eType,
                           uint32_t dwStartPos) = 0;
  virtual void OnTagAttribute(void* pTag,
                              const CFX_ByteStringC& bsAttri,
                              const CFX_ByteStringC& bsValue) = 0;
  virtual void OnTagBreak(void* pTag) = 0;
  virtual void OnTagData(void* pTag,
                         FX_SAXNODE eType,
                         const CFX_ByteStringC& bsData,
                         uint32_t dwStartPos) = 0;
  virtual void OnTagClose(void* pTag, uint32_t dwEndPos) = 0;
  virtual void OnTagEnd(void* pTag,
                        const CFX_ByteStringC& bsTagName,
                        uint32_t dwEndPos) = 0;
  virtual void OnTargetData(void* pTag,
                            FX_SAXNODE eType,
                            const CFX_ByteStringC& bsData,
                            uint32_t dwStartPos) = 0;
};

class IFX_SAXReader {
 public:
  virtual ~IFX_SAXReader() {}
  virtual void Release() = 0;
  virtual int32_t StartParse(IFX_FileRead* pFile,
                             uint32_t dwStart = 0,
                             uint32_t dwLen = -1,
                             uint32_t dwParseMode = 0) = 0;
  virtual int32_t ContinueParse(IFX_Pause* pPause = NULL) = 0;
  virtual void SkipCurrentNode() = 0;
  virtual void SetHandler(IFX_SAXReaderHandler* pHandler) = 0;
};
IFX_SAXReader* FX_SAXReader_Create();

#endif  // XFA_FGAS_XML_FGAS_SAX_H_
