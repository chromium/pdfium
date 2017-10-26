// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_CJS_GLOBAL_H_
#define FPDFSDK_JAVASCRIPT_CJS_GLOBAL_H_

#include "fpdfsdk/javascript/JS_Define.h"

class CJS_Global : public CJS_Object {
 public:
  explicit CJS_Global(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Global() override {}

  // CJS_Object
  void InitInstance(IJS_Runtime* pIRuntime) override;

  static const char* g_pClassName;
  static int g_nObjDefnID;
  static JSConstSpec ConstSpecs[];
  static JSPropertySpec PropertySpecs[];
  static JSMethodSpec MethodSpecs[];

  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);
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
};

#endif  // FPDFSDK_JAVASCRIPT_CJS_GLOBAL_H_
