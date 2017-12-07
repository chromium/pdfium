// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_treelist.h"

#include <memory>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/xfa/cjx_treelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_TreeList::CXFA_TreeList(CXFA_Document* pDocument)
    : CXFA_List(pDocument, pdfium::MakeUnique<CJX_TreeList>(this)) {
  m_pDocument->GetScriptContext()->AddToCacheList(
      std::unique_ptr<CXFA_TreeList>(this));
}

CXFA_TreeList::~CXFA_TreeList() {}

CXFA_Node* CXFA_TreeList::NamedItem(const WideStringView& wsName) {
  uint32_t dwHashCode = FX_HashCode_GetW(wsName, false);
  int32_t iCount = GetLength();
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_Node* ret = Item(i);
    if (dwHashCode == ret->GetNameHash())
      return ret;
  }
  return nullptr;
}
