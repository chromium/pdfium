// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLDOC_H_
#define CORE_FXCRT_XML_CFX_XMLDOC_H_

#include <memory>
#include <utility>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"

class CFX_XMLDoc {
 public:
  CFX_XMLDoc();
  ~CFX_XMLDoc();

  bool Load(const RetainPtr<CFX_SeekableStreamProxy>& pStream);

  std::unique_ptr<CFX_XMLNode> GetTree() { return std::move(m_pRoot); }

 private:
  std::unique_ptr<CFX_XMLNode> m_pRoot;
};

#endif  // CORE_FXCRT_XML_CFX_XMLDOC_H_
