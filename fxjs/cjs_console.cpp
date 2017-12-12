// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_console.h"

#include <vector>

#include "fxjs/JS_Define.h"
#include "fxjs/cjs_event_context.h"
#include "fxjs/cjs_eventhandler.h"
#include "fxjs/cjs_object.h"

const JSMethodSpec CJS_Console::MethodSpecs[] = {{"clear", clear_static},
                                                 {"hide", hide_static},
                                                 {"println", println_static},
                                                 {"show", show_static}};

int CJS_Console::ObjDefnID = -1;

// static
void CJS_Console::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID = pEngine->DefineObj("console", FXJSOBJTYPE_STATIC,
                                 JSConstructor<CJS_Console, console>,
                                 JSDestructor<CJS_Console>);
  DefineMethods(pEngine, ObjDefnID, MethodSpecs, FX_ArraySize(MethodSpecs));
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
