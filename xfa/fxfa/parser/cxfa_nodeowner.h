// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
#define XFA_FXFA_PARSER_CXFA_NODEOWNER_H_

#include <memory>
#include <vector>

class CXFA_List;
class CXFA_Node;

class CXFA_NodeOwner {
 public:
  virtual ~CXFA_NodeOwner();

  // Takes ownership of |node|, returns unowned pointer to it.
  CXFA_Node* AddOwnedNode(std::unique_ptr<CXFA_Node> node);

  // Takes ownership of |list|, returns unowned pointer to it.
  CXFA_List* AddOwnedList(std::unique_ptr<CXFA_List> list);

 protected:
  CXFA_NodeOwner();

  std::vector<std::unique_ptr<CXFA_Node>> nodes_;  // Must outlive |lists_|.
  std::vector<std::unique_ptr<CXFA_List>> lists_;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
