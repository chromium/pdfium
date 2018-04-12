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

  int32_t Parse();

 private:
  std::unique_ptr<CFX_XMLSyntaxParser> m_pParser;
  CFX_XMLNode* m_pParent;
  CFX_XMLNode* m_pChild;
  std::stack<CFX_XMLNode*> m_NodeStack;
  WideString m_ws1;
};

#endif  // CORE_FXCRT_XML_CFX_XMLPARSER_H_
