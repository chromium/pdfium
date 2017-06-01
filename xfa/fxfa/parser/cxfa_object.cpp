// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_object.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/app/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_nodelist.h"

CXFA_Object::CXFA_Object(CXFA_Document* pDocument,
                         XFA_ObjectType objectType,
                         XFA_Element elementType,
                         const CFX_WideStringC& elementName)
    : CFXJSE_HostObject(kXFA),
      m_pDocument(pDocument),
      m_objectType(objectType),
      m_elementType(elementType),
      m_elementNameHash(FX_HashCode_GetW(elementName, false)),
      m_elementName(elementName) {}

CXFA_Object::~CXFA_Object() {}

void CXFA_Object::Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                               bool bSetting,
                                               XFA_ATTRIBUTE eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(FX_UTF8Encode(GetClassName()).AsStringC());
}

void CXFA_Object::ThrowInvalidPropertyException() const {
  ThrowException(L"Invalid property set operation.");
}

void CXFA_Object::ThrowIndexOutOfBoundsException() const {
  ThrowException(L"Index value is out of bounds.");
}

void CXFA_Object::ThrowParamCountMismatchException(
    const CFX_WideString& method) const {
  ThrowException(L"Incorrect number of parameters calling method '%.16s'.",
                 method.c_str());
}

void CXFA_Object::ThrowArgumentMismatchException() const {
  ThrowException(L"Argument mismatch in property or function argument.");
}

void CXFA_Object::ThrowException(const wchar_t* str, ...) const {
  CFX_WideString wsMessage;
  va_list arg_ptr;
  va_start(arg_ptr, str);
  wsMessage.FormatV(str, arg_ptr);
  va_end(arg_ptr);
  ASSERT(!wsMessage.IsEmpty());
  FXJSE_ThrowMessage(wsMessage.UTF8Encode().AsStringC());
}

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
