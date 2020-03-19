// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_FXV8_H_
#define FXJS_FXV8_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "v8/include/v8.h"

namespace fxv8 {

v8::Local<v8::String> NewStringHelper(v8::Isolate* pIsolate,
                                      ByteStringView str);
v8::Local<v8::String> NewStringHelper(v8::Isolate* pIsolate,
                                      WideStringView str);

int ReentrantToInt32Helper(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
bool ReentrantToBooleanHelper(v8::Isolate* pIsolate,
                              v8::Local<v8::Value> pValue);
double ReentrantToDoubleHelper(v8::Isolate* pIsolate,
                               v8::Local<v8::Value> pValue);
WideString ReentrantToWideStringHelper(v8::Isolate* pIsolate,
                                       v8::Local<v8::Value> pValue);
ByteString ReentrantToByteStringHelper(v8::Isolate* pIsolate,
                                       v8::Local<v8::Value> pValue);

v8::Local<v8::Value> ReentrantGetObjectPropertyHelper(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObj,
    ByteStringView bsUTF8PropertyName);
std::vector<WideString> ReentrantGetObjectPropertyNamesHelper(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObj);
bool ReentrantPutObjectPropertyHelper(v8::Isolate* pIsolate,
                                      v8::Local<v8::Object> pObj,
                                      ByteStringView bsUTF8PropertyName,
                                      v8::Local<v8::Value> pPut);

void ThrowExceptionHelper(v8::Isolate* pIsolate, ByteStringView str);
void ThrowExceptionHelper(v8::Isolate* pIsolate, WideStringView str);

}  // namespace fxv8

#endif  // FXJS_FXV8_H_
