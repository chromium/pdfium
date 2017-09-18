// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLATTRIBUTENODE_H_
#define CORE_FXCRT_XML_CFX_XMLATTRIBUTENODE_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"

class CFX_XMLAttributeNode : public CFX_XMLNode {
 public:
  explicit CFX_XMLAttributeNode(const WideString& name);
  ~CFX_XMLAttributeNode() override;

  // CFX_XMLNode
  FX_XMLNODETYPE GetType() const override = 0;
  std::unique_ptr<CFX_XMLNode> Clone() override = 0;

  WideString GetName() const { return name_; }
  const std::map<WideString, WideString>& GetAttributes() const {
    return attrs_;
  }
  void SetAttributes(const std::map<WideString, WideString>& attrs) {
    attrs_ = attrs;
  }
  bool HasAttribute(const WideString& name) const;

  void SetString(const WideString& name, const WideString& value);
  WideString GetString(const WideString& name) const;

  void RemoveAttribute(const WideString& name);

 private:
  WideString name_;
  std::map<WideString, WideString> attrs_;
};

#endif  // CORE_FXCRT_XML_CFX_XMLATTRIBUTENODE_H_
