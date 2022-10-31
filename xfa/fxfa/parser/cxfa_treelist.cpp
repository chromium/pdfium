// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_treelist.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cjx_treelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_TreeList::CXFA_TreeList(CXFA_Document* doc)
    : CXFA_List(doc,
                XFA_ObjectType::TreeList,
                XFA_Element::TreeList,
                cppgc::MakeGarbageCollected<CJX_TreeList>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_TreeList::~CXFA_TreeList() = default;

CXFA_Node* CXFA_TreeList::NamedItem(WideStringView wsName) {
  uint32_t dwHashCode = FX_HashCode_GetW(wsName);
  size_t count = GetLength();
  for (size_t i = 0; i < count; i++) {
    CXFA_Node* ret = Item(i);
    if (dwHashCode == ret->GetNameHash())
      return ret;
  }
  return nullptr;
}
