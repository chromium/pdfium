// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_
#define XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_

#include "xfa/fxfa/parser/cxfa_nodelist.h"

class CXFA_Document;
class CXFA_Node;

class CXFA_AttachNodeList : public CXFA_NodeList {
 public:
  CXFA_AttachNodeList(CXFA_Document* pDocument, CXFA_Node* pAttachNode);

  // From CXFA_NodeList.
  int32_t GetLength() override;
  bool Append(CXFA_Node* pNode) override;
  bool Insert(CXFA_Node* pNewNode, CXFA_Node* pBeforeNode) override;
  bool Remove(CXFA_Node* pNode) override;
  CXFA_Node* Item(int32_t iIndex) override;

 private:
  CXFA_Node* m_pAttachNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_ATTACHNODELIST_H_
