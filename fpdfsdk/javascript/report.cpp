// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/report.h"

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"

JSConstSpec CJS_Report::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Report::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Report::MethodSpecs[] = {{"save", save_static},
                                          {"writeText", writeText_static},
                                          {0, 0}};

const char* CJS_Report::g_pClassName = "Report";
int CJS_Report::g_nObjDefnID = -1;

void CJS_Report::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_Report::JSConstructor(CFXJS_Engine* pEngine,
                               v8::Local<v8::Object> obj) {
  CJS_Object* pObj = new CJS_Report(obj);
  pObj->SetEmbedObject(new Report(pObj));
  pEngine->SetObjectPrivate(obj, pObj);
  pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));
}

void CJS_Report::JSDestructor(CFXJS_Engine* pEngine,
                              v8::Local<v8::Object> obj) {
  delete static_cast<CJS_Report*>(pEngine->GetObjectPrivate(obj));
}

void CJS_Report::DefineProps(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(PropertySpecs) - 1; ++i) {
    pEngine->DefineObjProperty(g_nObjDefnID, PropertySpecs[i].pName,
                               PropertySpecs[i].pPropGet,
                               PropertySpecs[i].pPropPut);
  }
}

void CJS_Report::DefineMethods(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(MethodSpecs) - 1; ++i) {
    pEngine->DefineObjMethod(g_nObjDefnID, MethodSpecs[i].pName,
                             MethodSpecs[i].pMethodCall);
  }
}

void CJS_Report::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj(CJS_Report::g_pClassName, eObjType,
                                    JSConstructor, JSDestructor);
  DefineConsts(pEngine);
  DefineProps(pEngine);
  DefineMethods(pEngine);
}

Report::Report(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {}

Report::~Report() {}

CJS_Return Report::writeText(CJS_Runtime* pRuntime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  // Unsafe, not supported.
  return CJS_Return(true);
}

CJS_Return Report::save(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params) {
  // Unsafe, not supported.
  return CJS_Return(true);
}
