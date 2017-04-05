// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLDOC_H_
#define XFA_FDE_XML_CFDE_XMLDOC_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fde/xml/cfde_xmlnode.h"
#include "xfa/fde/xml/cfde_xmlparser.h"
#include "xfa/fgas/crt/ifgas_stream.h"

class CFDE_XMLDoc {
 public:
  CFDE_XMLDoc();
  ~CFDE_XMLDoc();

  bool LoadXML(std::unique_ptr<CFDE_XMLParser> pXMLParser);
  int32_t DoLoad(IFX_Pause* pPause);
  void CloseXML();

  CFDE_XMLNode* GetRoot() const { return m_pRoot.get(); }
  void SaveXMLNode(const CFX_RetainPtr<IFGAS_Stream>& pXMLStream,
                   CFDE_XMLNode* pNode);

 private:
  int32_t m_iStatus;
  std::unique_ptr<CFDE_XMLNode> m_pRoot;
  std::unique_ptr<CFDE_XMLParser> m_pXMLParser;
  CFX_RetainPtr<IFGAS_Stream> m_pStream;
};

#endif  // XFA_FDE_XML_CFDE_XMLDOC_H_
