// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DATAEXPORTER_H_
#define XFA_FXFA_PARSER_CXFA_DATAEXPORTER_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"

class CXFA_Document;
class CXFA_Node;
class IFX_SeekableStream;
class CFX_SeekableStreamProxy;

class CXFA_DataExporter {
 public:
  explicit CXFA_DataExporter(CXFA_Document* pDocument);
  ~CXFA_DataExporter();

  bool Export(const CFX_RetainPtr<IFX_SeekableStream>& pWrite);
  bool Export(const CFX_RetainPtr<IFX_SeekableStream>& pWrite,
              CXFA_Node* pNode,
              uint32_t dwFlag,
              const char* pChecksum);

 private:
  bool Export(const CFX_RetainPtr<CFX_SeekableStreamProxy>& pStream,
              CXFA_Node* pNode,
              uint32_t dwFlag,
              const char* pChecksum);

  CFX_UnownedPtr<CXFA_Document> const m_pDocument;
};

#endif  // XFA_FXFA_PARSER_CXFA_DATAEXPORTER_H_
