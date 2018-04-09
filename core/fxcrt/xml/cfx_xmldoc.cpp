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

CFX_XMLDoc::CFX_XMLDoc(const RetainPtr<CFX_SeekableStreamProxy>& pStream)
    : m_iStatus(0),
      m_pRoot(pdfium::MakeUnique<CFX_XMLNode>()),
      m_pXMLParser(pdfium::MakeUnique<CFX_XMLParser>(m_pRoot.get(), pStream)) {
  ASSERT(pStream);

  m_pRoot->AppendChild(new CFX_XMLInstruction(L"xml"));
}

CFX_XMLDoc::~CFX_XMLDoc() {}

int32_t CFX_XMLDoc::DoLoad() {
  if (m_iStatus < 100)
    m_iStatus = m_pXMLParser->DoParser();

  return m_iStatus;
}

void CFX_XMLDoc::CloseXML() {
  m_pXMLParser.reset();
}
