// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_isolatetracker.h"

#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_runtimedata.h"

CFXJSE_ScopeUtil_IsolateHandle::CFXJSE_ScopeUtil_IsolateHandle(
    v8::Isolate* pIsolate)
    : isolate_scope_(pIsolate), handle_scope_(pIsolate) {}

CFXJSE_ScopeUtil_IsolateHandle::~CFXJSE_ScopeUtil_IsolateHandle() = default;

CFXJSE_ScopeUtil_Context::CFXJSE_ScopeUtil_Context(CFXJSE_Context* pContext)
    : context_scope_(pContext->GetContext()) {}

CFXJSE_ScopeUtil_Context::~CFXJSE_ScopeUtil_Context() = default;

CFXJSE_ScopeUtil_IsolateHandleContext::CFXJSE_ScopeUtil_IsolateHandleContext(
    CFXJSE_Context* pContext)
    : isolate_handle_(pContext->GetIsolate()), context_(pContext) {}

CFXJSE_ScopeUtil_IsolateHandleContext::
    ~CFXJSE_ScopeUtil_IsolateHandleContext() = default;

CFXJSE_ScopeUtil_RootContext::CFXJSE_ScopeUtil_RootContext(
    v8::Isolate* pIsolate)
    : context_scope_(v8::Local<v8::Context>::New(
          pIsolate,
          CFXJSE_RuntimeData::Get(pIsolate)->GetRootContext())) {}

CFXJSE_ScopeUtil_RootContext::~CFXJSE_ScopeUtil_RootContext() = default;

CFXJSE_ScopeUtil_IsolateHandleRootContext::
    CFXJSE_ScopeUtil_IsolateHandleRootContext(v8::Isolate* pIsolate)
    : isolate_handle_(pIsolate), root_context_(pIsolate) {}

CFXJSE_ScopeUtil_IsolateHandleRootContext::
    ~CFXJSE_ScopeUtil_IsolateHandleRootContext() = default;
