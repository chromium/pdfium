// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_runtimestub.h"

#include "fxjs/cjs_event_context_stub.h"

CJS_RuntimeStub::CJS_RuntimeStub(CPDFSDK_FormFillEnvironment* pFormFillEnv)
    : form_fill_env_(pFormFillEnv) {}

CJS_RuntimeStub::~CJS_RuntimeStub() = default;

IJS_EventContext* CJS_RuntimeStub::NewEventContext() {
  if (!context_) {
    context_ = std::make_unique<CJS_EventContextStub>();
  }
  return context_.get();
}

void CJS_RuntimeStub::ReleaseEventContext(IJS_EventContext* pContext) {}

CPDFSDK_FormFillEnvironment* CJS_RuntimeStub::GetFormFillEnv() const {
  return form_fill_env_;
}

CJS_Runtime* CJS_RuntimeStub::AsCJSRuntime() {
  return nullptr;
}

std::optional<IJS_Runtime::JS_Error> CJS_RuntimeStub::ExecuteScript(
    const WideString& script) {
  return std::nullopt;
}
