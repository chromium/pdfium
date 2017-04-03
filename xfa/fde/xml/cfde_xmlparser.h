// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLPARSER_H_
#define XFA_FDE_XML_CFDE_XMLPARSER_H_

#include <memory>
#include <stack>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlsyntaxparser.h"

class CFDE_XMLElement;
class CFDE_XMLNode;
class IFGAS_Stream;
class IFX_Pause;

class CFDE_XMLParser {
 public:
  CFDE_XMLParser(CFDE_XMLNode* pParent,
                 const CFX_RetainPtr<IFGAS_Stream>& pStream);
  ~CFDE_XMLParser();

  int32_t DoParser(IFX_Pause* pPause);

  FX_FILESIZE m_nStart[2];
  size_t m_nSize[2];
  FX_FILESIZE m_nElementStart;
  uint16_t m_dwCheckStatus;
  uint16_t m_dwCurrentCheckStatus;

 private:
  CFX_RetainPtr<IFGAS_Stream> m_pStream;
  std::unique_ptr<CFDE_XMLSyntaxParser> m_pParser;
  CFDE_XMLNode* m_pParent;
  CFDE_XMLNode* m_pChild;
  std::stack<CFDE_XMLNode*> m_NodeStack;
  CFX_WideString m_ws1;
  CFX_WideString m_ws2;
  FDE_XmlSyntaxResult m_syntaxParserResult;
};

#endif  // XFA_FDE_XML_CFDE_XMLPARSER_H_
