// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_document_parser.h"

#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_DocumentParser::CXFA_DocumentParser(CXFA_FFNotify* pNotify)
    : m_pNotify(pNotify) {}

CXFA_DocumentParser::~CXFA_DocumentParser() {
  m_pDocument->ReleaseXMLNodesIfNeeded();
}

bool CXFA_DocumentParser::Parse(const RetainPtr<IFX_SeekableStream>& pStream,
                                XFA_PacketType ePacketID) {
  m_pDocument = pdfium::MakeUnique<CXFA_Document>(GetNotify());

  // Note, we don't pass the document into the constructor as currently that
  // triggers different behaviour in the parser.
  CXFA_SimpleParser parser;
  parser.SetFactory(m_pDocument.get());

  if (!parser.Parse(pStream, ePacketID))
    return false;

  m_pXMLRoot = parser.GetXMLRoot();

  m_pDocument->SetRoot(parser.GetRootNode());
  return true;
}

CXFA_FFNotify* CXFA_DocumentParser::GetNotify() const {
  return m_pNotify.Get();
}

CXFA_Document* CXFA_DocumentParser::GetDocument() const {
  return m_pDocument.get();
}
