// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_UTIL_H_
#define FPDFSDK_JAVASCRIPT_UTIL_H_

#include <string>
#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"

// Return values for ParseDataType() below.
#define UTIL_INT 0
#define UTIL_DOUBLE 1
#define UTIL_STRING 2

class util : public CJS_EmbedObj {
 public:
  explicit util(CJS_Object* pJSObject);
  ~util() override;

  CJS_Return printd(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return printf(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return printx(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return scand(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return byteToChar(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);

  static WideString printx(const WideString& cFormat,
                           const WideString& cSource);

 private:
  friend class CJS_Util_ParseDataType_Test;

  static int ParseDataType(std::wstring* sFormat);
};

class CJS_Util : public CJS_Object {
 public:
  explicit CJS_Util(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Util() override {}

  static const char* g_pClassName;
  static int g_nObjDefnID;
  static JSMethodSpec MethodSpecs[];

  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);

  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);
  static void DefineMethods(CFXJS_Engine* pEngine);

  JS_STATIC_METHOD(printd, util);
  JS_STATIC_METHOD(printf, util);
  JS_STATIC_METHOD(printx, util);
  JS_STATIC_METHOD(scand, util);
  JS_STATIC_METHOD(byteToChar, util);
};

#endif  // FPDFSDK_JAVASCRIPT_UTIL_H_
