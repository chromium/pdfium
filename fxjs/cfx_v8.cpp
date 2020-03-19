// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_v8.h"

#include "core/fxcrt/fx_memory.h"
#include "fxjs/fxv8.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"

CFX_V8::CFX_V8(v8::Isolate* isolate) : m_pIsolate(isolate) {}

CFX_V8::~CFX_V8() = default;

v8::Local<v8::Value> CFX_V8::GetObjectProperty(
    v8::Local<v8::Object> pObj,
    ByteStringView bsUTF8PropertyName) {
  return fxv8::ReentrantGetObjectPropertyHelper(m_pIsolate.Get(), pObj,
                                                bsUTF8PropertyName);
}

std::vector<WideString> CFX_V8::GetObjectPropertyNames(
    v8::Local<v8::Object> pObj) {
  return fxv8::ReentrantGetObjectPropertyNamesHelper(m_pIsolate.Get(), pObj);
}

bool CFX_V8::PutObjectProperty(v8::Local<v8::Object> pObj,
                               ByteStringView bsUTF8PropertyName,
                               v8::Local<v8::Value> pPut) {
  return fxv8::ReentrantPutObjectPropertyHelper(m_pIsolate.Get(), pObj,
                                                bsUTF8PropertyName, pPut);
}

void CFX_V8::DisposeIsolate() {
  if (m_pIsolate)
    m_pIsolate.Release()->Dispose();
}

v8::Local<v8::Array> CFX_V8::NewArray() {
  return v8::Array::New(GetIsolate());
}

v8::Local<v8::Object> CFX_V8::NewObject() {
  return v8::Object::New(GetIsolate());
}

bool CFX_V8::PutArrayElement(v8::Local<v8::Array> pArray,
                             unsigned index,
                             v8::Local<v8::Value> pValue) {
  ASSERT(!pValue.IsEmpty());
  if (pArray.IsEmpty())
    return false;
  return pArray->Set(m_pIsolate->GetCurrentContext(), index, pValue).IsJust();
}

v8::Local<v8::Value> CFX_V8::GetArrayElement(v8::Local<v8::Array> pArray,
                                             unsigned index) {
  if (pArray.IsEmpty())
    return v8::Local<v8::Value>();
  v8::Local<v8::Value> val;
  if (!pArray->Get(m_pIsolate->GetCurrentContext(), index).ToLocal(&val))
    return v8::Local<v8::Value>();
  return val;
}

unsigned CFX_V8::GetArrayLength(v8::Local<v8::Array> pArray) {
  if (pArray.IsEmpty())
    return 0;
  return pArray->Length();
}

v8::Local<v8::Number> CFX_V8::NewNumber(int number) {
  return v8::Int32::New(GetIsolate(), number);
}

v8::Local<v8::Number> CFX_V8::NewNumber(double number) {
  return v8::Number::New(GetIsolate(), number);
}

v8::Local<v8::Number> CFX_V8::NewNumber(float number) {
  return v8::Number::New(GetIsolate(), number);
}

v8::Local<v8::Boolean> CFX_V8::NewBoolean(bool b) {
  return v8::Boolean::New(GetIsolate(), b);
}

v8::Local<v8::String> CFX_V8::NewString(ByteStringView str) {
  v8::Isolate* pIsolate = m_pIsolate ? GetIsolate() : v8::Isolate::GetCurrent();
  return fxv8::NewStringHelper(pIsolate, str);
}

v8::Local<v8::String> CFX_V8::NewString(WideStringView str) {
  // Conversion from pdfium's wchar_t wide-strings to v8's uint16_t
  // wide-strings isn't handled by v8, so use UTF8 as a common
  // intermediate format.
  return NewString(FX_UTF8Encode(str).AsStringView());
}

v8::Local<v8::Value> CFX_V8::NewNull() {
  return v8::Null(GetIsolate());
}

v8::Local<v8::Value> CFX_V8::NewUndefined() {
  return v8::Undefined(GetIsolate());
}

v8::Local<v8::Date> CFX_V8::NewDate(double d) {
  return v8::Date::New(m_pIsolate->GetCurrentContext(), d)
      .ToLocalChecked()
      .As<v8::Date>();
}

int CFX_V8::ToInt32(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToInt32Helper(m_pIsolate.Get(), pValue);
}

bool CFX_V8::ToBoolean(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToBooleanHelper(m_pIsolate.Get(), pValue);
}

double CFX_V8::ToDouble(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToDoubleHelper(m_pIsolate.Get(), pValue);
}

WideString CFX_V8::ToWideString(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToWideStringHelper(m_pIsolate.Get(), pValue);
}

ByteString CFX_V8::ToByteString(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToByteStringHelper(m_pIsolate.Get(), pValue);
}

v8::Local<v8::Object> CFX_V8::ToObject(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty() || !pValue->IsObject())
    return v8::Local<v8::Object>();
  v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
  return pValue->ToObject(context).ToLocalChecked();
}

v8::Local<v8::Array> CFX_V8::ToArray(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty() || !pValue->IsArray())
    return v8::Local<v8::Array>();
  v8::Local<v8::Context> context = m_pIsolate->GetCurrentContext();
  return v8::Local<v8::Array>::Cast(pValue->ToObject(context).ToLocalChecked());
}

void* CFX_V8ArrayBufferAllocator::Allocate(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return GetArrayBufferPartitionAllocator().root()->AllocFlags(
      pdfium::base::PartitionAllocZeroFill, length, "CFX_V8ArrayBuffer");
}

void* CFX_V8ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return GetArrayBufferPartitionAllocator().root()->Alloc(length,
                                                          "CFX_V8ArrayBuffer");
}

void CFX_V8ArrayBufferAllocator::Free(void* data, size_t length) {
  GetArrayBufferPartitionAllocator().root()->Free(data);
}
