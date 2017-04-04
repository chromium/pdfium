// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLNODE_H_
#define XFA_FDE_XML_CFDE_XMLNODE_H_

#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "xfa/fgas/crt/ifgas_stream.h"

enum FDE_XMLNODETYPE {
  FDE_XMLNODE_Unknown = 0,
  FDE_XMLNODE_Instruction,
  FDE_XMLNODE_Element,
  FDE_XMLNODE_Text,
  FDE_XMLNODE_CharData,
};

struct FDE_XMLNODE {
  int32_t iNodeNum;
  FDE_XMLNODETYPE eNodeType;
};

class CFDE_XMLNode {
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

  CFDE_XMLNode();
  virtual ~CFDE_XMLNode();

  virtual FDE_XMLNODETYPE GetType() const;
  virtual std::unique_ptr<CFDE_XMLNode> Clone();

  int32_t CountChildNodes() const;
  CFDE_XMLNode* GetChildNode(int32_t index) const;
  int32_t GetChildNodeIndex(CFDE_XMLNode* pNode) const;
  int32_t InsertChildNode(CFDE_XMLNode* pNode, int32_t index = -1);
  void RemoveChildNode(CFDE_XMLNode* pNode);
  void DeleteChildren();

  CFDE_XMLNode* GetPath(const wchar_t* pPath,
                        int32_t iLength = -1,
                        bool bQualifiedName = true) const;

  int32_t GetNodeLevel() const;
  CFDE_XMLNode* GetNodeItem(CFDE_XMLNode::NodeItem eItem) const;
  bool InsertNodeItem(CFDE_XMLNode::NodeItem eItem, CFDE_XMLNode* pNode);
  CFDE_XMLNode* RemoveNodeItem(CFDE_XMLNode::NodeItem eItem);

  void SaveXMLNode(const CFX_RetainPtr<IFGAS_Stream>& pXMLStream);

  CFDE_XMLNode* m_pParent;
  CFDE_XMLNode* m_pChild;
  CFDE_XMLNode* m_pPrior;
  CFDE_XMLNode* m_pNext;
};

#endif  // XFA_FDE_XML_CFDE_XMLNODE_H_
