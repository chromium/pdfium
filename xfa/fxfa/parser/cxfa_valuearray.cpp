// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_valuearray.h"

#include <algorithm>

#include "xfa/fxfa/parser/cxfa_scriptcontext.h"

CXFA_ValueArray::CXFA_ValueArray(v8::Isolate* pIsolate)
    : m_pIsolate(pIsolate) {}

CXFA_ValueArray::~CXFA_ValueArray() {}

std::vector<CXFA_Object*> CXFA_ValueArray::GetAttributeObject() {
  std::vector<CXFA_Object*> result(m_Values.size());
  std::transform(m_Values.begin(), m_Values.end(), result.begin(),
                 [](const std::unique_ptr<CFXJSE_Value>& value) {
                   return CXFA_ScriptContext::ToObject(value.get(), nullptr);
                 });
  return result;
}
