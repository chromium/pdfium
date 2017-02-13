// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_object.h"

#include "core/fxcrt/fx_ext.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/app/xfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_Object::CXFA_Object(CXFA_Document* pDocument,
                         XFA_ObjectType objectType,
                         XFA_Element elementType,
                         const CFX_WideStringC& elementName)
    : m_pDocument(pDocument),
      m_objectType(objectType),
      m_elementType(elementType),
      m_elementNameHash(FX_HashCode_GetW(elementName, false)),
      m_elementName(elementName) {}

CXFA_Object::~CXFA_Object() {}

CFX_WideStringC CXFA_Object::GetClassName() const {
  return m_elementName;
}

uint32_t CXFA_Object::GetClassHashCode() const {
  return m_elementNameHash;
}

XFA_Element CXFA_Object::GetElementType() const {
  return m_elementType;
}

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
  ThrowException(L"Incorrect number of parameters calling method '%s'.",
                 method.c_str());
}

void CXFA_Object::ThrowArgumentMismatchException() const {
  ThrowException(L"Argument mismatch in property or function argument.");
}

void CXFA_Object::ThrowException(const FX_WCHAR* str, ...) const {
  CFX_WideString wsMessage;
  va_list arg_ptr;
  va_start(arg_ptr, str);
  wsMessage.FormatV(str, arg_ptr);
  va_end(arg_ptr);
  FXJSE_ThrowMessage(wsMessage.UTF8Encode().AsStringC());
}
