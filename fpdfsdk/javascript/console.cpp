// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/console.h"

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"

JSConstSpec CJS_Console::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Console::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Console::MethodSpecs[] = {{"clear", clear_static},
                                           {"hide", hide_static},
                                           {"println", println_static},
                                           {"show", show_static},
                                           {0, 0}};

IMPLEMENT_JS_CLASS(CJS_Console, console, console)

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
