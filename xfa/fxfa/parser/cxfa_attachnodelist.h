// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_
#define XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_

#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/parser/cxfa_treelist.h"

class CXFA_Document;
class CXFA_Node;

class CXFA_AttachNodeList final : public CXFA_TreeList {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_AttachNodeList() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_TreeList:
  size_t GetLength() override;
  bool Append(CXFA_Node* pNode) override;
  bool Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) override;
  void Remove(CXFA_Node* pNode) override;
  CXFA_Node* Item(size_t iIndex) override;

 private:
  CXFA_AttachNodeList(CXFA_Document* pDocument, CXFA_Node* pAttachNode);

  cppgc::Member<CXFA_Node> const m_pAttachNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_
