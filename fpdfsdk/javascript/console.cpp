// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/console.h"

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_eventhandler.h"

JSConstSpec CJS_Console::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Console::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Console::MethodSpecs[] = {{"clear", clear_static},
                                           {"hide", hide_static},
                                           {"println", println_static},
                                           {"show", show_static},
                                           {0, 0}};

const char* CJS_Console::g_pClassName = "console";
int CJS_Console::g_nObjDefnID = -1;

void CJS_Console::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_Console::JSConstructor(CFXJS_Engine* pEngine,
                                v8::Local<v8::Object> obj) {
  CJS_Object* pObj = new CJS_Console(obj);
  pObj->SetEmbedObject(new console(pObj));
  pEngine->SetObjectPrivate(obj, pObj);
  pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));
}

void CJS_Console::JSDestructor(CFXJS_Engine* pEngine,
                               v8::Local<v8::Object> obj) {
  delete static_cast<CJS_Console*>(pEngine->GetObjectPrivate(obj));
}

void CJS_Console::DefineProps(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(PropertySpecs) - 1; ++i) {
    pEngine->DefineObjProperty(g_nObjDefnID, PropertySpecs[i].pName,
                               PropertySpecs[i].pPropGet,
                               PropertySpecs[i].pPropPut);
  }
}

void CJS_Console::DefineMethods(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(MethodSpecs) - 1; ++i) {
    pEngine->DefineObjMethod(g_nObjDefnID, MethodSpecs[i].pName,
                             MethodSpecs[i].pMethodCall);
  }
}

void CJS_Console::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj(CJS_Console::g_pClassName, eObjType,
                                    JSConstructor, JSDestructor);
  DefineConsts(pEngine);
  DefineProps(pEngine);
  DefineMethods(pEngine);
}

console::console(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {}

console::~console() {}

CJS_Return console::clear(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return console::hide(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return console::println(CJS_Runtime* pRuntime,
                            const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(params.size() > 0);
}

CJS_Return console::show(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}
