// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLNODE_H_
#define CORE_FXCRT_XML_CFX_XMLNODE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"

enum FX_XMLNODETYPE {
  FX_XMLNODE_Unknown = 0,
  FX_XMLNODE_Instruction,
  FX_XMLNODE_Element,
  FX_XMLNODE_Text,
  FX_XMLNODE_CharData,
};

class CFX_XMLNode {
 public:
  using const_iterator =
      std::vector<std::unique_ptr<CFX_XMLNode>>::const_iterator;

  CFX_XMLNode();
  virtual ~CFX_XMLNode();

  virtual FX_XMLNODETYPE GetType() const = 0;
  virtual std::unique_ptr<CFX_XMLNode> Clone() = 0;
  virtual void Save(const RetainPtr<IFX_SeekableWriteStream>& pXMLStream) = 0;

  CFX_XMLNode* GetRoot();
  CFX_XMLNode* GetParent() const { return parent_.Get(); }
  CFX_XMLNode* GetNextSibling() const { return next_sibling_; }
  bool HasChildren() const { return !children_.empty(); }
  const_iterator begin() const { return children_.begin(); }
  const_iterator end() const { return children_.end(); }

  void AppendChild(std::unique_ptr<CFX_XMLNode> pNode);
  void InsertChildNode(std::unique_ptr<CFX_XMLNode> pNode, int32_t index);
  void RemoveChildNode(CFX_XMLNode* pNode);
  void DeleteChildren();
  // Note |root| must not have any children.
  void MoveChildrenTo(CFX_XMLNode* root);

 protected:
  WideString EncodeEntities(const WideString& value);

 private:
  UnownedPtr<CFX_XMLNode> parent_;
  // The next_sibling is owned by the vector. We don't use an UnownedPtr
  // because we don't know the destruction order of the vector.
  CFX_XMLNode* next_sibling_;
  std::vector<std::unique_ptr<CFX_XMLNode>> children_;
};

#endif  // CORE_FXCRT_XML_CFX_XMLNODE_H_
