// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLPARSER_H_
#define CORE_FXCRT_XML_CFX_XMLPARSER_H_

#include <memory>
#include <stack>
#include <vector>

#include "core/fxcrt/cfx_blockbuffer.h"
#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"

class CFX_XMLElement;
class CFX_XMLNode;
class IFX_SeekableStream;

enum class FX_XmlSyntaxResult {
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

class CFX_XMLParser {
 public:
  static bool IsXMLNameChar(wchar_t ch, bool bFirstChar);

  CFX_XMLParser(CFX_XMLNode* pParent,
                const RetainPtr<IFX_SeekableStream>& pStream);
  virtual ~CFX_XMLParser();

  bool Parse();

 protected:
  FX_XmlSyntaxResult DoSyntaxParse();

  WideString GetTagName() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  WideString GetAttributeName() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  WideString GetAttributeValue() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

  WideString GetTextData() const {
    return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
  }

 private:
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
  bool GetStatus() const;

  CFX_XMLNode* m_pParent;
  CFX_XMLNode* m_pChild;
  std::stack<CFX_XMLNode*> m_NodeStack;
  WideString m_ws1;

  RetainPtr<CFX_SeekableStreamProxy> m_pStream;
  size_t m_iXMLPlaneSize;
  FX_FILESIZE m_iCurrentPos;
  std::vector<wchar_t> m_Buffer;
  bool m_bEOS;
  FX_FILESIZE m_Start;  // Start position in m_Buffer
  FX_FILESIZE m_End;    // End position in m_Buffer
  FX_XMLNODETYPE m_CurNodeType;
  std::stack<FX_XMLNODETYPE> m_XMLNodeTypeStack;
  CFX_BlockBuffer m_BlockBuffer;
  wchar_t* m_pCurrentBlock;  // Pointer into CFX_BlockBuffer
  size_t m_iIndexInBlock;
  int32_t m_iTextDataLength;
  FX_XmlSyntaxResult m_syntaxParserResult;
  FDE_XmlSyntaxState m_syntaxParserState;
  wchar_t m_wQuotationMark;
  int32_t m_iEntityStart;
  std::stack<wchar_t> m_SkipStack;
  wchar_t m_SkipChar;
};

#endif  // CORE_FXCRT_XML_CFX_XMLPARSER_H_
