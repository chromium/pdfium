// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLNODE_H_
#define CORE_FXCRT_XML_CFX_XMLNODE_H_

#include <memory>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/retain_ptr.h"

enum FX_XMLNODETYPE {
  FX_XMLNODE_Unknown = 0,
  FX_XMLNODE_Instruction,
  FX_XMLNODE_Element,
  FX_XMLNODE_Text,
  FX_XMLNODE_CharData,
};

struct FX_XMLNODE {
  int32_t iNodeNum;
  FX_XMLNODETYPE eNodeType;
};

class CFX_XMLNode {
 public:
  CFX_XMLNode();
  virtual ~CFX_XMLNode();

  virtual FX_XMLNODETYPE GetType() const;
  virtual std::unique_ptr<CFX_XMLNode> Clone();
  virtual void Save(const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream);

  CFX_XMLNode* GetRoot();
  CFX_XMLNode* GetParent() const { return parent_; }
  CFX_XMLNode* GetFirstChild() const { return first_child_; }
  CFX_XMLNode* GetNextSibling() const { return next_sibling_; }

  void AppendChild(CFX_XMLNode* pNode);
  void InsertChildNode(CFX_XMLNode* pNode, int32_t index);
  void RemoveChildNode(CFX_XMLNode* pNode);
  void DeleteChildren();

 protected:
  WideString AttributeToString(const WideString& name, const WideString& value);
  WideString EncodeEntities(const WideString& value);

 private:
  CFX_XMLNode* parent_ = nullptr;
  CFX_XMLNode* next_sibling_ = nullptr;
  CFX_XMLNode* prev_sibling_ = nullptr;
  CFX_XMLNode* first_child_ = nullptr;
  CFX_XMLNode* last_child_ = nullptr;
};

#endif  // CORE_FXCRT_XML_CFX_XMLNODE_H_
