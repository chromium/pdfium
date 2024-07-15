// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_logpseudomodel.h"

#include "core/fxcrt/span.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_logpseudomodel.h"

const CJX_MethodSpec CJX_LogPseudoModel::MethodSpecs[] = {
    {"message", message_static},
    {"traceEnabled", traceEnabled_static},
    {"traceActivate", traceActivate_static},
    {"traceDeactivate", traceDeactivate_static},
    {"trace", trace_static}};

CJX_LogPseudoModel::CJX_LogPseudoModel(CScript_LogPseudoModel* model)
    : CJX_Object(model) {
  DefineMethods(MethodSpecs);
}

CJX_LogPseudoModel::~CJX_LogPseudoModel() = default;

bool CJX_LogPseudoModel::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_LogPseudoModel::message(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  // Uncomment to allow using xfa.log.message(""); from JS.
  // fprintf(stderr, "LOG\n");
  // for (auto& val : params) {
  //   v8::String::Utf8Value str(runtime->GetIsolate(), val);
  //   fprintf(stderr, "  %ls\n", WideString::FromUTF8(*str).c_str());
  // }

  return CJS_Result::Success();
}

CJS_Result CJX_LogPseudoModel::traceEnabled(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJX_LogPseudoModel::traceActivate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJX_LogPseudoModel::traceDeactivate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJX_LogPseudoModel::trace(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}
