// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DOCUMENT_PARSER_H_
#define XFA_FXFA_PARSER_CXFA_DOCUMENT_PARSER_H_

#include <memory>

#include "xfa/fxfa/parser/cxfa_simple_parser.h"

class CFX_XMLDoc;
class CXFA_Document;
class CXFA_FFNotify;
class CXFA_Notify;
class IFX_SeekableStream;

class CXFA_DocumentParser {
 public:
  explicit CXFA_DocumentParser(CXFA_FFNotify* pNotify);
  ~CXFA_DocumentParser();

  int32_t StartParse(const RetainPtr<IFX_SeekableStream>& pStream,
                     XFA_PacketType ePacketID);
  int32_t DoParse();

  CFX_XMLDoc* GetXMLDoc() const;
  CXFA_FFNotify* GetNotify() const;
  CXFA_Document* GetDocument() const;

 private:
  UnownedPtr<CXFA_FFNotify> const m_pNotify;
  std::unique_ptr<CXFA_Document> m_pDocument;
  CXFA_SimpleParser m_nodeParser;
};

#endif  // XFA_FXFA_PARSER_CXFA_DOCUMENT_PARSER_H_
