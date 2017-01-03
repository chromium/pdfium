// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_object.h"

#include <memory>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

CXFA_NodeList::CXFA_NodeList(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::NodeList,
                  XFA_Element::NodeList,
                  CFX_WideStringC(L"nodeList")) {
  m_pDocument->GetScriptContext()->AddToCacheList(
      std::unique_ptr<CXFA_NodeList>(this));
}

CXFA_NodeList::~CXFA_NodeList() {}

CXFA_Node* CXFA_NodeList::NamedItem(const CFX_WideStringC& wsName) {
  uint32_t dwHashCode = FX_HashCode_GetW(wsName, false);
  int32_t iCount = GetLength();
  for (int32_t i = 0; i < iCount; i++) {
    CXFA_Node* ret = Item(i);
    if (dwHashCode == ret->GetNameHash())
      return ret;
  }
  return nullptr;
}

void CXFA_NodeList::Script_ListClass_Append(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"append");
    return;
  }

  CXFA_Node* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode) {
    ThrowArgumentMismatchException();
    return;
  }
  Append(pNode);
}

void CXFA_NodeList::Script_ListClass_Insert(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 2) {
    ThrowParamCountMismatchException(L"insert");
    return;
  }

  CXFA_Node* pNewNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  CXFA_Node* pBeforeNode = static_cast<CXFA_Node*>(pArguments->GetObject(1));
  if (!pNewNode) {
    ThrowArgumentMismatchException();
    return;
  }
  Insert(pNewNode, pBeforeNode);
}

void CXFA_NodeList::Script_ListClass_Remove(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"remove");
    return;
  }

  CXFA_Node* pNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pNode) {
    ThrowArgumentMismatchException();
    return;
  }
  Remove(pNode);
}

void CXFA_NodeList::Script_ListClass_Item(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"item");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0 || iIndex >= GetLength()) {
    ThrowIndexOutOfBoundsException();
    return;
  }
  pArguments->GetReturnValue()->Assign(
      m_pDocument->GetScriptContext()->GetJSValueFromMap(Item(iIndex)));
}

void CXFA_NodeList::Script_TreelistClass_NamedItem(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1) {
    ThrowParamCountMismatchException(L"namedItem");
    return;
  }

  CFX_ByteString szName = pArguments->GetUTF8String(0);
  CXFA_Node* pNode =
      NamedItem(CFX_WideString::FromUTF8(szName.AsStringC()).AsStringC());
  if (!pNode)
    return;

  pArguments->GetReturnValue()->Assign(
      m_pDocument->GetScriptContext()->GetJSValueFromMap(pNode));
}

void CXFA_NodeList::Script_ListClass_Length(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetLength());
}
