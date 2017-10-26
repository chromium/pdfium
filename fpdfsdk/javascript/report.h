// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_REPORT_H_
#define FPDFSDK_JAVASCRIPT_REPORT_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"

class Report : public CJS_EmbedObj {
 public:
  explicit Report(CJS_Object* pJSObject);
  ~Report() override;

 public:
  CJS_Return save(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return writeText(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
};

class CJS_Report : public CJS_Object {
 public:
  explicit CJS_Report(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Report() override {}

  static const char* g_pClassName;
  static int g_nObjDefnID;
  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);
  static JSConstSpec ConstSpecs[];
  static void DefineConsts(CFXJS_Engine* pEngine);
  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);
  static void DefineProps(CFXJS_Engine* pEngine);
  static void DefineMethods(CFXJS_Engine* pEngine);
  static JSPropertySpec PropertySpecs[];
  static JSMethodSpec MethodSpecs[];

  JS_STATIC_METHOD(save, Report)
  JS_STATIC_METHOD(writeText, Report);
};

#endif  // FPDFSDK_JAVASCRIPT_REPORT_H_
