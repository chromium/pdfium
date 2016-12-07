// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_INT_H_
#define CORE_FXCRT_XML_INT_H_

#include <algorithm>

#include "core/fxcrt/fx_stream.h"

class CFX_UTF8Decoder;
class CXML_Element;

class CXML_Parser {
 public:
  CXML_Parser();
  ~CXML_Parser();

  bool Init(uint8_t* pBuffer, size_t size);
  bool Init(const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead);
  bool Init(const CFX_RetainPtr<IFX_BufferedReadStream>& pBuffer);
  bool Init();
  bool ReadNextBlock();
  bool IsEOF();
  bool HaveAvailData();
  void SkipWhiteSpaces();
  void GetName(CFX_ByteString& space, CFX_ByteString& name);
  void GetAttrValue(CFX_WideString& value);
  uint32_t GetCharRef();
  void GetTagName(CFX_ByteString& space,
                  CFX_ByteString& name,
                  bool& bEndTag,
                  bool bStartTag = false);
  void SkipLiterals(const CFX_ByteStringC& str);
  CXML_Element* ParseElement(CXML_Element* pParent, bool bStartTag = false);
  void InsertContentSegment(bool bCDATA,
                            const CFX_WideStringC& content,
                            CXML_Element* pElement);
  void InsertCDATASegment(CFX_UTF8Decoder& decoder, CXML_Element* pElement);

  CFX_RetainPtr<IFX_BufferedReadStream> m_pDataAcc;
  FX_FILESIZE m_nOffset;
  bool m_bSaveSpaceChars;
  const uint8_t* m_pBuffer;
  size_t m_dwBufferSize;
  FX_FILESIZE m_nBufferOffset;
  size_t m_dwIndex;
};

void FX_XML_SplitQualifiedName(const CFX_ByteStringC& bsFullName,
                               CFX_ByteStringC& bsSpace,
                               CFX_ByteStringC& bsName);

#endif  // CORE_FXCRT_XML_INT_H_
