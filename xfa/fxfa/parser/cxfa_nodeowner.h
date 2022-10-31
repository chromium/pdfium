// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
#define XFA_FXFA_PARSER_CXFA_NODEOWNER_H_

#include <vector>

#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"

class CXFA_List;

class CXFA_NodeOwner : public cppgc::GarbageCollected<CXFA_NodeOwner> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_NodeOwner();

  void Trace(cppgc::Visitor* visitor) const;
  void PersistList(CXFA_List* list);

 private:
  CXFA_NodeOwner();

  std::vector<cppgc::Member<CXFA_List>> lists_;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODEOWNER_H_
