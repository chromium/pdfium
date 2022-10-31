// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/fxjse.h"

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-template.h"

namespace pdfium {
namespace fxjse {

const char kFuncTag[] = "function descriptor tag";
const char kClassTag[] = "class descriptor tag";

}  // namespace fxjse
}  // namespace pdfium

// static
CFXJSE_HostObject* CFXJSE_HostObject::FromV8(v8::Local<v8::Value> arg) {
  if (!fxv8::IsObject(arg))
    return nullptr;

  return FXJSE_RetrieveObjectBinding(arg.As<v8::Object>());
}

CFXJSE_HostObject::CFXJSE_HostObject() = default;

CFXJSE_HostObject::~CFXJSE_HostObject() = default;

CFXJSE_FormCalcContext* CFXJSE_HostObject::AsFormCalcContext() {
  return nullptr;
}

CJX_Object* CFXJSE_HostObject::AsCJXObject() {
  return nullptr;
}

v8::Local<v8::Object> CFXJSE_HostObject::NewBoundV8Object(
    v8::Isolate* pIsolate,
    v8::Local<v8::FunctionTemplate> tmpl) {
  v8::Local<v8::Object> hObject =
      tmpl->InstanceTemplate()
          ->NewInstance(pIsolate->GetCurrentContext())
          .ToLocalChecked();
  FXJSE_UpdateObjectBinding(hObject, this);
  return hObject;
}
