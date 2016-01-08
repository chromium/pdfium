// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fxv8.h"
#include "context.h"
#include "class.h"
#include "value.h"
#include "scope_inline.h"
#include "util_inline.h"
FXJSE_HCONTEXT FXJSE_Context_Create(FXJSE_HRUNTIME hRuntime,
                                    const FXJSE_CLASS* lpGlobalClass,
                                    void* lpGlobalObject) {
  CFXJSE_Context* pContext = CFXJSE_Context::Create(
      reinterpret_cast<v8::Isolate*>(hRuntime), lpGlobalClass, lpGlobalObject);
  return reinterpret_cast<FXJSE_HCONTEXT>(pContext);
}
void FXJSE_Context_Release(FXJSE_HCONTEXT hContext) {
  CFXJSE_Context* pContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  if (pContext) {
    delete pContext;
  }
}
FXJSE_HVALUE FXJSE_Context_GetGlobalObject(FXJSE_HCONTEXT hContext) {
  CFXJSE_Context* pContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  if (!pContext) {
    return NULL;
  }
  CFXJSE_Value* lpValue = CFXJSE_Value::Create(pContext->GetRuntime());
  ASSERT(lpValue);
  pContext->GetGlobalObject(lpValue);
  return reinterpret_cast<FXJSE_HVALUE>(lpValue);
}
FXJSE_HRUNTIME FXJSE_Context_GetRuntime(FXJSE_HCONTEXT hContext) {
  CFXJSE_Context* pContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  return pContext ? reinterpret_cast<FXJSE_HRUNTIME>(pContext->GetRuntime())
                  : NULL;
}
static const FX_CHAR* szCompatibleModeScripts[] = {
    "(function (global, list) { 'use strict'; var objname; for (objname in list) { var globalobj = global[objname];\n\
			if (globalobj) { list[objname].forEach( function (name) { if (!globalobj[name]) { Object.defineProperty(globalobj, name, {writable: true, enumerable: false, value: \n\
			(function (obj) {\n\
	if (arguments.length === 0) {\n\
		throw new TypeError('missing argument 0 when calling function ' + objname + '.' + name);\n\
	}\n\
	return globalobj.prototype[name].apply(obj, Array.prototype.slice.call(arguments, 1));\n\
})});}});}}}(this, {String: ['substr', 'toUpperCase']}));",
};
void FXJSE_Context_EnableCompatibleMode(FXJSE_HCONTEXT hContext,
                                        FX_DWORD dwCompatibleFlags) {
  for (uint32_t i = 0; i < (uint32_t)FXJSE_COMPATIBLEMODEFLAGCOUNT; i++) {
    if (dwCompatibleFlags & (1 << i)) {
      FXJSE_ExecuteScript(hContext, szCompatibleModeScripts[i], NULL, NULL);
    }
  }
}
FX_BOOL FXJSE_ExecuteScript(FXJSE_HCONTEXT hContext,
                            const FX_CHAR* szScript,
                            FXJSE_HVALUE hRetValue,
                            FXJSE_HVALUE hNewThisObject) {
  CFXJSE_Context* pContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  ASSERT(pContext);
  return pContext->ExecuteScript(
      szScript, reinterpret_cast<CFXJSE_Value*>(hRetValue),
      reinterpret_cast<CFXJSE_Value*>(hNewThisObject));
}
v8::Local<v8::Object> FXJSE_CreateReturnValue(v8::Isolate* pIsolate,
                                              v8::TryCatch& trycatch) {
  v8::Local<v8::Object> hReturnValue = v8::Object::New(pIsolate);
  if (trycatch.HasCaught()) {
    v8::Local<v8::Value> hException = trycatch.Exception();
    v8::Local<v8::Message> hMessage = trycatch.Message();
    if (hException->IsObject()) {
      v8::Local<v8::Value> hValue;
      hValue = hException.As<v8::Object>()->Get(
          v8::String::NewFromUtf8(pIsolate, "name"));
      if (hValue->IsString() || hValue->IsStringObject()) {
        hReturnValue->Set(0, hValue);
      } else {
        hReturnValue->Set(0, v8::String::NewFromUtf8(pIsolate, "Error"));
      }
      hValue = hException.As<v8::Object>()->Get(
          v8::String::NewFromUtf8(pIsolate, "message"));
      if (hValue->IsString() || hValue->IsStringObject()) {
        hReturnValue->Set(1, hValue);
      } else {
        hReturnValue->Set(1, hMessage->Get());
      }
    } else {
      hReturnValue->Set(0, v8::String::NewFromUtf8(pIsolate, "Error"));
      hReturnValue->Set(1, hMessage->Get());
    }
    hReturnValue->Set(2, hException);
    hReturnValue->Set(3, v8::Integer::New(pIsolate, hMessage->GetLineNumber()));
    hReturnValue->Set(4, hMessage->GetSourceLine());
    hReturnValue->Set(5,
                      v8::Integer::New(pIsolate, hMessage->GetStartColumn()));
    hReturnValue->Set(6, v8::Integer::New(pIsolate, hMessage->GetEndColumn()));
  }
  return hReturnValue;
}
FX_BOOL FXJSE_ReturnValue_GetMessage(FXJSE_HVALUE hRetValue,
                                     CFX_ByteString& utf8Name,
                                     CFX_ByteString& utf8Message) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hRetValue);
  if (!lpValue) {
    return FALSE;
  }
  v8::Isolate* pIsolate = lpValue->GetIsolate();
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hValue =
      v8::Local<v8::Value>::New(pIsolate, lpValue->DirectGetValue());
  if (!hValue->IsObject()) {
    return FALSE;
  }
  v8::String::Utf8Value hStringVal0(
      hValue.As<v8::Object>()->Get(0)->ToString());
  utf8Name = *hStringVal0;
  v8::String::Utf8Value hStringVal1(
      hValue.As<v8::Object>()->Get(1)->ToString());
  utf8Message = *hStringVal1;
  return TRUE;
}
FX_BOOL FXJSE_ReturnValue_GetLineInfo(FXJSE_HVALUE hRetValue,
                                      int32_t& nLine,
                                      int32_t& nCol) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hRetValue);
  if (!lpValue) {
    return FALSE;
  }
  v8::Isolate* pIsolate = lpValue->GetIsolate();
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hValue =
      v8::Local<v8::Value>::New(pIsolate, lpValue->DirectGetValue());
  if (!hValue->IsObject()) {
    return FALSE;
  }
  nLine = hValue.As<v8::Object>()->Get(3)->ToInt32()->Value();
  nCol = hValue.As<v8::Object>()->Get(5)->ToInt32()->Value();
  return TRUE;
}
CFXJSE_Context* CFXJSE_Context::Create(v8::Isolate* pIsolate,
                                       const FXJSE_CLASS* lpGlobalClass,
                                       void* lpGlobalObject) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  CFXJSE_Context* pContext = new CFXJSE_Context(pIsolate);
  CFXJSE_Class* lpGlobalClassObj = NULL;
  v8::Local<v8::ObjectTemplate> hObjectTemplate;
  if (lpGlobalClass) {
    lpGlobalClassObj = CFXJSE_Class::Create(pContext, lpGlobalClass, TRUE);
    ASSERT(lpGlobalClassObj);
    v8::Local<v8::FunctionTemplate> hFunctionTemplate =
        v8::Local<v8::FunctionTemplate>::New(pIsolate,
                                             lpGlobalClassObj->m_hTemplate);
    hObjectTemplate = hFunctionTemplate->InstanceTemplate();
  } else {
    hObjectTemplate = v8::ObjectTemplate::New();
    hObjectTemplate->SetInternalFieldCount(1);
  }
  v8::Local<v8::Context> hNewContext =
      v8::Context::New(pIsolate, NULL, hObjectTemplate);
  v8::Local<v8::Context> hRootContext = v8::Local<v8::Context>::New(
      pIsolate, CFXJSE_RuntimeData::Get(pIsolate)->m_hRootContext);
  hNewContext->SetSecurityToken(hRootContext->GetSecurityToken());
  v8::Local<v8::Object> hGlobalObject =
      FXJSE_GetGlobalObjectFromContext(hNewContext);
  FXJSE_UpdateObjectBinding(hGlobalObject, lpGlobalObject);
  pContext->m_hContext.Reset(pIsolate, hNewContext);
  return pContext;
}
CFXJSE_Context::~CFXJSE_Context() {
  for (int32_t i = 0, count = m_rgClasses.GetSize(); i < count; i++) {
    CFXJSE_Class* pClass = m_rgClasses[i];
    if (pClass) {
      delete pClass;
    }
  }
  m_rgClasses.RemoveAll();
}
void CFXJSE_Context::GetGlobalObject(CFXJSE_Value* pValue) {
  ASSERT(pValue);
  CFXJSE_ScopeUtil_IsolateHandleContext scope(this);
  v8::Local<v8::Context> hContext =
      v8::Local<v8::Context>::New(m_pIsolate, m_hContext);
  v8::Local<v8::Object> hGlobalObject = hContext->Global();
  pValue->ForceSetValue(hGlobalObject);
}
FX_BOOL CFXJSE_Context::ExecuteScript(const FX_CHAR* szScript,
                                      CFXJSE_Value* lpRetValue,
                                      CFXJSE_Value* lpNewThisObject) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(this);
  v8::TryCatch trycatch;
  v8::Local<v8::String> hScriptString =
      v8::String::NewFromUtf8(m_pIsolate, szScript);
  if (lpNewThisObject == NULL) {
    v8::Local<v8::Script> hScript = v8::Script::Compile(hScriptString);
    if (!trycatch.HasCaught()) {
      v8::Local<v8::Value> hValue = hScript->Run();
      if (!trycatch.HasCaught()) {
        if (lpRetValue) {
          lpRetValue->m_hValue.Reset(m_pIsolate, hValue);
        }
        return TRUE;
      }
    }
    if (lpRetValue) {
      lpRetValue->m_hValue.Reset(m_pIsolate,
                                 FXJSE_CreateReturnValue(m_pIsolate, trycatch));
    }
    return FALSE;
  } else {
    v8::Local<v8::Value> hNewThis =
        v8::Local<v8::Value>::New(m_pIsolate, lpNewThisObject->m_hValue);
    ASSERT(!hNewThis.IsEmpty());
    v8::Local<v8::Script> hWrapper =
        v8::Script::Compile(v8::String::NewFromUtf8(
            m_pIsolate, "(function () { return eval(arguments[0]); })"));
    v8::Local<v8::Value> hWrapperValue = hWrapper->Run();
    ASSERT(hWrapperValue->IsFunction());
    v8::Local<v8::Function> hWrapperFn = hWrapperValue.As<v8::Function>();
    if (!trycatch.HasCaught()) {
      v8::Local<v8::Value> rgArgs[] = {hScriptString};
      v8::Local<v8::Value> hValue =
          hWrapperFn->Call(hNewThis.As<v8::Object>(), 1, rgArgs);
      if (!trycatch.HasCaught()) {
        if (lpRetValue) {
          lpRetValue->m_hValue.Reset(m_pIsolate, hValue);
        }
        return TRUE;
      }
    }
    if (lpRetValue) {
      lpRetValue->m_hValue.Reset(m_pIsolate,
                                 FXJSE_CreateReturnValue(m_pIsolate, trycatch));
    }
    return FALSE;
  }
}
