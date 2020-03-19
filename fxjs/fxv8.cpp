// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/fxv8.h"

namespace fxv8 {

v8::Local<v8::String> NewStringHelper(v8::Isolate* pIsolate,
                                      ByteStringView str) {
  return v8::String::NewFromUtf8(pIsolate, str.unterminated_c_str(),
                                 v8::NewStringType::kNormal, str.GetLength())
      .ToLocalChecked();
}

v8::Local<v8::String> NewStringHelper(v8::Isolate* pIsolate,
                                      WideStringView str) {
  return NewStringHelper(pIsolate, FX_UTF8Encode(str).AsStringView());
}

int ReentrantToInt32Helper(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0;
  return pValue->Int32Value(pIsolate->GetCurrentContext()).FromMaybe(0);
}

bool ReentrantToBooleanHelper(v8::Isolate* pIsolate,
                              v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return false;
  return pValue->BooleanValue(pIsolate);
}

double ReentrantToDoubleHelper(v8::Isolate* pIsolate,
                               v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0.0;
  return pValue->NumberValue(pIsolate->GetCurrentContext()).FromMaybe(0.0);
}

WideString ReentrantToWideStringHelper(v8::Isolate* pIsolate,
                                       v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return WideString();

  v8::MaybeLocal<v8::String> maybe_string =
      pValue->ToString(pIsolate->GetCurrentContext());
  if (maybe_string.IsEmpty())
    return WideString();

  v8::String::Utf8Value s(pIsolate, maybe_string.ToLocalChecked());
  return WideString::FromUTF8(ByteStringView(*s, s.length()));
}

ByteString ReentrantToByteStringHelper(v8::Isolate* pIsolate,
                                       v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return ByteString();

  v8::MaybeLocal<v8::String> maybe_string =
      pValue->ToString(pIsolate->GetCurrentContext());
  if (maybe_string.IsEmpty())
    return ByteString();

  v8::String::Utf8Value s(pIsolate, maybe_string.ToLocalChecked());
  return ByteString(*s);
}

v8::Local<v8::Value> ReentrantGetObjectPropertyHelper(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObj,
    ByteStringView bsUTF8PropertyName) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Value>();

  v8::Local<v8::Value> val;
  if (!pObj->Get(pIsolate->GetCurrentContext(),
                 NewStringHelper(pIsolate, bsUTF8PropertyName))
           .ToLocal(&val)) {
    return v8::Local<v8::Value>();
  }
  return val;
}

std::vector<WideString> ReentrantGetObjectPropertyNamesHelper(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return std::vector<WideString>();

  v8::Local<v8::Array> val;
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  if (!pObj->GetPropertyNames(context).ToLocal(&val))
    return std::vector<WideString>();

  std::vector<WideString> result;
  for (uint32_t i = 0; i < val->Length(); ++i) {
    result.push_back(ReentrantToWideStringHelper(
        pIsolate, val->Get(context, i).ToLocalChecked()));
  }
  return result;
}

bool ReentrantPutObjectPropertyHelper(v8::Isolate* pIsolate,
                                      v8::Local<v8::Object> pObj,
                                      ByteStringView bsUTF8PropertyName,
                                      v8::Local<v8::Value> pPut) {
  ASSERT(!pPut.IsEmpty());
  if (pObj.IsEmpty())
    return false;

  v8::Local<v8::String> name = NewStringHelper(pIsolate, bsUTF8PropertyName);
  return pObj->Set(pIsolate->GetCurrentContext(), name, pPut).IsJust();
}

void ThrowExceptionHelper(v8::Isolate* pIsolate, ByteStringView str) {
  pIsolate->ThrowException(NewStringHelper(pIsolate, str));
}

void ThrowExceptionHelper(v8::Isolate* pIsolate, WideStringView str) {
  pIsolate->ThrowException(NewStringHelper(pIsolate, str));
}

}  // namespace fxv8
