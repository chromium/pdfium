// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_runtimedata.h"

#include <utility>

#include "core/fxcrt/check_op.h"
#include "fxjs/cfxjs_engine.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-external.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "v8/include/v8-template.h"

CFXJSE_RuntimeData::CFXJSE_RuntimeData() = default;

CFXJSE_RuntimeData::~CFXJSE_RuntimeData() = default;

std::unique_ptr<CFXJSE_RuntimeData> CFXJSE_RuntimeData::Create(
    v8::Isolate* pIsolate) {
  std::unique_ptr<CFXJSE_RuntimeData> pRuntimeData(new CFXJSE_RuntimeData());
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::FunctionTemplate> hFuncTemplate =
      v8::FunctionTemplate::New(pIsolate);

  v8::Local<v8::ObjectTemplate> hGlobalTemplate =
      hFuncTemplate->InstanceTemplate();
  hGlobalTemplate->Set(v8::Symbol::GetToStringTag(pIsolate),
                       fxv8::NewStringHelper(pIsolate, "global"));

  v8::Local<v8::Context> hContext =
      v8::Context::New(pIsolate, nullptr, hGlobalTemplate);

  DCHECK_EQ(hContext->Global()->InternalFieldCount(), 0);
  DCHECK_EQ(
      hContext->Global()->GetPrototype().As<v8::Object>()->InternalFieldCount(),
      0);

  hContext->SetSecurityToken(v8::External::New(pIsolate, pIsolate));
  pRuntimeData->root_context_global_template_.Reset(pIsolate, hFuncTemplate);
  pRuntimeData->root_context_.Reset(pIsolate, hContext);
  return pRuntimeData;
}

CFXJSE_RuntimeData* CFXJSE_RuntimeData::Get(v8::Isolate* pIsolate) {
  CFXJS_PerIsolateData::SetUp(pIsolate);
  CFXJS_PerIsolateData* pData = CFXJS_PerIsolateData::Get(pIsolate);
  if (!pData->GetExtension())
    pData->SetExtension(CFXJSE_RuntimeData::Create(pIsolate));
  return static_cast<CFXJSE_RuntimeData*>(pData->GetExtension());
}

v8::Local<v8::Context> CFXJSE_RuntimeData::GetRootContext(
    v8::Isolate* pIsolate) {
  return v8::Local<v8::Context>::New(pIsolate, root_context_);
}
