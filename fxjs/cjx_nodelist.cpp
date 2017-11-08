// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_nodelist.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodelist.h"

CJX_NodeList::CJX_NodeList(CXFA_NodeList* list) : CJX_Object(list) {}

CJX_NodeList::~CJX_NodeList() {}

CXFA_NodeList* CJX_NodeList::GetXFANodeList() {
  return static_cast<CXFA_NodeList*>(GetXFAObject());
}

void CJX_NodeList::Script_ListClass_Append(CFXJSE_Arguments* pArguments) {
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
  GetXFANodeList()->Append(pNode);
}

void CJX_NodeList::Script_ListClass_Insert(CFXJSE_Arguments* pArguments) {
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
  GetXFANodeList()->Insert(pNewNode, pBeforeNode);
}

void CJX_NodeList::Script_ListClass_Remove(CFXJSE_Arguments* pArguments) {
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
  GetXFANodeList()->Remove(pNode);
}

void CJX_NodeList::Script_ListClass_Item(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"item");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0 || iIndex >= GetXFANodeList()->GetLength()) {
    ThrowIndexOutOfBoundsException();
    return;
  }
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(
          GetXFANodeList()->Item(iIndex)));
}

void CJX_NodeList::Script_TreelistClass_NamedItem(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"namedItem");
    return;
  }

  ByteString szName = pArguments->GetUTF8String(0);
  CXFA_Node* pNode = GetXFANodeList()->NamedItem(
      WideString::FromUTF8(szName.AsStringView()).AsStringView());
  if (!pNode)
    return;

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNode));
}

void CJX_NodeList::Script_ListClass_Length(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetXFANodeList()->GetLength());
}
