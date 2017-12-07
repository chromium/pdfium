// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_treelist.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_treelist.h"

const CJX_MethodSpec CJX_TreeList::MethodSpecs[] = {
    {"namedItem", namedItem_static},
    {"", nullptr}};

CJX_TreeList::CJX_TreeList(CXFA_TreeList* list) : CJX_List(list) {
  DefineMethods(MethodSpecs);
}

CJX_TreeList::~CJX_TreeList() {}

CXFA_TreeList* CJX_TreeList::GetXFATreeList() {
  return static_cast<CXFA_TreeList*>(GetXFAObject());
}

void CJX_TreeList::namedItem(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"namedItem");
    return;
  }

  ByteString szName = pArguments->GetUTF8String(0);
  CXFA_Node* pNode = GetXFATreeList()->NamedItem(
      WideString::FromUTF8(szName.AsStringView()).AsStringView());
  if (!pNode)
    return;

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNode));
}
