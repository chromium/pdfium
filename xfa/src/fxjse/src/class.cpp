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
static void FXJSE_V8ConstructorCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info);
static void FXJSE_V8FunctionCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info);
static void FXJSE_V8GetterCallback_Wrapper(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info);
static void FXJSE_V8SetterCallback_Wrapper(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info);
void FXJSE_DefineFunctions(FXJSE_HCONTEXT hContext,
                           const FXJSE_FUNCTION* lpFunctions,
                           int nNum) {
  CFXJSE_Context* lpContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  ASSERT(lpContext);
  CFXJSE_ScopeUtil_IsolateHandleContext scope(lpContext);
  v8::Isolate* pIsolate = lpContext->GetRuntime();
  v8::Local<v8::Object> hGlobalObject =
      FXJSE_GetGlobalObjectFromContext(scope.GetLocalContext());
  for (int32_t i = 0; i < nNum; i++) {
    hGlobalObject->ForceSet(
        v8::String::NewFromUtf8(pIsolate, lpFunctions[i].name),
        v8::Function::New(
            pIsolate, FXJSE_V8FunctionCallback_Wrapper,
            v8::External::New(pIsolate,
                              const_cast<FXJSE_FUNCTION*>(lpFunctions + i))),
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
}
FXJSE_HCLASS FXJSE_DefineClass(FXJSE_HCONTEXT hContext,
                               const FXJSE_CLASS* lpClass) {
  CFXJSE_Context* lpContext = reinterpret_cast<CFXJSE_Context*>(hContext);
  ASSERT(lpContext);
  return reinterpret_cast<FXJSE_HCLASS>(
      CFXJSE_Class::Create(lpContext, lpClass, FALSE));
}
FXJSE_HCLASS FXJSE_GetClass(FXJSE_HCONTEXT hContext,
                            const CFX_ByteStringC& szName) {
  return reinterpret_cast<FXJSE_HCLASS>(CFXJSE_Class::GetClassFromContext(
      reinterpret_cast<CFXJSE_Context*>(hContext), szName));
}
static void FXJSE_V8FunctionCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_FUNCTION* lpFunctionInfo =
      static_cast<FXJSE_FUNCTION*>(info.Data().As<v8::External>()->Value());
  if (!lpFunctionInfo) {
    return;
  }
  CFX_ByteStringC szFunctionName(lpFunctionInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  CFXJSE_Value* lpRetValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_ArgumentsImpl impl = {&info, lpRetValue};
  lpFunctionInfo->callbackProc(reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                               szFunctionName,
                               reinterpret_cast<CFXJSE_Arguments&>(impl));
  if (!lpRetValue->DirectGetValue().IsEmpty()) {
    info.GetReturnValue().Set(lpRetValue->DirectGetValue());
  }
  delete lpRetValue;
  lpRetValue = NULL;
  delete lpThisValue;
  lpThisValue = NULL;
}
static void FXJSE_V8ClassGlobalConstructorCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS* lpClassDefinition =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClassDefinition) {
    return;
  }
  CFX_ByteStringC szFunctionName(lpClassDefinition->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  CFXJSE_Value* lpRetValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_ArgumentsImpl impl = {&info, lpRetValue};
  lpClassDefinition->constructor(reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                                 szFunctionName,
                                 reinterpret_cast<CFXJSE_Arguments&>(impl));
  if (!lpRetValue->DirectGetValue().IsEmpty()) {
    info.GetReturnValue().Set(lpRetValue->DirectGetValue());
  }
  delete lpRetValue;
  lpRetValue = NULL;
  delete lpThisValue;
  lpThisValue = NULL;
}
static void FXJSE_V8GetterCallback_Wrapper(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  const FXJSE_PROPERTY* lpPropertyInfo =
      static_cast<FXJSE_PROPERTY*>(info.Data().As<v8::External>()->Value());
  if (!lpPropertyInfo) {
    return;
  }
  CFX_ByteStringC szPropertyName(lpPropertyInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpPropValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpPropertyInfo->getProc(reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                          szPropertyName,
                          reinterpret_cast<FXJSE_HVALUE>(lpPropValue));
  info.GetReturnValue().Set(lpPropValue->DirectGetValue());
  delete lpThisValue;
  lpThisValue = NULL;
  delete lpPropValue;
  lpPropValue = NULL;
}
static void FXJSE_V8SetterCallback_Wrapper(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  const FXJSE_PROPERTY* lpPropertyInfo =
      static_cast<FXJSE_PROPERTY*>(info.Data().As<v8::External>()->Value());
  if (!lpPropertyInfo) {
    return;
  }
  CFX_ByteStringC szPropertyName(lpPropertyInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpPropValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpPropValue->ForceSetValue(value);
  lpPropertyInfo->setProc(reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                          szPropertyName,
                          reinterpret_cast<FXJSE_HVALUE>(lpPropValue));
  delete lpThisValue;
  lpThisValue = NULL;
  delete lpPropValue;
  lpPropValue = NULL;
}
static void FXJSE_V8ConstructorCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS* lpClassDefinition =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClassDefinition) {
    return;
  }
  FXSYS_assert(info.This()->InternalFieldCount());
  info.This()->SetAlignedPointerInInternalField(0, NULL);
}
FXJSE_HRUNTIME CFXJSE_Arguments::GetRuntime() const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return reinterpret_cast<FXJSE_HRUNTIME>(
      lpArguments->m_pRetValue->GetIsolate());
}
int32_t CFXJSE_Arguments::GetLength() const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return lpArguments->m_pInfo->Length();
}
FXJSE_HVALUE CFXJSE_Arguments::GetValue(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  CFXJSE_Value* lpArgValue = CFXJSE_Value::Create(v8::Isolate::GetCurrent());
  ASSERT(lpArgValue);
  lpArgValue->ForceSetValue((*lpArguments->m_pInfo)[index]);
  return reinterpret_cast<FXJSE_HVALUE>(lpArgValue);
}
FX_BOOL CFXJSE_Arguments::GetBoolean(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return (*lpArguments->m_pInfo)[index]->BooleanValue();
}
int32_t CFXJSE_Arguments::GetInt32(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return static_cast<int32_t>((*lpArguments->m_pInfo)[index]->NumberValue());
}
FX_FLOAT CFXJSE_Arguments::GetFloat(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return static_cast<FX_FLOAT>((*lpArguments->m_pInfo)[index]->NumberValue());
}
CFX_ByteString CFXJSE_Arguments::GetUTF8String(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  v8::Local<v8::String> hString = (*lpArguments->m_pInfo)[index]->ToString();
  v8::String::Utf8Value szStringVal(hString);
  return CFX_ByteString(*szStringVal);
}
void* CFXJSE_Arguments::GetObject(int32_t index, FXJSE_HCLASS hClass) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  v8::Local<v8::Value> hValue = (*lpArguments->m_pInfo)[index];
  ASSERT(!hValue.IsEmpty());
  if (!hValue->IsObject()) {
    return NULL;
  }
  CFXJSE_Class* lpClass = reinterpret_cast<CFXJSE_Class*>(hClass);
  return FXJSE_RetrieveObjectBinding(hValue.As<v8::Object>(), lpClass);
}
FXJSE_HVALUE CFXJSE_Arguments::GetReturnValue() {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return reinterpret_cast<FXJSE_HVALUE>(lpArguments->m_pRetValue);
}
static void FXJSE_Context_GlobalObjToString(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClass) {
    return;
  }
  if (info.This() == info.Holder() && lpClass->name) {
    CFX_ByteString szStringVal;
    szStringVal.Format("[object %s]", lpClass->name);
    info.GetReturnValue().Set(v8::String::NewFromUtf8(
        info.GetIsolate(), (const FX_CHAR*)szStringVal,
        v8::String::kNormalString, szStringVal.GetLength()));
  } else {
    info.GetReturnValue().Set(info.This()->ObjectProtoToString());
  }
}
CFXJSE_Class* CFXJSE_Class::Create(CFXJSE_Context* lpContext,
                                   const FXJSE_CLASS* lpClassDefinition,
                                   FX_BOOL bIsJSGlobal) {
  if (!lpContext || !lpClassDefinition) {
    return NULL;
  }
  CFXJSE_Class* pClass =
      GetClassFromContext(lpContext, lpClassDefinition->name);
  if (pClass) {
    return pClass;
  }
  v8::Isolate* pIsolate = lpContext->m_pIsolate;
  pClass = new CFXJSE_Class(lpContext);
  pClass->m_szClassName = lpClassDefinition->name;
  pClass->m_lpClassDefinition = lpClassDefinition;
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::FunctionTemplate> hFunctionTemplate = v8::FunctionTemplate::New(
      pIsolate, bIsJSGlobal ? 0 : FXJSE_V8ConstructorCallback_Wrapper,
      v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(lpClassDefinition)));
  hFunctionTemplate->SetClassName(
      v8::String::NewFromUtf8(pIsolate, lpClassDefinition->name));
  hFunctionTemplate->InstanceTemplate()->SetInternalFieldCount(1);
  v8::Local<v8::ObjectTemplate> hObjectTemplate =
      hFunctionTemplate->InstanceTemplate();
  SetUpNamedPropHandler(pIsolate, hObjectTemplate, lpClassDefinition);

  if (lpClassDefinition->propNum) {
    for (int32_t i = 0; i < lpClassDefinition->propNum; i++) {
      hObjectTemplate->SetNativeDataProperty(
          v8::String::NewFromUtf8(pIsolate,
                                  lpClassDefinition->properties[i].name),
          lpClassDefinition->properties[i].getProc
              ? FXJSE_V8GetterCallback_Wrapper
              : NULL,
          lpClassDefinition->properties[i].setProc
              ? FXJSE_V8SetterCallback_Wrapper
              : NULL,
          v8::External::New(pIsolate, const_cast<FXJSE_PROPERTY*>(
                                          lpClassDefinition->properties + i)),
          static_cast<v8::PropertyAttribute>(v8::DontDelete));
    }
  }
  if (lpClassDefinition->methNum) {
    for (int32_t i = 0; i < lpClassDefinition->methNum; i++) {
      hObjectTemplate->Set(
          v8::String::NewFromUtf8(pIsolate, lpClassDefinition->methods[i].name),
          v8::FunctionTemplate::New(
              pIsolate, FXJSE_V8FunctionCallback_Wrapper,
              v8::External::New(pIsolate, const_cast<FXJSE_FUNCTION*>(
                                              lpClassDefinition->methods + i))),
          static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
    }
  }
  if (lpClassDefinition->constructor) {
    if (bIsJSGlobal) {
      hObjectTemplate->Set(
          v8::String::NewFromUtf8(pIsolate, lpClassDefinition->name),
          v8::FunctionTemplate::New(
              pIsolate, FXJSE_V8ClassGlobalConstructorCallback_Wrapper,
              v8::External::New(pIsolate,
                                const_cast<FXJSE_CLASS*>(lpClassDefinition))),
          static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
    } else {
      v8::Local<v8::Context> hLocalContext =
          v8::Local<v8::Context>::New(pIsolate, lpContext->m_hContext);
      FXJSE_GetGlobalObjectFromContext(hLocalContext)
          ->Set(v8::String::NewFromUtf8(pIsolate, lpClassDefinition->name),
                v8::Function::New(
                    pIsolate, FXJSE_V8ClassGlobalConstructorCallback_Wrapper,
                    v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                    lpClassDefinition))));
    }
  }
  if (bIsJSGlobal) {
    hObjectTemplate->Set(
        v8::String::NewFromUtf8(pIsolate, "toString"),
        v8::FunctionTemplate::New(
            pIsolate, FXJSE_Context_GlobalObjToString,
            v8::External::New(pIsolate,
                              const_cast<FXJSE_CLASS*>(lpClassDefinition))));
  }
  pClass->m_hTemplate.Reset(lpContext->m_pIsolate, hFunctionTemplate);
  lpContext->m_rgClasses.Add(pClass);
  return pClass;
}
CFXJSE_Class* CFXJSE_Class::GetClassFromContext(CFXJSE_Context* pContext,
                                                const CFX_ByteStringC& szName) {
  for (int count = pContext->m_rgClasses.GetSize(), i = 0; i < count; i++) {
    CFXJSE_Class* pClass = pContext->m_rgClasses[i];
    if (pClass->m_szClassName == szName) {
      return pClass;
    }
  }
  return NULL;
}
