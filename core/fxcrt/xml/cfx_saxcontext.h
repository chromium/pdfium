// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_SAXCONTEXT_H_
#define CORE_FXCRT_XML_CFX_SAXCONTEXT_H_

#include <sstream>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_saxreader.h"

class CFX_SAXContext {
 public:
  CFX_SAXContext();
  ~CFX_SAXContext();

  std::ostringstream m_TextBuf;
  ByteString m_bsTagName;
  CFX_SAXItem::Type m_eNode;
};

#endif  // CORE_FXCRT_XML_CFX_SAXCONTEXT_H_
