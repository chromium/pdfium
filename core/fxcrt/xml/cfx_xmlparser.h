// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLPARSER_H_
#define CORE_FXCRT_XML_CFX_XMLPARSER_H_

#include <memory>
#include <stack>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"

class CFX_XMLDocument;
class CFX_XMLElement;
class CFX_XMLNode;
class IFX_SeekableReadStream;

class CFX_XMLParser {
 public:
  static bool IsXMLNameChar(wchar_t ch, bool bFirstChar);

  explicit CFX_XMLParser(const RetainPtr<IFX_SeekableReadStream>& pStream);
  virtual ~CFX_XMLParser();

  std::unique_ptr<CFX_XMLDocument> Parse();

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
    CloseInstruction,
    BreakElement,
    CloseElement,
    SkipDeclNode,
    SkipComment,
    SkipCommentOrDecl,
    SkipCData,
    TargetData
  };

  bool DoSyntaxParse(CFX_XMLDocument* doc);
  WideString GetTextData();
  void ProcessTextChar(wchar_t ch);
  void ProcessTargetData();

  CFX_XMLNode* current_node_ = nullptr;
  WideString current_attribute_name_;
  RetainPtr<IFX_SeekableReadStream> m_pStream;
  FX_FILESIZE m_Start = 0;  // Start position in m_Buffer
  FX_FILESIZE m_End = 0;    // End position in m_Buffer
  FDE_XmlSyntaxState m_syntaxParserState = FDE_XmlSyntaxState::Text;
  std::stack<FX_XMLNODETYPE> m_XMLNodeTypeStack;
  std::stack<wchar_t> m_SkipStack;
  std::vector<wchar_t> m_Buffer;
  std::vector<wchar_t> current_text_;
  size_t m_iXMLPlaneSize = 1024;
  int32_t m_iEntityStart = -1;
  wchar_t m_wQuotationMark = 0;
  wchar_t m_SkipChar = 0;
};

#endif  // CORE_FXCRT_XML_CFX_XMLPARSER_H_
