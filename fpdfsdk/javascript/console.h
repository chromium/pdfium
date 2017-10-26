// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_CONSOLE_H_
#define FPDFSDK_JAVASCRIPT_CONSOLE_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"

class console : public CJS_EmbedObj {
 public:
  explicit console(CJS_Object* pJSObject);
  ~console() override;

 public:
  CJS_Return clear(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return hide(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return println(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return show(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params);
};

class CJS_Console : public CJS_Object {
 public:
  explicit CJS_Console(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Console() override {}

  static const char* g_pClassName;
  static int g_nObjDefnID;
  static JSMethodSpec MethodSpecs[];

  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);

  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);
  static void DefineMethods(CFXJS_Engine* pEngine);

  JS_STATIC_METHOD(clear, console);
  JS_STATIC_METHOD(hide, console);
  JS_STATIC_METHOD(println, console);
  JS_STATIC_METHOD(show, console);
};

#endif  // FPDFSDK_JAVASCRIPT_CONSOLE_H_
