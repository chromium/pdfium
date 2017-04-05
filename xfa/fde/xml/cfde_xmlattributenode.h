// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLATTRIBUTENODE_H_
#define XFA_FDE_XML_CFDE_XMLATTRIBUTENODE_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlnode.h"

class CFDE_XMLAttributeNode : public CFDE_XMLNode {
 public:
  explicit CFDE_XMLAttributeNode(const CFX_WideString& name);
  ~CFDE_XMLAttributeNode() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override = 0;
  std::unique_ptr<CFDE_XMLNode> Clone() override = 0;

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

#endif  // XFA_FDE_XML_CFDE_XMLATTRIBUTENODE_H_
