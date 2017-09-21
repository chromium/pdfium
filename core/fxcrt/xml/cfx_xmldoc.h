// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLDOC_H_
#define CORE_FXCRT_XML_CFX_XMLDOC_H_

#include <memory>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"

class CFX_XMLDoc {
 public:
  CFX_XMLDoc();
  ~CFX_XMLDoc();

  bool LoadXML(std::unique_ptr<CFX_XMLParser> pXMLParser);
  int32_t DoLoad();
  void CloseXML();

  CFX_XMLNode* GetRoot() const { return m_pRoot.get(); }
  void SaveXMLNode(const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream,
                   CFX_XMLNode* pNode);

 private:
  int32_t m_iStatus;
  std::unique_ptr<CFX_XMLNode> m_pRoot;
  std::unique_ptr<CFX_XMLParser> m_pXMLParser;
  RetainPtr<CFX_SeekableStreamProxy> m_pStream;
};

#endif  // CORE_FXCRT_XML_CFX_XMLDOC_H_
