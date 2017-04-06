// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLSYNTAXPARSER_H_
#define XFA_FDE_XML_CFDE_XMLSYNTAXPARSER_H_

#include <stack>
#include <vector>

#include "core/fxcrt/cfx_blockbuffer.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlnode.h"
#include "xfa/fgas/crt/ifgas_stream.h"

enum class FDE_XmlSyntaxResult {
  None,
  InstructionOpen,
  InstructionClose,
  ElementOpen,
  ElementBreak,
  ElementClose,
  TargetName,
  TagName,
  AttriName,
  AttriValue,
  Text,
  CData,
  TargetData,
  Error,
  EndOfString
};

class CFDE_XMLSyntaxParser {
 public:
  explicit CFDE_XMLSyntaxParser(const CFX_RetainPtr<IFGAS_Stream>& pStream);
  ~CFDE_XMLSyntaxParser();

  FDE_XmlSyntaxResult DoSyntaxParse();

  int32_t GetStatus() const;
  FX_FILESIZE GetCurrentPos() const { return m_ParsedChars + m_Start; }
  FX_FILESIZE GetCurrentBinaryPos() const;
  int32_t GetCurrentNodeNumber() const { return m_iCurrentNodeNum; }
  int32_t GetLastNodeNumber() const { return m_iLastNodeNum; }

  CFX_WideString GetTargetName() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  CFX_WideString GetTagName() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  CFX_WideString GetAttributeName() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  CFX_WideString GetAttributeValue() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  CFX_WideString GetTextData() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  CFX_WideString GetTargetData() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

 protected:
  enum class FDE_XmlSyntaxState {
    Text,
    Node,
    Target,
    Tag,
    AttriName,
    AttriEqualSign,
    AttriQuotation,
    AttriValue,
    Entity,
    EntityDecimal,
    EntityHex,
    CloseInstruction,
    BreakElement,
    CloseElement,
    SkipDeclNode,
    DeclCharData,
    SkipComment,
    SkipCommentOrDecl,
    SkipCData,
    TargetData
  };

  void ParseTextChar(wchar_t ch);

  CFX_RetainPtr<IFGAS_Stream> m_pStream;
  int32_t m_iXMLPlaneSize;
  int32_t m_iCurrentPos;
  int32_t m_iCurrentNodeNum;
  int32_t m_iLastNodeNum;
  int32_t m_iParsedBytes;
  FX_FILESIZE m_ParsedChars;
  std::vector<wchar_t> m_Buffer;
  int32_t m_iBufferChars;
  bool m_bEOS;
  FX_FILESIZE m_Start;  // Start position in m_Buffer
  FX_FILESIZE m_End;    // End position in m_Buffer
  FDE_XMLNODE m_CurNode;
  std::stack<FDE_XMLNODE> m_XMLNodeStack;
  CFX_BlockBuffer m_BlockBuffer;
  int32_t m_iAllocStep;
  wchar_t* m_pCurrentBlock;  // Pointer into CFX_BlockBuffer
  int32_t m_iIndexInBlock;
  int32_t m_iTextDataLength;
  FDE_XmlSyntaxResult m_syntaxParserResult;
  FDE_XmlSyntaxState m_syntaxParserState;
  wchar_t m_wQuotationMark;
  int32_t m_iEntityStart;
  std::stack<wchar_t> m_SkipStack;
  wchar_t m_SkipChar;
};

#endif  // XFA_FDE_XML_CFDE_XMLSYNTAXPARSER_H_
