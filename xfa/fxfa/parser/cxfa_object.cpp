// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_object.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodelist.h"

CXFA_Object::CXFA_Object(CXFA_Document* pDocument,
                         XFA_ObjectType objectType,
                         XFA_Element elementType,
                         const WideStringView& elementName,
                         std::unique_ptr<CJX_Object> jsObject)
    : CFXJSE_HostObject(kXFA),
      m_pDocument(pDocument),
      m_objectType(objectType),
      m_elementType(elementType),
      m_elementNameHash(FX_HashCode_GetW(elementName, false)),
      m_elementName(elementName),
      m_pJSObject(std::move(jsObject)) {}

CXFA_Object::~CXFA_Object() {}

CXFA_Node* CXFA_Object::AsNode() {
  return IsNode() ? static_cast<CXFA_Node*>(this) : nullptr;
}

CXFA_NodeList* CXFA_Object::AsNodeList() {
  return IsNodeList() ? static_cast<CXFA_NodeList*>(this) : nullptr;
}

const CXFA_Node* CXFA_Object::AsNode() const {
  return IsNode() ? static_cast<const CXFA_Node*>(this) : nullptr;
}

const CXFA_NodeList* CXFA_Object::AsNodeList() const {
  return IsNodeList() ? static_cast<const CXFA_NodeList*>(this) : nullptr;
}

CXFA_Node* ToNode(CXFA_Object* pObj) {
  return pObj ? pObj->AsNode() : nullptr;
}

const CXFA_Node* ToNode(const CXFA_Object* pObj) {
  return pObj ? pObj->AsNode() : nullptr;
}
