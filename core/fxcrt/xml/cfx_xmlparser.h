// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLPARSER_H_
#define CORE_FXCRT_XML_CFX_XMLPARSER_H_

#include <memory>
#include <stack>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_xmlsyntaxparser.h"

class CFX_XMLElement;
class CFX_XMLNode;
class CFX_SeekableStreamProxy;

class CFX_XMLParser {
 public:
  CFX_XMLParser(CFX_XMLNode* pParent,
                const RetainPtr<CFX_SeekableStreamProxy>& pStream);
  ~CFX_XMLParser();

  int32_t DoParser();

  FX_FILESIZE m_nStart[2];
  size_t m_nSize[2];
  FX_FILESIZE m_nElementStart;
  uint16_t m_dwCheckStatus;
  uint16_t m_dwCurrentCheckStatus;

 private:
  RetainPtr<CFX_SeekableStreamProxy> m_pStream;
  std::unique_ptr<CFX_XMLSyntaxParser> m_pParser;
  CFX_XMLNode* m_pParent;
  CFX_XMLNode* m_pChild;
  std::stack<CFX_XMLNode*> m_NodeStack;
  WideString m_ws1;
  WideString m_ws2;
  FX_XmlSyntaxResult m_syntaxParserResult;
};

#endif  // CORE_FXCRT_XML_CFX_XMLPARSER_H_
