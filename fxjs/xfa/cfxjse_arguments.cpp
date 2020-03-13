// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_arguments.h"

#include "fxjs/cfx_v8.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "third_party/base/ptr_util.h"

CFXJSE_Arguments::CFXJSE_Arguments(
    const v8::FunctionCallbackInfo<v8::Value>* pInfo,
    CFXJSE_Value* pRetValue)
    : m_pInfo(pInfo), m_pRetValue(pRetValue) {}

CFXJSE_Arguments::~CFXJSE_Arguments() {}

int32_t CFXJSE_Arguments::GetLength() const {
  return m_pInfo->Length();
}

std::unique_ptr<CFXJSE_Value> CFXJSE_Arguments::GetValue(int32_t index) const {
  auto pArgValue = pdfium::MakeUnique<CFXJSE_Value>(v8::Isolate::GetCurrent());
  pArgValue->ForceSetValue((*m_pInfo)[index]);
  return pArgValue;
}

bool CFXJSE_Arguments::GetBoolean(int32_t index) const {
  return CFX_V8::ReentrantToBooleanHelper(m_pInfo->GetIsolate(),
                                          (*m_pInfo)[index]);
}

int32_t CFXJSE_Arguments::GetInt32(int32_t index) const {
  return CFX_V8::ReentrantToInt32Helper(m_pInfo->GetIsolate(),
                                        (*m_pInfo)[index]);
}

float CFXJSE_Arguments::GetFloat(int32_t index) const {
  return static_cast<float>(CFX_V8::ReentrantToDoubleHelper(
      m_pInfo->GetIsolate(), (*m_pInfo)[index]));
}

ByteString CFXJSE_Arguments::GetUTF8String(int32_t index) const {
  return CFX_V8::ReentrantToByteStringHelper(m_pInfo->GetIsolate(),
                                             (*m_pInfo)[index]);
}

CFXJSE_Value* CFXJSE_Arguments::GetReturnValue() const {
  return m_pRetValue.Get();
}
