// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_console.h"

#include "core/fxcrt/span.h"
#include "fxjs/cjs_event_context.h"
#include "fxjs/cjs_object.h"
#include "fxjs/js_define.h"

const JSMethodSpec CJS_Console::MethodSpecs[] = {{"clear", clear_static},
                                                 {"hide", hide_static},
                                                 {"println", println_static},
                                                 {"show", show_static}};

uint32_t CJS_Console::ObjDefnID = 0;
const char CJS_Console::kName[] = "console";

// static
uint32_t CJS_Console::GetObjDefnID() {
  return ObjDefnID;
}

// static
void CJS_Console::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID = pEngine->DefineObj(CJS_Console::kName, FXJSOBJTYPE_STATIC,
                                 JSConstructor<CJS_Console>, JSDestructor);
  DefineMethods(pEngine, ObjDefnID, MethodSpecs);
}

CJS_Console::CJS_Console(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime)
    : CJS_Object(pObject, pRuntime) {}

CJS_Console::~CJS_Console() = default;

CJS_Result CJS_Console::clear(CJS_Runtime* pRuntime,
                              pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Console::hide(CJS_Runtime* pRuntime,
                             pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Console::println(CJS_Runtime* pRuntime,
                                pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Console::show(CJS_Runtime* pRuntime,
                             pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}
