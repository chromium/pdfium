// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_object.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"
#include "xfa/fxfa/parser/cxfa_treelist.h"

CXFA_Object::CXFA_Object(CXFA_Document* pDocument,
                         XFA_ObjectType objectType,
                         XFA_Element elementType,
                         const WideStringView& elementName,
                         std::unique_ptr<CJX_Object> jsObject)
    : m_pDocument(pDocument),
      m_objectType(objectType),
      m_elementType(elementType),
      m_elementNameHash(FX_HashCode_GetW(elementName, false)),
      m_elementName(elementName),
      m_pJSObject(std::move(jsObject)) {}

CXFA_Object::~CXFA_Object() = default;

CXFA_Object* CXFA_Object::AsCXFAObject() {
  return this;
}

WideString CXFA_Object::GetSOMExpression() {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext)
    return WideString();

  return pScriptContext->GetSomExpression(ToNode(this));
}

CXFA_List* CXFA_Object::AsList() {
  return IsList() ? static_cast<CXFA_List*>(this) : nullptr;
}

CXFA_Node* CXFA_Object::AsNode() {
  return IsNode() ? static_cast<CXFA_Node*>(this) : nullptr;
}

CXFA_TreeList* CXFA_Object::AsTreeList() {
  return IsTreeList() ? static_cast<CXFA_TreeList*>(this) : nullptr;
}

CXFA_ThisProxy* CXFA_Object::AsThisProxy() {
  return IsThisProxy() ? static_cast<CXFA_ThisProxy*>(this) : nullptr;
}

CXFA_List* ToList(CXFA_Object* pObj) {
  return pObj ? pObj->AsList() : nullptr;
}

CXFA_Node* ToNode(CXFA_Object* pObj) {
  return pObj ? pObj->AsNode() : nullptr;
}

CXFA_TreeList* ToTreeList(CXFA_Object* pObj) {
  return pObj ? pObj->AsTreeList() : nullptr;
}

CXFA_ThisProxy* ToThisProxy(CXFA_Object* pObj) {
  return pObj ? pObj->AsThisProxy() : nullptr;
}
