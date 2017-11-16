// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_object.h"

#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_object.h"

CJX_Object::CJX_Object(CXFA_Object* obj) : object_(obj) {}

CJX_Object::~CJX_Object() {}

CXFA_Document* CJX_Object::GetDocument() const {
  return object_->GetDocument();
}

void CJX_Object::Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(
      FX_UTF8Encode(GetXFAObject()->GetClassName()).AsStringView());
}

void CJX_Object::ThrowInvalidPropertyException() const {
  ThrowException(L"Invalid property set operation.");
}

void CJX_Object::ThrowIndexOutOfBoundsException() const {
  ThrowException(L"Index value is out of bounds.");
}

void CJX_Object::ThrowParamCountMismatchException(
    const WideString& method) const {
  ThrowException(L"Incorrect number of parameters calling method '%.16s'.",
                 method.c_str());
}

void CJX_Object::ThrowArgumentMismatchException() const {
  ThrowException(L"Argument mismatch in property or function argument.");
}

void CJX_Object::ThrowException(const wchar_t* str, ...) const {
  va_list arg_ptr;
  va_start(arg_ptr, str);
  WideString wsMessage = WideString::FormatV(str, arg_ptr);
  va_end(arg_ptr);

  ASSERT(!wsMessage.IsEmpty());
  FXJSE_ThrowMessage(wsMessage.UTF8Encode().AsStringView());
}
