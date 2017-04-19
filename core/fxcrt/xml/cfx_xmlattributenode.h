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
  explicit CFX_XMLAttributeNode(const CFX_WideString& name);
  ~CFX_XMLAttributeNode() override;

  // CFX_XMLNode
  FX_XMLNODETYPE GetType() const override = 0;
  std::unique_ptr<CFX_XMLNode> Clone() override = 0;

  CFX_WideString GetName() const { return name_; }
  const std::map<CFX_WideString, CFX_WideString>& GetAttributes() const {
    return attrs_;
  }
  void SetAttributes(const std::map<CFX_WideString, CFX_WideString>& attrs) {
    attrs_ = attrs;
  }
  bool HasAttribute(const CFX_WideString& name) const;

  void SetString(const CFX_WideString& name, const CFX_WideString& value);
  CFX_WideString GetString(const CFX_WideString& name) const;

  void RemoveAttribute(const CFX_WideString& name);

 private:
  CFX_WideString name_;
  std::map<CFX_WideString, CFX_WideString> attrs_;
};

#endif  // CORE_FXCRT_XML_CFX_XMLATTRIBUTENODE_H_
