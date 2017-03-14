// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_valuearray.h"

#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

CXFA_ValueArray::CXFA_ValueArray(v8::Isolate* pIsolate)
    : m_pIsolate(pIsolate) {}

CXFA_ValueArray::~CXFA_ValueArray() {
  for (int32_t i = 0; i < GetSize(); i++)
    delete GetAt(i);
}

std::vector<CXFA_Object*> CXFA_ValueArray::GetAttributeObject() {
  std::vector<CXFA_Object*> objArray;
  for (int32_t i = 0; i < GetSize(); i++)
    objArray.push_back(CXFA_ScriptContext::ToObject(GetAt(i), nullptr));

  return objArray;
}
