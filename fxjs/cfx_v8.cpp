// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_v8.h"

#include "fxjs/fxv8.h"
#include "v8/include/v8-isolate.h"

CFX_V8::CFX_V8(v8::Isolate* isolate) : m_pIsolate(isolate) {}

CFX_V8::~CFX_V8() = default;

v8::Local<v8::Value> CFX_V8::GetObjectProperty(
    v8::Local<v8::Object> pObj,
    ByteStringView bsUTF8PropertyName) {
  return fxv8::ReentrantGetObjectPropertyHelper(GetIsolate(), pObj,
                                                bsUTF8PropertyName);
}

std::vector<WideString> CFX_V8::GetObjectPropertyNames(
    v8::Local<v8::Object> pObj) {
  return fxv8::ReentrantGetObjectPropertyNamesHelper(GetIsolate(), pObj);
}

void CFX_V8::PutObjectProperty(v8::Local<v8::Object> pObj,
                               ByteStringView bsUTF8PropertyName,
                               v8::Local<v8::Value> pPut) {
  fxv8::ReentrantPutObjectPropertyHelper(GetIsolate(), pObj, bsUTF8PropertyName,
                                         pPut);
}

void CFX_V8::DisposeIsolate() {
  if (m_pIsolate)
    m_pIsolate.ExtractAsDangling()->Dispose();
}

v8::Local<v8::Array> CFX_V8::NewArray() {
  return fxv8::NewArrayHelper(GetIsolate());
}

v8::Local<v8::Object> CFX_V8::NewObject() {
  return fxv8::NewObjectHelper(GetIsolate());
}

void CFX_V8::PutArrayElement(v8::Local<v8::Array> pArray,
                             size_t index,
                             v8::Local<v8::Value> pValue) {
  fxv8::ReentrantPutArrayElementHelper(GetIsolate(), pArray, index, pValue);
}

v8::Local<v8::Value> CFX_V8::GetArrayElement(v8::Local<v8::Array> pArray,
                                             size_t index) {
  return fxv8::ReentrantGetArrayElementHelper(GetIsolate(), pArray, index);
}

size_t CFX_V8::GetArrayLength(v8::Local<v8::Array> pArray) {
  return fxv8::GetArrayLengthHelper(pArray);
}

v8::Local<v8::Number> CFX_V8::NewNumber(int number) {
  return fxv8::NewNumberHelper(GetIsolate(), number);
}

v8::Local<v8::Number> CFX_V8::NewNumber(double number) {
  return fxv8::NewNumberHelper(GetIsolate(), number);
}

v8::Local<v8::Number> CFX_V8::NewNumber(float number) {
  return fxv8::NewNumberHelper(GetIsolate(), number);
}

v8::Local<v8::Boolean> CFX_V8::NewBoolean(bool b) {
  return fxv8::NewBooleanHelper(GetIsolate(), b);
}

v8::Local<v8::String> CFX_V8::NewString(ByteStringView str) {
  return fxv8::NewStringHelper(GetIsolate(), str);
}

v8::Local<v8::String> CFX_V8::NewString(WideStringView str) {
  // Conversion from pdfium's wchar_t wide-strings to v8's uint16_t
  // wide-strings isn't handled by v8, so use UTF8 as a common
  // intermediate format.
  return NewString(FX_UTF8Encode(str).AsStringView());
}

v8::Local<v8::Value> CFX_V8::NewNull() {
  return fxv8::NewNullHelper(GetIsolate());
}

v8::Local<v8::Value> CFX_V8::NewUndefined() {
  return fxv8::NewUndefinedHelper(GetIsolate());
}

v8::Local<v8::Date> CFX_V8::NewDate(double d) {
  return fxv8::NewDateHelper(GetIsolate(), d);
}

int CFX_V8::ToInt32(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToInt32Helper(GetIsolate(), pValue);
}

bool CFX_V8::ToBoolean(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToBooleanHelper(GetIsolate(), pValue);
}

double CFX_V8::ToDouble(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToDoubleHelper(GetIsolate(), pValue);
}

WideString CFX_V8::ToWideString(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToWideStringHelper(GetIsolate(), pValue);
}

ByteString CFX_V8::ToByteString(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToByteStringHelper(GetIsolate(), pValue);
}

v8::Local<v8::Object> CFX_V8::ToObject(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToObjectHelper(GetIsolate(), pValue);
}

v8::Local<v8::Array> CFX_V8::ToArray(v8::Local<v8::Value> pValue) {
  return fxv8::ReentrantToArrayHelper(GetIsolate(), pValue);
}

void CFX_V8IsolateDeleter::operator()(v8::Isolate* ptr) {
  ptr->Dispose();
}
