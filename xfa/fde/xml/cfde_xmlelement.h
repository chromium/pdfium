// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLELEMENT_H_
#define XFA_FDE_XML_CFDE_XMLELEMENT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlattributenode.h"

class CFDE_XMLElement : public CFDE_XMLAttributeNode {
 public:
  explicit CFDE_XMLElement(const CFX_WideString& wsTag);
  ~CFDE_XMLElement() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFDE_XMLNode> Clone() override;

  CFX_WideString GetLocalTagName() const;
  CFX_WideString GetNamespacePrefix() const;
  CFX_WideString GetNamespaceURI() const;

  CFX_WideString GetTextData() const;
  void SetTextData(const CFX_WideString& wsText);
};

#endif  // XFA_FDE_XML_CFDE_XMLELEMENT_H_
