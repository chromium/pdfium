// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxjse/class.h"

#include "xfa/fxjse/cfxjse_arguments.h"
#include "xfa/fxjse/context.h"
#include "xfa/fxjse/scope_inline.h"
#include "xfa/fxjse/util_inline.h"
#include "xfa/fxjse/value.h"

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

CFXJSE_Class* FXJSE_DefineClass(CFXJSE_Context* pContext,
                                const FXJSE_CLASS_DESCRIPTOR* lpClass) {
  ASSERT(pContext);
  return CFXJSE_Class::Create(pContext, lpClass, FALSE);
}

static void FXJSE_V8FunctionCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_FUNCTION_DESCRIPTOR* lpFunctionInfo =
      static_cast<FXJSE_FUNCTION_DESCRIPTOR*>(
          info.Data().As<v8::External>()->Value());
  if (!lpFunctionInfo) {
    return;
  }
  CFX_ByteStringC szFunctionName(lpFunctionInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  CFXJSE_Value* lpRetValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_ArgumentsImpl impl = {&info, lpRetValue};
  lpFunctionInfo->callbackProc(lpThisValue, szFunctionName,
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
  const FXJSE_CLASS_DESCRIPTOR* lpClassDefinition =
      static_cast<FXJSE_CLASS_DESCRIPTOR*>(
          info.Data().As<v8::External>()->Value());
  if (!lpClassDefinition) {
    return;
  }
  CFX_ByteStringC szFunctionName(lpClassDefinition->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  CFXJSE_Value* lpRetValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_ArgumentsImpl impl = {&info, lpRetValue};
  lpClassDefinition->constructor(lpThisValue, szFunctionName,
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
  const FXJSE_PROPERTY_DESCRIPTOR* lpPropertyInfo =
      static_cast<FXJSE_PROPERTY_DESCRIPTOR*>(
          info.Data().As<v8::External>()->Value());
  if (!lpPropertyInfo) {
    return;
  }
  CFX_ByteStringC szPropertyName(lpPropertyInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpPropValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpPropertyInfo->getProc(lpThisValue, szPropertyName, lpPropValue);
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
  const FXJSE_PROPERTY_DESCRIPTOR* lpPropertyInfo =
      static_cast<FXJSE_PROPERTY_DESCRIPTOR*>(
          info.Data().As<v8::External>()->Value());
  if (!lpPropertyInfo) {
    return;
  }
  CFX_ByteStringC szPropertyName(lpPropertyInfo->name);
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpPropValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpPropValue->ForceSetValue(value);
  lpPropertyInfo->setProc(lpThisValue, szPropertyName, lpPropValue);
  delete lpThisValue;
  lpThisValue = NULL;
  delete lpPropValue;
  lpPropValue = NULL;
}

static void FXJSE_V8ConstructorCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS_DESCRIPTOR* lpClassDefinition =
      static_cast<FXJSE_CLASS_DESCRIPTOR*>(
          info.Data().As<v8::External>()->Value());
  if (!lpClassDefinition) {
    return;
  }
  ASSERT(info.This()->InternalFieldCount());
  info.This()->SetAlignedPointerInInternalField(0, NULL);
}

v8::Isolate* CFXJSE_Arguments::GetRuntime() const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return lpArguments->m_pRetValue->GetIsolate();
}

int32_t CFXJSE_Arguments::GetLength() const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return lpArguments->m_pInfo->Length();
}

CFXJSE_Value* CFXJSE_Arguments::GetValue(int32_t index) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  CFXJSE_Value* lpArgValue = CFXJSE_Value::Create(v8::Isolate::GetCurrent());
  ASSERT(lpArgValue);
  lpArgValue->ForceSetValue((*lpArguments->m_pInfo)[index]);
  return lpArgValue;
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

void* CFXJSE_Arguments::GetObject(int32_t index, CFXJSE_Class* pClass) const {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  v8::Local<v8::Value> hValue = (*lpArguments->m_pInfo)[index];
  ASSERT(!hValue.IsEmpty());
  if (!hValue->IsObject()) {
    return NULL;
  }
  return FXJSE_RetrieveObjectBinding(hValue.As<v8::Object>(), pClass);
}

CFXJSE_Value* CFXJSE_Arguments::GetReturnValue() {
  const CFXJSE_ArgumentsImpl* lpArguments =
      reinterpret_cast<const CFXJSE_ArgumentsImpl* const>(this);
  return lpArguments->m_pRetValue;
}
static void FXJSE_Context_GlobalObjToString(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS_DESCRIPTOR* lpClass = static_cast<FXJSE_CLASS_DESCRIPTOR*>(
      info.Data().As<v8::External>()->Value());
  if (!lpClass) {
    return;
  }
  if (info.This() == info.Holder() && lpClass->name) {
    CFX_ByteString szStringVal;
    szStringVal.Format("[object %s]", lpClass->name);
    info.GetReturnValue().Set(v8::String::NewFromUtf8(
        info.GetIsolate(), szStringVal.c_str(), v8::String::kNormalString,
        szStringVal.GetLength()));
  } else {
    v8::Local<v8::String> local_str =
        info.This()
            ->ObjectProtoToString(info.GetIsolate()->GetCurrentContext())
            .FromMaybe(v8::Local<v8::String>());
    info.GetReturnValue().Set(local_str);
  }
}

CFXJSE_Class* CFXJSE_Class::Create(
    CFXJSE_Context* lpContext,
    const FXJSE_CLASS_DESCRIPTOR* lpClassDefinition,
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
      v8::External::New(
          pIsolate, const_cast<FXJSE_CLASS_DESCRIPTOR*>(lpClassDefinition)));
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
          v8::External::New(pIsolate, const_cast<FXJSE_PROPERTY_DESCRIPTOR*>(
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
              v8::External::New(pIsolate,
                                const_cast<FXJSE_FUNCTION_DESCRIPTOR*>(
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
              v8::External::New(pIsolate, const_cast<FXJSE_CLASS_DESCRIPTOR*>(
                                              lpClassDefinition))),
          static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
    } else {
      v8::Local<v8::Context> hLocalContext =
          v8::Local<v8::Context>::New(pIsolate, lpContext->m_hContext);
      FXJSE_GetGlobalObjectFromContext(hLocalContext)
          ->Set(v8::String::NewFromUtf8(pIsolate, lpClassDefinition->name),
                v8::Function::New(
                    pIsolate, FXJSE_V8ClassGlobalConstructorCallback_Wrapper,
                    v8::External::New(pIsolate,
                                      const_cast<FXJSE_CLASS_DESCRIPTOR*>(
                                          lpClassDefinition))));
    }
  }
  if (bIsJSGlobal) {
    hObjectTemplate->Set(
        v8::String::NewFromUtf8(pIsolate, "toString"),
        v8::FunctionTemplate::New(
            pIsolate, FXJSE_Context_GlobalObjToString,
            v8::External::New(pIsolate, const_cast<FXJSE_CLASS_DESCRIPTOR*>(
                                            lpClassDefinition))));
  }
  pClass->m_hTemplate.Reset(lpContext->m_pIsolate, hFunctionTemplate);
  lpContext->m_rgClasses.push_back(std::unique_ptr<CFXJSE_Class>(pClass));
  return pClass;
}

CFXJSE_Class* CFXJSE_Class::GetClassFromContext(CFXJSE_Context* pContext,
                                                const CFX_ByteStringC& szName) {
  for (const auto& pClass : pContext->m_rgClasses) {
    if (pClass->m_szClassName == szName)
      return pClass.get();
  }
  return nullptr;
}
