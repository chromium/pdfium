// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_SAXCONTEXT_H_
#define XFA_FXFA_CXFA_SAXCONTEXT_H_

class CXFA_SAXContext {
 public:
  CXFA_SAXContext() : m_eNode(CFX_SAXItem::Type::Unknown) {}

  CFX_ByteTextBuf m_TextBuf;
  CFX_ByteString m_bsTagName;
  CFX_SAXItem::Type m_eNode;
};

#endif  // XFA_FXFA_CXFA_SAXCONTEXT_H_
