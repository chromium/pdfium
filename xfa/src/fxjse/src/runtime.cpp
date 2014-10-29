// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../foxitlib.h"
#include "fxv8.h"
#include "runtime.h"
#include "scope_inline.h"
static void FXJSE_KillV8()
{
    v8::V8::Dispose();
}
void FXJSE_Initialize()
{
    static FX_BOOL bV8Initialized = FALSE;
    if (bV8Initialized) {
        return;
    }
    bV8Initialized = TRUE;
    atexit(FXJSE_KillV8);
    FX_LPCSTR szCmdFlags =
        "--harmony_proxies "
        "--block_concurrent_recompilation "
        ;
    v8::V8::SetFlagsFromString(szCmdFlags, FXSYS_strlen(szCmdFlags));
    v8::V8::InitializeICU();
	v8::Platform* platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();
    if(!CFXJSE_RuntimeData::g_RuntimeList) {
        CFXJSE_RuntimeData::g_RuntimeList = FX_NEW CFXJSE_RuntimeList;
    }
}
static void FXJSE_Runtime_DisposeCallback(v8::Isolate* pIsolate)
{
    {
        v8::Locker locker(pIsolate);
        CFXJSE_RuntimeData *pRuntimeData = reinterpret_cast<CFXJSE_RuntimeData*>(pIsolate->GetData(0));
        if(pRuntimeData) {
            pIsolate->SetData(0, NULL);
            delete pRuntimeData;
        }
    }
    pIsolate->Dispose();
}
void FXJSE_Finalize()
{
    if(CFXJSE_RuntimeData::g_RuntimeList) {
        CFXJSE_RuntimeData::g_RuntimeList->RemoveAllRuntimes(FXJSE_Runtime_DisposeCallback);
        delete CFXJSE_RuntimeData::g_RuntimeList;
        CFXJSE_RuntimeData::g_RuntimeList = NULL;
    }
}
FXJSE_HRUNTIME	FXJSE_Runtime_Create()
{
    v8::Isolate* pIsolate = v8::Isolate::New();
    ASSERT(pIsolate && CFXJSE_RuntimeData::g_RuntimeList);
    CFXJSE_RuntimeData::g_RuntimeList->AppendRuntime(pIsolate);
    return reinterpret_cast<FXJSE_HRUNTIME>(pIsolate);
}
void FXJSE_Runtime_Release(FXJSE_HRUNTIME hRuntime)
{
    v8::Isolate* pIsolate = reinterpret_cast<v8::Isolate*>(hRuntime);
    if(pIsolate) {
        ASSERT(CFXJSE_RuntimeData::g_RuntimeList);
        CFXJSE_RuntimeData::g_RuntimeList->RemoveRuntime(pIsolate, FXJSE_Runtime_DisposeCallback);
    }
}
CFXJSE_RuntimeData* CFXJSE_RuntimeData::Create(v8::Isolate* pIsolate)
{
    CFXJSE_RuntimeData* pRuntimeData = FX_NEW CFXJSE_RuntimeData(pIsolate);
    ASSERT(pRuntimeData);
    CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
    v8::Local<v8::FunctionTemplate> hFuncTemplate = v8::FunctionTemplate::New(pIsolate);
    v8::Local<v8::Context> hContext = v8::Context::New(pIsolate, 0, hFuncTemplate->InstanceTemplate());
    hContext->SetSecurityToken(v8::External::New(pIsolate, pIsolate));
    pRuntimeData->m_hRootContextGlobalTemplate.Reset(pIsolate, hFuncTemplate);
    pRuntimeData->m_hRootContext.Reset(pIsolate, hContext);
    return pRuntimeData;
}
CFXJSE_RuntimeData* CFXJSE_RuntimeData::Get(v8::Isolate* pIsolate)
{
    ASSERT(pIsolate);
    CFXJSE_RuntimeData* pRuntimeData = static_cast<CFXJSE_RuntimeData*>(pIsolate->GetData(0));
    if(!pRuntimeData) {
        pRuntimeData = CFXJSE_RuntimeData::Create(pIsolate);
        ASSERT(pRuntimeData);
        pIsolate->SetData(0, pRuntimeData);
    }
    return pRuntimeData;
}
CFXJSE_RuntimeList * CFXJSE_RuntimeData::g_RuntimeList = NULL;
void CFXJSE_RuntimeList::AppendRuntime(v8::Isolate* pIsolate)
{
    m_RuntimeList.Add(pIsolate);
}
void CFXJSE_RuntimeList::RemoveRuntime(v8::Isolate* pIsolate, CFXJSE_RuntimeList::RuntimeDisposeCallback lpfnDisposeCallback)
{
    FX_INT32 iIdx = m_RuntimeList.Find(pIsolate, 0);
    if(iIdx >= 0) {
        m_RuntimeList.RemoveAt(iIdx, 1);
    }
    if(lpfnDisposeCallback) {
        lpfnDisposeCallback(pIsolate);
    }
}
void CFXJSE_RuntimeList::RemoveAllRuntimes(CFXJSE_RuntimeList::RuntimeDisposeCallback lpfnDisposeCallback)
{
    FX_INT32 iSize = m_RuntimeList.GetSize();
    if(lpfnDisposeCallback) {
        for(FX_INT32 iIdx = 0; iIdx < iSize; iIdx++) {
            lpfnDisposeCallback(m_RuntimeList[iIdx]);
        }
    }
    m_RuntimeList.RemoveAll();
}
