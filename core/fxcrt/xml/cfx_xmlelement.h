// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLELEMENT_H_
#define CORE_FXCRT_XML_CFX_XMLELEMENT_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_xmlattributenode.h"

class CFX_XMLElement : public CFX_XMLAttributeNode {
 public:
  explicit CFX_XMLElement(const WideString& wsTag);
  ~CFX_XMLElement() override;

  // CFX_XMLNode
  FX_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFX_XMLNode> Clone() override;

  WideString GetLocalTagName() const;
  WideString GetNamespacePrefix() const;
  WideString GetNamespaceURI() const;

  WideString GetTextData() const;
  void SetTextData(const WideString& wsText);
};

#endif  // CORE_FXCRT_XML_CFX_XMLELEMENT_H_
