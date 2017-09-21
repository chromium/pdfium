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
  enum NodeItem {
    Root = 0,
    Parent,
    FirstSibling,
    PriorSibling,
    NextSibling,
    LastSibling,
    FirstNeighbor,
    PriorNeighbor,
    NextNeighbor,
    LastNeighbor,
    FirstChild,
    LastChild
  };

  CFX_XMLNode();
  virtual ~CFX_XMLNode();

  virtual FX_XMLNODETYPE GetType() const;
  virtual std::unique_ptr<CFX_XMLNode> Clone();

  int32_t CountChildNodes() const;
  CFX_XMLNode* GetChildNode(int32_t index) const;
  int32_t GetChildNodeIndex(CFX_XMLNode* pNode) const;
  int32_t InsertChildNode(CFX_XMLNode* pNode, int32_t index = -1);
  void RemoveChildNode(CFX_XMLNode* pNode);
  void DeleteChildren();

  CFX_XMLNode* GetPath(const wchar_t* pPath,
                       int32_t iLength = -1,
                       bool bQualifiedName = true) const;

  int32_t GetNodeLevel() const;
  CFX_XMLNode* GetNodeItem(CFX_XMLNode::NodeItem eItem) const;
  bool InsertNodeItem(CFX_XMLNode::NodeItem eItem, CFX_XMLNode* pNode);
  CFX_XMLNode* RemoveNodeItem(CFX_XMLNode::NodeItem eItem);

  void SaveXMLNode(const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream);

  CFX_XMLNode* m_pParent;
  CFX_XMLNode* m_pChild;
  CFX_XMLNode* m_pPrior;
  CFX_XMLNode* m_pNext;
};

#endif  // CORE_FXCRT_XML_CFX_XMLNODE_H_
