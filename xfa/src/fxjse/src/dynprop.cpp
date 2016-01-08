// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fxv8.h"
#include "class.h"
#include "value.h"
static void FXJSE_DynPropGetterAdapter_MethodCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> hCallBackInfo = info.Data().As<v8::Object>();
  FXJSE_CLASS* lpClass = static_cast<FXJSE_CLASS*>(
      hCallBackInfo->GetAlignedPointerFromInternalField(0));
  v8::Local<v8::String> hPropName =
      hCallBackInfo->GetInternalField(1).As<v8::String>();
  ASSERT(lpClass && !hPropName.IsEmpty());
  v8::String::Utf8Value szPropName(hPropName);
  CFX_ByteStringC szFxPropName = *szPropName;
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  CFXJSE_Value* lpRetValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_ArgumentsImpl impl = {&info, lpRetValue};
  lpClass->dynMethodCall(reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                         szFxPropName,
                         reinterpret_cast<CFXJSE_Arguments&>(impl));
  if (!lpRetValue->DirectGetValue().IsEmpty()) {
    info.GetReturnValue().Set(lpRetValue->DirectGetValue());
  }
  delete lpRetValue;
  lpRetValue = nullptr;
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_DynPropGetterAdapter(const FXJSE_CLASS* lpClass,
                                       FXJSE_HOBJECT hObject,
                                       const CFX_ByteStringC& szPropName,
                                       FXJSE_HVALUE hValue) {
  ASSERT(lpClass);
  int32_t nPropType =
      lpClass->dynPropTypeGetter == nullptr
          ? FXJSE_ClassPropType_Property
          : lpClass->dynPropTypeGetter(hObject, szPropName, FALSE);
  if (nPropType == FXJSE_ClassPropType_Property) {
    if (lpClass->dynPropGetter) {
      lpClass->dynPropGetter(hObject, szPropName, hValue);
    }
  } else if (nPropType == FXJSE_ClassPropType_Method) {
    if (lpClass->dynMethodCall && hValue) {
      CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
      v8::Isolate* pIsolate = lpValue->GetIsolate();
      v8::HandleScope hscope(pIsolate);
      v8::Local<v8::ObjectTemplate> hCallBackInfoTemplate =
          v8::ObjectTemplate::New();
      hCallBackInfoTemplate->SetInternalFieldCount(2);
      v8::Local<v8::Object> hCallBackInfo =
          hCallBackInfoTemplate->NewInstance();
      hCallBackInfo->SetAlignedPointerInInternalField(
          0, const_cast<FXJSE_CLASS*>(lpClass));
      hCallBackInfo->SetInternalField(
          1, v8::String::NewFromUtf8(
                 pIsolate, reinterpret_cast<const char*>(szPropName.GetPtr()),
                 v8::String::kNormalString, szPropName.GetLength()));
      lpValue->ForceSetValue(v8::Function::New(
          lpValue->GetIsolate(), FXJSE_DynPropGetterAdapter_MethodCallback,
          hCallBackInfo));
    }
  }
}
static void FXJSE_DynPropSetterAdapter(const FXJSE_CLASS* lpClass,
                                       FXJSE_HOBJECT hObject,
                                       const CFX_ByteStringC& szPropName,
                                       FXJSE_HVALUE hValue) {
  ASSERT(lpClass);
  int32_t nPropType =
      lpClass->dynPropTypeGetter == nullptr
          ? FXJSE_ClassPropType_Property
          : lpClass->dynPropTypeGetter(hObject, szPropName, FALSE);
  if (nPropType != FXJSE_ClassPropType_Method) {
    if (lpClass->dynPropSetter) {
      lpClass->dynPropSetter(hObject, szPropName, hValue);
    }
  }
}
static FX_BOOL FXJSE_DynPropQueryAdapter(const FXJSE_CLASS* lpClass,
                                         FXJSE_HOBJECT hObject,
                                         const CFX_ByteStringC& szPropName) {
  ASSERT(lpClass);
  int32_t nPropType =
      lpClass->dynPropTypeGetter == nullptr
          ? FXJSE_ClassPropType_Property
          : lpClass->dynPropTypeGetter(hObject, szPropName, TRUE);
  return nPropType != FXJSE_ClassPropType_None;
}
static FX_BOOL FXJSE_DynPropDeleterAdapter(const FXJSE_CLASS* lpClass,
                                           FXJSE_HOBJECT hObject,
                                           const CFX_ByteStringC& szPropName) {
  ASSERT(lpClass);
  int32_t nPropType =
      lpClass->dynPropTypeGetter == nullptr
          ? FXJSE_ClassPropType_Property
          : lpClass->dynPropTypeGetter(hObject, szPropName, FALSE);
  if (nPropType != FXJSE_ClassPropType_Method) {
    if (lpClass->dynPropDeleter) {
      return lpClass->dynPropDeleter(hObject, szPropName);
    } else {
      return nPropType == FXJSE_ClassPropType_Property ? FALSE : TRUE;
    }
  }
  return FALSE;
}
static void FXJSE_V8ProxyCallback_getOwnPropertyDescriptor_getter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> hCallBackInfo = info.Data().As<v8::Object>();
  FXJSE_CLASS* lpClass = static_cast<FXJSE_CLASS*>(
      hCallBackInfo->GetAlignedPointerFromInternalField(0));
  v8::Local<v8::String> hPropName =
      hCallBackInfo->GetInternalField(1).As<v8::String>();
  ASSERT(lpClass && !hPropName.IsEmpty());
  v8::String::Utf8Value szPropName(hPropName);
  CFX_ByteStringC szFxPropName = *szPropName;
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpNewValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  FXJSE_DynPropGetterAdapter(
      lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName,
      reinterpret_cast<FXJSE_HVALUE>(lpNewValue));
  info.GetReturnValue().Set(lpNewValue->DirectGetValue());
  delete lpThisValue;
  lpThisValue = nullptr;
  delete lpNewValue;
  lpNewValue = nullptr;
}
static void FXJSE_V8ProxyCallback_getOwnPropertyDescriptor_setter(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> hCallBackInfo = info.Data().As<v8::Object>();
  FXJSE_CLASS* lpClass = static_cast<FXJSE_CLASS*>(
      hCallBackInfo->GetAlignedPointerFromInternalField(0));
  v8::Local<v8::String> hPropName =
      hCallBackInfo->GetInternalField(1).As<v8::String>();
  ASSERT(lpClass && !hPropName.IsEmpty());
  v8::String::Utf8Value szPropName(hPropName);
  CFX_ByteStringC szFxPropName = *szPropName;
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpNewValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpNewValue->ForceSetValue(info[0]);
  FXJSE_DynPropSetterAdapter(
      lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName,
      reinterpret_cast<FXJSE_HVALUE>(lpNewValue));
  delete lpThisValue;
  lpThisValue = nullptr;
  delete lpNewValue;
  lpNewValue = nullptr;
}
static void FXJSE_V8ProxyCallback_getOwnPropertyDescriptor(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClass) {
    return;
  }
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  v8::Local<v8::String> hPropName = info[0]->ToString();
  v8::String::Utf8Value szPropName(hPropName);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  v8::Local<v8::ObjectTemplate> hCallBackInfoTemplate =
      v8::ObjectTemplate::New();
  hCallBackInfoTemplate->SetInternalFieldCount(2);
  v8::Local<v8::Object> hCallBackInfo = hCallBackInfoTemplate->NewInstance();
  hCallBackInfo->SetAlignedPointerInInternalField(
      0, const_cast<FXJSE_CLASS*>(lpClass));
  hCallBackInfo->SetInternalField(1, hPropName);
  v8::Local<v8::Object> hPropDescriptor = v8::Object::New(pIsolate);
  hPropDescriptor->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "get"),
      v8::Function::New(pIsolate,
                        FXJSE_V8ProxyCallback_getOwnPropertyDescriptor_getter,
                        hCallBackInfo));
  hPropDescriptor->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "set"),
      v8::Function::New(pIsolate,
                        FXJSE_V8ProxyCallback_getOwnPropertyDescriptor_setter,
                        hCallBackInfo));
  hPropDescriptor->ForceSet(v8::String::NewFromUtf8(pIsolate, "enumerable"),
                            v8::Boolean::New(pIsolate, false));
  hPropDescriptor->ForceSet(v8::String::NewFromUtf8(pIsolate, "configurable"),
                            v8::Boolean::New(pIsolate, true));
  info.GetReturnValue().Set(hPropDescriptor);
}
static void FXJSE_V8ProxyCallback_getPropertyDescriptor(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::Local<v8::Object> hChainObj =
      info.This()->GetPrototype().As<v8::Object>();
  v8::Local<v8::Script> fnSource = v8::Script::Compile(v8::String::NewFromUtf8(
      pIsolate,
      "(function (o, name) { var fn, x, d; fn = "
      "Object.getOwnPropertyDescriptor; x = o; while(x && !(d = fn(x, "
      "name))){x = x.__proto__;} return d; })"));
  v8::Local<v8::Function> fn = fnSource->Run().As<v8::Function>();
  v8::Local<v8::Value> rgArgs[] = {hChainObj, info[0]};
  v8::Local<v8::Value> hChainDescriptor = fn->Call(info.This(), 2, rgArgs);
  if (!hChainDescriptor.IsEmpty() && hChainDescriptor->IsObject()) {
    info.GetReturnValue().Set(hChainDescriptor);
  } else {
    FXJSE_V8ProxyCallback_getOwnPropertyDescriptor(info);
  }
}
static void FXJSE_V8ProxyCallback_getOwnPropertyNames(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  info.GetReturnValue().Set(v8::Array::New(pIsolate));
}
static void FXJSE_V8ProxyCallback_getPropertyNames(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> hChainObj =
      info.This()->GetPrototype().As<v8::Object>();
  v8::Local<v8::Value> hChainPropertyNames = hChainObj->GetPropertyNames();
  info.GetReturnValue().Set(hChainPropertyNames);
}
static void FXJSE_V8ProxyCallback_defineProperty(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClass) {
    return;
  }
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  v8::Local<v8::String> hPropName = info[0]->ToString();
  v8::Local<v8::Object> hPropDescriptor = info[1]->ToObject();
  v8::String::Utf8Value szPropName(hPropName);
  if (!hPropDescriptor->Has(v8::String::NewFromUtf8(pIsolate, "value"))) {
    return;
  }
  v8::Local<v8::Value> hPropValue =
      hPropDescriptor->Get(v8::String::NewFromUtf8(pIsolate, "value"));
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  CFXJSE_Value* lpPropValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  lpPropValue->ForceSetValue(hPropValue);
  FXJSE_DynPropSetterAdapter(
      lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName,
      reinterpret_cast<FXJSE_HVALUE>(lpPropValue));
  delete lpThisValue;
  lpThisValue = nullptr;
  delete lpPropValue;
  lpPropValue = nullptr;
}
static void FXJSE_V8ProxyCallback_delete(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(true);
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  if (!lpClass) {
    return;
  }
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  v8::Local<v8::String> hPropName = info[0]->ToString();
  v8::String::Utf8Value szPropName(hPropName);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(info.This());
  info.GetReturnValue().Set(
      FXJSE_DynPropDeleterAdapter(
          lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName)
          ? true
          : false);
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_V8ProxyCallback_fix(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().SetUndefined();
}
static void FXJSE_V8_GenericNamedPropertyQueryCallback(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Integer>& info) {
  v8::Local<v8::Object> thisObject = info.This();
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  v8::String::Utf8Value szPropName(property);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(thisObject);
  if (FXJSE_DynPropQueryAdapter(lpClass,
                                reinterpret_cast<FXJSE_HOBJECT>(lpThisValue),
                                szFxPropName)) {
    info.GetReturnValue().Set(v8::DontDelete);
  } else {
    const int32_t iV8Absent = 64;
    info.GetReturnValue().Set(iV8Absent);
  }
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_V8_GenericNamedPropertyDeleterCallback(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  v8::Local<v8::Object> thisObject = info.This();
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::HandleScope scope(pIsolate);
  v8::String::Utf8Value szPropName(property);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(thisObject);
  info.GetReturnValue().Set(
      FXJSE_DynPropDeleterAdapter(
          lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName)
          ? true
          : false);
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_V8_GenericNamedPropertyGetterCallback(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> thisObject = info.This();
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  v8::String::Utf8Value szPropName(property);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(thisObject);
  CFXJSE_Value* lpNewValue = CFXJSE_Value::Create(info.GetIsolate());
  FXJSE_DynPropGetterAdapter(
      lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName,
      reinterpret_cast<FXJSE_HVALUE>(lpNewValue));
  info.GetReturnValue().Set(lpNewValue->DirectGetValue());
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_V8_GenericNamedPropertySetterCallback(
    v8::Local<v8::Name> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> thisObject = info.This();
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  v8::String::Utf8Value szPropName(property);
  CFX_ByteStringC szFxPropName(*szPropName, szPropName.length());
  CFXJSE_Value* lpThisValue = CFXJSE_Value::Create(info.GetIsolate());
  lpThisValue->ForceSetValue(thisObject);
  CFXJSE_Value* lpNewValue = CFXJSE_Value::Create(info.GetIsolate());
  lpNewValue->ForceSetValue(value);
  FXJSE_DynPropSetterAdapter(
      lpClass, reinterpret_cast<FXJSE_HOBJECT>(lpThisValue), szFxPropName,
      reinterpret_cast<FXJSE_HVALUE>(lpNewValue));
  info.GetReturnValue().Set(value);
  delete lpThisValue;
  lpThisValue = nullptr;
}
static void FXJSE_V8_GenericNamedPropertyEnumeratorCallback(
    const v8::PropertyCallbackInfo<v8::Array>& info) {
  const FXJSE_CLASS* lpClass =
      static_cast<FXJSE_CLASS*>(info.Data().As<v8::External>()->Value());
  v8::Isolate* pIsolate = info.GetIsolate();
  v8::Local<v8::Array> newArray = v8::Array::New(pIsolate, lpClass->propNum);
  for (int i = 0; i < lpClass->propNum; i++) {
    newArray->Set(
        i, v8::String::NewFromUtf8(pIsolate, lpClass->properties[i].name));
  }
  info.GetReturnValue().Set(newArray);
}

void CFXJSE_Class::SetUpDynPropHandler(CFXJSE_Context* pContext,
                                       CFXJSE_Value* pValue,
                                       const FXJSE_CLASS* lpClassDefinition) {
  v8::Isolate* pIsolate = pValue->GetIsolate();
  CFXJSE_ScopeUtil_IsolateHandleRootOrNormalContext scope(pIsolate, pContext);
  v8::Local<v8::Context> hContext = v8::Local<v8::Context>::New(
      pIsolate, pContext ? pContext->m_hContext
                         : CFXJSE_RuntimeData::Get(pIsolate)->m_hRootContext);
  v8::Local<v8::Object> hObject =
      v8::Local<v8::Value>::New(pIsolate, pValue->m_hValue).As<v8::Object>();
  v8::Local<v8::Object> hHarmonyProxyObj =
      hContext->Global()
          ->Get(v8::String::NewFromUtf8(pIsolate, "Proxy"))
          .As<v8::Object>();
  v8::Local<v8::Function> hHarmonyProxyCreateFn =
      hHarmonyProxyObj->Get(v8::String::NewFromUtf8(pIsolate, "create"))
          .As<v8::Function>();
  v8::Local<v8::Value> hOldPrototype = hObject->GetPrototype();
  v8::Local<v8::Object> hTrapper = v8::Object::New(pIsolate);
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "getOwnPropertyDescriptor"),
      v8::Function::New(
          pIsolate, FXJSE_V8ProxyCallback_getOwnPropertyDescriptor,
          v8::External::New(pIsolate,
                            const_cast<FXJSE_CLASS*>(lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "getPropertyDescriptor"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_getPropertyDescriptor,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "getOwnPropertyNames"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_getOwnPropertyNames,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "getPropertyNames"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_getPropertyNames,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "delete"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_delete,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "defineProperty"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_defineProperty,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  hTrapper->ForceSet(
      v8::String::NewFromUtf8(pIsolate, "fix"),
      v8::Function::New(pIsolate, FXJSE_V8ProxyCallback_fix,
                        v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(
                                                        lpClassDefinition))));
  v8::Local<v8::Value> rgArgs[] = {hTrapper, hOldPrototype};
  v8::Local<v8::Value> hNewPrototype =
      hHarmonyProxyCreateFn->Call(hHarmonyProxyObj, 2, rgArgs);
  hObject->SetPrototype(hNewPrototype);
}
void CFXJSE_Class::SetUpNamedPropHandler(
    v8::Isolate* pIsolate,
    v8::Local<v8::ObjectTemplate>& hObjectTemplate,
    const FXJSE_CLASS* lpClassDefinition) {
  v8::NamedPropertyHandlerConfiguration configuration(
      lpClassDefinition->dynPropGetter
          ? FXJSE_V8_GenericNamedPropertyGetterCallback
          : 0,
      lpClassDefinition->dynPropSetter
          ? FXJSE_V8_GenericNamedPropertySetterCallback
          : 0,
      lpClassDefinition->dynPropTypeGetter
          ? FXJSE_V8_GenericNamedPropertyQueryCallback
          : 0,
      lpClassDefinition->dynPropDeleter
          ? FXJSE_V8_GenericNamedPropertyDeleterCallback
          : 0,
      FXJSE_V8_GenericNamedPropertyEnumeratorCallback,
      v8::External::New(pIsolate, const_cast<FXJSE_CLASS*>(lpClassDefinition)),
      v8::PropertyHandlerFlags::kNonMasking);
  hObjectTemplate->SetHandler(configuration);
}
