// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_object.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"
#include "xfa/fxfa/parser/cxfa_treelist.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"

CXFA_Object::CXFA_Object(CXFA_Document* pDocument,
                         XFA_ObjectType objectType,
                         XFA_Element elementType,
                         CJX_Object* jsObject)
    : m_objectType(objectType),
      m_elementType(elementType),
      m_elementName(XFA_ElementToName(elementType)),
      m_elementNameHash(FX_HashCode_GetAsIfW(m_elementName)),
      m_pDocument(pDocument),
      m_pJSObject(jsObject) {}

CXFA_Object::~CXFA_Object() = default;

void CXFA_Object::Trace(cppgc::Visitor* visitor) const {
  visitor->Trace(m_pDocument);
  visitor->Trace(m_pJSObject);
}

WideString CXFA_Object::GetSOMExpression() {
  CXFA_Node* pNode = AsNode();
  return pNode ? pNode->GetNameExpression() : WideString();
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
