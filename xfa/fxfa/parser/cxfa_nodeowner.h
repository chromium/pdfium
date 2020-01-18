// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
#define XFA_FXFA_PARSER_CXFA_NODEOWNER_H_

#include <memory>
#include <vector>

class CXFA_Node;

class CXFA_NodeOwner {
 public:
  virtual ~CXFA_NodeOwner();

  CXFA_Node* AddOwnedNode(std::unique_ptr<CXFA_Node> node);
  bool IsBeingDestroyed() const { return is_being_destroyed_; }

 protected:
  CXFA_NodeOwner();

  bool is_being_destroyed_ = false;
  std::vector<std::unique_ptr<CXFA_Node>> nodes_;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
