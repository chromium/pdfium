// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLNODE_H_
#define CORE_FXCRT_XML_CFX_XMLNODE_H_

#include <memory>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"

enum FX_XMLNODETYPE {
  FX_XMLNODE_Instruction = 0,
  FX_XMLNODE_Element,
  FX_XMLNODE_Text,
  FX_XMLNODE_CharData,
};

class CFX_XMLNode {
 public:
  CFX_XMLNode();
  virtual ~CFX_XMLNode();

  virtual FX_XMLNODETYPE GetType() const = 0;
  virtual std::unique_ptr<CFX_XMLNode> Clone() = 0;
  virtual void Save(const RetainPtr<IFX_SeekableWriteStream>& pXMLStream) = 0;

  CFX_XMLNode* GetRoot();
  CFX_XMLNode* GetParent() const { return parent_.Get(); }
  CFX_XMLNode* GetFirstChild() const { return first_child_.get(); }
  CFX_XMLNode* GetNextSibling() const { return next_sibling_.get(); }

  void AppendChild(std::unique_ptr<CFX_XMLNode> pNode);
  void InsertChildNode(std::unique_ptr<CFX_XMLNode> pNode, int32_t index);
  void RemoveChildNode(CFX_XMLNode* pNode);
  void DeleteChildren();

  CFX_XMLNode* GetLastChildForTesting() const { return last_child_.Get(); }
  CFX_XMLNode* GetPrevSiblingForTesting() const { return prev_sibling_.Get(); }

 protected:
  WideString EncodeEntities(const WideString& value);

 private:
  // A node owns its first child and it owns its next sibling. The rest
  // are unowned pointers.
  UnownedPtr<CFX_XMLNode> parent_;
  UnownedPtr<CFX_XMLNode> last_child_;
  UnownedPtr<CFX_XMLNode> prev_sibling_;
  std::unique_ptr<CFX_XMLNode> first_child_;
  std::unique_ptr<CFX_XMLNode> next_sibling_;
};

#endif  // CORE_FXCRT_XML_CFX_XMLNODE_H_
