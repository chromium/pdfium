// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLTEXT_H_
#define XFA_FDE_XML_CFDE_XMLTEXT_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlnode.h"

class CFDE_XMLText : public CFDE_XMLNode {
 public:
  explicit CFDE_XMLText(const CFX_WideString& wsText);
  ~CFDE_XMLText() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFDE_XMLNode> Clone() override;

  CFX_WideString GetText() const { return m_wsText; }
  void SetText(const CFX_WideString& wsText) { m_wsText = wsText; }

 private:
  CFX_WideString m_wsText;
};

#endif  // XFA_FDE_XML_CFDE_XMLTEXT_H_
