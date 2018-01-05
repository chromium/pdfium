// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_PARSER_H_
#define CORE_FXCRT_XML_CXML_PARSER_H_

#include <algorithm>
#include <memory>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/xml/cxml_databufacc.h"

class CFX_UTF8Decoder;
class CXML_Element;

class CXML_Parser {
 public:
  CXML_Parser();
  ~CXML_Parser();

  bool Init(const uint8_t* pBuffer, size_t size);
  bool ReadNextBlock();
  bool IsEOF();
  bool HaveAvailData();
  void SkipWhiteSpaces();
  void GetName(ByteString* space, ByteString* name);
  WideString GetAttrValue();
  uint32_t GetCharRef();
  void GetTagName(bool bStartTag,
                  bool* bEndTag,
                  ByteString* space,
                  ByteString* name);
  void SkipLiterals(const ByteStringView& str);
  std::unique_ptr<CXML_Element> ParseElement(CXML_Element* pParent,
                                             bool bStartTag);
  void InsertContentSegment(bool bCDATA,
                            const WideStringView& content,
                            CXML_Element* pElement);
  void InsertCDATASegment(CFX_UTF8Decoder& decoder, CXML_Element* pElement);

 private:
  std::unique_ptr<CXML_Element> ParseElementInternal(CXML_Element* pParent,
                                                     bool bStartTag,
                                                     int nDepth);

  std::unique_ptr<CXML_DataBufAcc> m_pDataAcc;
  FX_FILESIZE m_nOffset;
  const uint8_t* m_pBuffer;
  size_t m_dwBufferSize;
  FX_FILESIZE m_nBufferOffset;
  size_t m_dwIndex;
};

#endif  // CORE_FXCRT_XML_CXML_PARSER_H_
