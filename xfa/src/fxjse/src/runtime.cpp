// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/jsapi/fxjs_v8.h"  // For per-isolate data.
#include "xfa/src/foxitlib.h"
#include "fxv8.h"
#include "runtime.h"
#include "scope_inline.h"

// Duplicates fpdfsdk's JS_Runtime.h, but keeps XFA from depending on it.
// TODO(tsepez): make a single version of this.
class FXJSE_ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
  void* Allocate(size_t length) override { return calloc(1, length); }
  void* AllocateUninitialized(size_t length) override { return malloc(length); }
  void Free(void* data, size_t length) override { free(data); }
};

static void FXJSE_KillV8() {
  v8::V8::Dispose();
}
void FXJSE_Initialize() {
  if (!CFXJSE_RuntimeData::g_RuntimeList) {
    CFXJSE_RuntimeData::g_RuntimeList = new CFXJSE_RuntimeList;
  }
  static FX_BOOL bV8Initialized = FALSE;
  if (bV8Initialized) {
    return;
  }
  bV8Initialized = TRUE;
  atexit(FXJSE_KillV8);
}
static void FXJSE_Runtime_DisposeCallback(v8::Isolate* pIsolate) {
  {
    v8::Locker locker(pIsolate);
    if (FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate)) {
      delete pData->m_pFXJSERuntimeData;
      pData->m_pFXJSERuntimeData = nullptr;
    }
  }
  pIsolate->Dispose();
}
void FXJSE_Finalize() {
  if (CFXJSE_RuntimeData::g_RuntimeList) {
    CFXJSE_RuntimeData::g_RuntimeList->RemoveAllRuntimes(
        FXJSE_Runtime_DisposeCallback);
    delete CFXJSE_RuntimeData::g_RuntimeList;
    CFXJSE_RuntimeData::g_RuntimeList = NULL;
  }
}
FXJSE_HRUNTIME FXJSE_Runtime_Create() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = new FXJSE_ArrayBufferAllocator();
  v8::Isolate* pIsolate = v8::Isolate::New(params);
  ASSERT(pIsolate && CFXJSE_RuntimeData::g_RuntimeList);
  CFXJSE_RuntimeData::g_RuntimeList->AppendRuntime(pIsolate);
  return reinterpret_cast<FXJSE_HRUNTIME>(pIsolate);
}
void FXJSE_Runtime_Release(FXJSE_HRUNTIME hRuntime) {
  v8::Isolate* pIsolate = reinterpret_cast<v8::Isolate*>(hRuntime);
  if (pIsolate) {
    ASSERT(CFXJSE_RuntimeData::g_RuntimeList);
    CFXJSE_RuntimeData::g_RuntimeList->RemoveRuntime(
        pIsolate, FXJSE_Runtime_DisposeCallback);
  }
}
CFXJSE_RuntimeData* CFXJSE_RuntimeData::Create(v8::Isolate* pIsolate) {
  CFXJSE_RuntimeData* pRuntimeData = new CFXJSE_RuntimeData(pIsolate);
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::FunctionTemplate> hFuncTemplate =
      v8::FunctionTemplate::New(pIsolate);
  v8::Local<v8::Context> hContext =
      v8::Context::New(pIsolate, 0, hFuncTemplate->InstanceTemplate());
  hContext->SetSecurityToken(v8::External::New(pIsolate, pIsolate));
  pRuntimeData->m_hRootContextGlobalTemplate.Reset(pIsolate, hFuncTemplate);
  pRuntimeData->m_hRootContext.Reset(pIsolate, hContext);
  return pRuntimeData;
}
CFXJSE_RuntimeData* CFXJSE_RuntimeData::Get(v8::Isolate* pIsolate) {
  FXJS_PerIsolateData::SetUp(pIsolate);
  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate);
  if (!pData->m_pFXJSERuntimeData)
    pData->m_pFXJSERuntimeData = CFXJSE_RuntimeData::Create(pIsolate);
  return pData->m_pFXJSERuntimeData;
}
CFXJSE_RuntimeList* CFXJSE_RuntimeData::g_RuntimeList = NULL;
void CFXJSE_RuntimeList::AppendRuntime(v8::Isolate* pIsolate) {
  m_RuntimeList.Add(pIsolate);
}
void CFXJSE_RuntimeList::RemoveRuntime(
    v8::Isolate* pIsolate,
    CFXJSE_RuntimeList::RuntimeDisposeCallback lpfnDisposeCallback) {
  int32_t iIdx = m_RuntimeList.Find(pIsolate, 0);
  if (iIdx >= 0) {
    m_RuntimeList.RemoveAt(iIdx, 1);
  }
  if (lpfnDisposeCallback) {
    lpfnDisposeCallback(pIsolate);
  }
}
void CFXJSE_RuntimeList::RemoveAllRuntimes(
    CFXJSE_RuntimeList::RuntimeDisposeCallback lpfnDisposeCallback) {
  int32_t iSize = m_RuntimeList.GetSize();
  if (lpfnDisposeCallback) {
    for (int32_t iIdx = 0; iIdx < iSize; iIdx++) {
      lpfnDisposeCallback(m_RuntimeList[iIdx]);
    }
  }
  m_RuntimeList.RemoveAll();
}
