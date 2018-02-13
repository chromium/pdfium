// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmldoc.h"

#include <utility>
#include <vector>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLDoc::CFX_XMLDoc()
    : m_iStatus(0), m_pRoot(pdfium::MakeUnique<CFX_XMLNode>()) {
  m_pRoot->AppendChild(new CFX_XMLInstruction(L"xml"));
}

CFX_XMLDoc::~CFX_XMLDoc() {}

bool CFX_XMLDoc::LoadXML(std::unique_ptr<CFX_XMLParser> pXMLParser) {
  if (!pXMLParser)
    return false;

  m_iStatus = 0;
  m_pStream.Reset();
  m_pRoot->DeleteChildren();
  m_pXMLParser = std::move(pXMLParser);
  return true;
}

int32_t CFX_XMLDoc::DoLoad() {
  if (m_iStatus < 100)
    m_iStatus = m_pXMLParser->DoParser();

  return m_iStatus;
}

void CFX_XMLDoc::CloseXML() {
  m_pXMLParser.reset();
}

