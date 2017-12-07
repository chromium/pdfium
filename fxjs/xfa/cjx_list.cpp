// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_list.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

const CJX_MethodSpec CJX_List::MethodSpecs[] = {{"append", append_static},
                                                {"insert", insert_static},
                                                {"item", item_static},
                                                {"remove", remove_static},
                                                {"", nullptr}};

CJX_List::CJX_List(CXFA_List* list) : CJX_Object(list) {
  DefineMethods(MethodSpecs);
}

CJX_List::~CJX_List() {}

CXFA_List* CJX_List::GetXFAList() {
  return static_cast<CXFA_List*>(GetXFAObject());
}

void CJX_List::append(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"append");
    return;
  }

  auto* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode) {
    ThrowArgumentMismatchException();
    return;
  }
  GetXFAList()->Append(pNode);
}

void CJX_List::insert(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 2) {
    ThrowParamCountMismatchException(L"insert");
    return;
  }

  auto* pNewNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  auto* pBeforeNode = static_cast<CXFA_Node*>(pArguments->GetObject(1));
  if (!pNewNode) {
    ThrowArgumentMismatchException();
    return;
  }
  GetXFAList()->Insert(pNewNode, pBeforeNode);
}

void CJX_List::remove(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"remove");
    return;
  }

  auto* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode) {
    ThrowArgumentMismatchException();
    return;
  }
  GetXFAList()->Remove(pNode);
}

void CJX_List::item(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"item");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0 || iIndex >= GetXFAList()->GetLength()) {
    ThrowIndexOutOfBoundsException();
    return;
  }
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(
          GetXFAList()->Item(iIndex)));
}

void CJX_List::length(CFXJSE_Value* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetXFAList()->GetLength());
}
