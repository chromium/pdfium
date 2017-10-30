// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_CONSOLE_H_
#define FXJS_CJS_CONSOLE_H_

#include <vector>

#include "fxjs/JS_Define.h"

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
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_Console(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Console() override {}

  JS_STATIC_METHOD(clear, console);
  JS_STATIC_METHOD(hide, console);
  JS_STATIC_METHOD(println, console);
  JS_STATIC_METHOD(show, console);

 private:
  static int ObjDefnID;
  static const JSMethodSpec MethodSpecs[];
};

#endif  // FXJS_CJS_CONSOLE_H_
