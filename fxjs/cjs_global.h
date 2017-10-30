// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_GLOBAL_H_
#define FXJS_CJS_GLOBAL_H_

#include "fxjs/JS_Define.h"

class CJS_Global : public CJS_Object {
 public:
  static void DefineJSObjects(CFXJS_Engine* pEngine);
  static void DefineAllProperties(CFXJS_Engine* pEngine);

  static void queryprop_static(
      v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Integer>& info);
  static void getprop_static(v8::Local<v8::String> property,
                             const v8::PropertyCallbackInfo<v8::Value>& info);
  static void putprop_static(v8::Local<v8::String> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<v8::Value>& info);
  static void delprop_static(v8::Local<v8::String> property,
                             const v8::PropertyCallbackInfo<v8::Boolean>& info);

  static void setPersistent_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  explicit CJS_Global(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Global() override {}

  // CJS_Object
  void InitInstance(IJS_Runtime* pIRuntime) override;

 private:
  static int ObjDefnID;
  static const JSMethodSpec MethodSpecs[];
};

#endif  // FXJS_CJS_GLOBAL_H_
