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

}  // namespace fxv8
