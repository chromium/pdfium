// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodeowner.h"

#include <utility>

#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_NodeOwner::CXFA_NodeOwner() = default;

CXFA_NodeOwner::~CXFA_NodeOwner() = default;

void CXFA_NodeOwner::Trace(cppgc::Visitor* visitor) const {
  for (const auto& list : lists_)
    visitor->Trace(list);
}

void CXFA_NodeOwner::PersistList(CXFA_List* list) {
  ASSERT(list);
  lists_.emplace_back(list);
}
