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

CFX_XMLDoc::CFX_XMLDoc() : m_pRoot(pdfium::MakeUnique<CFX_XMLNode>()) {
  m_pRoot->AppendChild(new CFX_XMLInstruction(L"xml"));
}

CFX_XMLDoc::~CFX_XMLDoc() {}

bool CFX_XMLDoc::Load(const RetainPtr<CFX_SeekableStreamProxy>& pStream) {
  ASSERT(pStream);
  CFX_XMLParser parser(m_pRoot.get(), pStream);
  return parser.Parse();
}
