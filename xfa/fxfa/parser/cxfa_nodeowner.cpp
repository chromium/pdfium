// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodeowner.h"

#include "fxjs/gc/container_trace.h"
#include "third_party/base/check.h"
#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_NodeOwner::CXFA_NodeOwner() = default;

CXFA_NodeOwner::~CXFA_NodeOwner() = default;

void CXFA_NodeOwner::Trace(cppgc::Visitor* visitor) const {
  ContainerTrace(visitor, lists_);
}

void CXFA_NodeOwner::PersistList(CXFA_List* list) {
  DCHECK(list);
  lists_.emplace_back(list);
}
