// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_class.h"

#include <memory>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "fxjs/cjs_result.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-container.h"
#include "v8/include/v8-external.h"
#include "v8/include/v8-function-callback.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "v8/include/v8-template.h"

using pdfium::fxjse::kClassTag;
using pdfium::fxjse::kFuncTag;

namespace {

FXJSE_FUNCTION_DESCRIPTOR* AsFunctionDescriptor(void* ptr) {
  auto* result = static_cast<FXJSE_FUNCTION_DESCRIPTOR*>(ptr);
  return result && result->tag == kFuncTag ? result : nullptr;
}

FXJSE_CLASS_DESCRIPTOR* AsClassDescriptor(void* ptr) {
  auto* result = static_cast<FXJSE_CLASS_DESCRIPTOR*>(ptr);
  return result && result->tag == kClassTag ? result : nullptr;
}

void V8FunctionCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_FUNCTION_DESCRIPTOR* pFunctionInfo =
      AsFunctionDescriptor(info.Data().As<v8::External>()->Value());
  if (!pFunctionInfo) {
    return;
  }

  pFunctionInfo->callbackProc(CFXJSE_HostObject::FromV8(info.This()), info);
}

void V8ConstructorCallback_Wrapper(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (!info.IsConstructCall()) {
    return;
  }

  const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor =
      AsClassDescriptor(info.Data().As<v8::External>()->Value());
  if (!pClassDescriptor) {
    return;
  }

  DCHECK_EQ(info.This()->InternalFieldCount(), 2);
  info.This()->SetAlignedPointerInInternalField(0, nullptr);
  info.This()->SetAlignedPointerInInternalField(1, nullptr);
}

void Context_GlobalObjToString(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor =
      AsClassDescriptor(info.Data().As<v8::External>()->Value());
  if (!pClassDescriptor) {
    return;
  }

  if (pClassDescriptor->name) {
    ByteString szStringVal =
        ByteString::Format("[object %s]", pClassDescriptor->name);
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), szStringVal.AsStringView()));
    return;
  }
  v8::Local<v8::String> local_str =
      info.This()
          ->ObjectProtoToString(info.GetIsolate()->GetCurrentContext())
          .FromMaybe(v8::Local<v8::String>());
  info.GetReturnValue().Set(local_str);
}

void DynPropGetterAdapter_MethodCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Object> hCallBackInfo = info.Data().As<v8::Object>();
  if (hCallBackInfo->InternalFieldCount() != 2) {
    return;
  }

  auto* pClassDescriptor = static_cast<const FXJSE_CLASS_DESCRIPTOR*>(
      hCallBackInfo->GetAlignedPointerFromInternalField(0));
  if (pClassDescriptor != &kGlobalClassDescriptor &&
      pClassDescriptor != &kNormalClassDescriptor &&
      pClassDescriptor != &kVariablesClassDescriptor &&
      pClassDescriptor != &kFormCalcDescriptor) {
    return;
  }

  v8::Local<v8::String> hPropName =
      hCallBackInfo->GetInternalField(1).As<v8::Value>().As<v8::String>();
  if (hPropName.IsEmpty()) {
    return;
  }

  v8::String::Utf8Value szPropName(info.GetIsolate(), hPropName);
  CJS_Result result =
      pClassDescriptor->dynMethodCall(info, WideString::FromUTF8(*szPropName));

  if (result.HasError()) {
    WideString err = JSFormatErrorString(pClassDescriptor->name, *szPropName,
                                         result.Error());
    fxv8::ThrowExceptionHelper(info.GetIsolate(), err.AsStringView());
    return;
  }

  if (result.HasReturn()) {
    info.GetReturnValue().Set(result.Return());
  }
}

std::unique_ptr<CFXJSE_Value> DynPropGetterAdapter(
    v8::Isolate* pIsolate,
    const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor,
    v8::Local<v8::Object> pObject,
    ByteStringView szPropName) {
  FXJSE_ClassPropType nPropType =
      pClassDescriptor->dynPropTypeGetter
          ? pClassDescriptor->dynPropTypeGetter(pIsolate, pObject, szPropName,
                                                false)
          : FXJSE_ClassPropType::kProperty;
  if (nPropType == FXJSE_ClassPropType::kProperty) {
    if (pClassDescriptor->dynPropGetter) {
      return std::make_unique<CFXJSE_Value>(
          pIsolate,
          pClassDescriptor->dynPropGetter(pIsolate, pObject, szPropName));
    }
  } else if (nPropType == FXJSE_ClassPropType::kMethod) {
    if (pClassDescriptor->dynMethodCall) {
      v8::HandleScope hscope(pIsolate);
      v8::Local<v8::ObjectTemplate> hCallBackInfoTemplate =
          v8::ObjectTemplate::New(pIsolate);
      hCallBackInfoTemplate->SetInternalFieldCount(2);
      v8::Local<v8::Object> hCallBackInfo =
          hCallBackInfoTemplate->NewInstance(pIsolate->GetCurrentContext())
              .ToLocalChecked();
      hCallBackInfo->SetAlignedPointerInInternalField(
          0, const_cast<FXJSE_CLASS_DESCRIPTOR*>(pClassDescriptor));
      hCallBackInfo->SetInternalField(
          1, fxv8::NewStringHelper(pIsolate, szPropName));
      return std::make_unique<CFXJSE_Value>(
          pIsolate,
          v8::Function::New(pIsolate->GetCurrentContext(),
                            DynPropGetterAdapter_MethodCallback, hCallBackInfo,
                            0, v8::ConstructorBehavior::kThrow)
              .ToLocalChecked());
    }
  }
  return std::make_unique<CFXJSE_Value>();
}

void DynPropSetterAdapter(v8::Isolate* pIsolate,
                          const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor,
                          v8::Local<v8::Object> pObject,
                          ByteStringView szPropName,
                          CFXJSE_Value* pValue) {
  DCHECK(pClassDescriptor);
  FXJSE_ClassPropType nPropType =
      pClassDescriptor->dynPropTypeGetter
          ? pClassDescriptor->dynPropTypeGetter(pIsolate, pObject, szPropName,
                                                false)
          : FXJSE_ClassPropType::kProperty;
  if (nPropType != FXJSE_ClassPropType::kMethod) {
    if (pClassDescriptor->dynPropSetter) {
      pClassDescriptor->dynPropSetter(pIsolate, pObject, szPropName,
                                      pValue->GetValue(pIsolate));
    }
  }
}

bool DynPropQueryAdapter(v8::Isolate* pIsolate,
                         const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor,
                         v8::Local<v8::Object> pObject,
                         ByteStringView szPropName) {
  FXJSE_ClassPropType nPropType = pClassDescriptor->dynPropTypeGetter
                                      ? pClassDescriptor->dynPropTypeGetter(
                                            pIsolate, pObject, szPropName, true)
                                      : FXJSE_ClassPropType::kProperty;
  return nPropType != FXJSE_ClassPropType::kNone;
}

v8::Intercepted NamedPropertyQueryCallback(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Integer>& info) {
  const FXJSE_CLASS_DESCRIPTOR* pClass =
      AsClassDescriptor(info.Data().As<v8::External>()->Value());
  if (!pClass) {
    return v8::Intercepted::kNo;
  }

  v8::HandleScope scope(info.GetIsolate());
  v8::String::Utf8Value szPropName(info.GetIsolate(), property);
  // SAFETY: required from V8.
  auto szFxPropName =
      UNSAFE_BUFFERS(ByteStringView(*szPropName, szPropName.length()));
  if (DynPropQueryAdapter(info.GetIsolate(), pClass, info.HolderV2(),
                          szFxPropName)) {
    info.GetReturnValue().Set(v8::DontDelete);
    return v8::Intercepted::kYes;
  }

  return v8::Intercepted::kNo;
}

v8::Intercepted NamedPropertyGetterCallback(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  const FXJSE_CLASS_DESCRIPTOR* pClass =
      AsClassDescriptor(info.Data().As<v8::External>()->Value());
  if (!pClass) {
    return v8::Intercepted::kNo;
  }

  v8::String::Utf8Value szPropName(info.GetIsolate(), property);
  // SAFETY: required from V8.
  auto szFxPropName =
      UNSAFE_BUFFERS(ByteStringView(*szPropName, szPropName.length()));
  std::unique_ptr<CFXJSE_Value> pNewValue = DynPropGetterAdapter(
      info.GetIsolate(), pClass, info.HolderV2(), szFxPropName);
  info.GetReturnValue().Set(pNewValue->DirectGetValue());
  return v8::Intercepted::kYes;
}

v8::Intercepted NamedPropertySetterCallback(
    v8::Local<v8::Name> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  const FXJSE_CLASS_DESCRIPTOR* pClass =
      AsClassDescriptor(info.Data().As<v8::External>()->Value());
  if (!pClass) {
    return v8::Intercepted::kNo;
  }

  v8::String::Utf8Value szPropName(info.GetIsolate(), property);
  // SAFETY: required from V8.
  auto szFxPropName =
      UNSAFE_BUFFERS(ByteStringView(*szPropName, szPropName.length()));
  auto pNewValue = std::make_unique<CFXJSE_Value>(info.GetIsolate(), value);
  DynPropSetterAdapter(info.GetIsolate(), pClass, info.HolderV2(), szFxPropName,
                       pNewValue.get());
  return v8::Intercepted::kYes;
}

void NamedPropertyEnumeratorCallback(
    const v8::PropertyCallbackInfo<v8::Array>& info) {
  info.GetReturnValue().Set(v8::Array::New(info.GetIsolate()));
}

void SetUpNamedPropHandler(v8::Isolate* pIsolate,
                           v8::Local<v8::ObjectTemplate> pObjectTemplate,
                           const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor) {
  v8::NamedPropertyHandlerConfiguration configuration(
      pClassDescriptor->dynPropGetter ? NamedPropertyGetterCallback : nullptr,
      pClassDescriptor->dynPropSetter ? NamedPropertySetterCallback : nullptr,
      pClassDescriptor->dynPropTypeGetter ? NamedPropertyQueryCallback
                                          : nullptr,
      nullptr, NamedPropertyEnumeratorCallback,
      v8::External::New(pIsolate,
                        const_cast<FXJSE_CLASS_DESCRIPTOR*>(pClassDescriptor)),
      v8::PropertyHandlerFlags::kNonMasking);
  pObjectTemplate->SetHandler(configuration);
}

}  // namespace

// static
CFXJSE_Class* CFXJSE_Class::Create(
    CFXJSE_Context* pContext,
    const FXJSE_CLASS_DESCRIPTOR* pClassDescriptor,
    bool bIsJSGlobal) {
  if (!pContext || !pClassDescriptor) {
    return nullptr;
  }

  CFXJSE_Class* pExistingClass =
      pContext->GetClassByName(pClassDescriptor->name);
  if (pExistingClass) {
    return pExistingClass;
  }

  v8::Isolate* pIsolate = pContext->GetIsolate();
  auto pClass = std::make_unique<CFXJSE_Class>(pContext);
  pClass->class_name_ = pClassDescriptor->name;
  pClass->class_descriptor_ = pClassDescriptor;
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::FunctionTemplate> hFunctionTemplate = v8::FunctionTemplate::New(
      pIsolate, bIsJSGlobal ? nullptr : V8ConstructorCallback_Wrapper,
      v8::External::New(pIsolate,
                        const_cast<FXJSE_CLASS_DESCRIPTOR*>(pClassDescriptor)));
  v8::Local<v8::String> classname =
      fxv8::NewStringHelper(pIsolate, pClassDescriptor->name);
  hFunctionTemplate->SetClassName(classname);
  hFunctionTemplate->PrototypeTemplate()->Set(
      v8::Symbol::GetToStringTag(pIsolate), classname,
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontEnum));
  hFunctionTemplate->InstanceTemplate()->SetInternalFieldCount(2);
  v8::Local<v8::ObjectTemplate> hObjectTemplate =
      hFunctionTemplate->InstanceTemplate();
  SetUpNamedPropHandler(pIsolate, hObjectTemplate, pClassDescriptor);

  for (const auto& method : pClassDescriptor->methods) {
    v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(
        pIsolate, V8FunctionCallback_Wrapper,
        v8::External::New(pIsolate,
                          const_cast<FXJSE_FUNCTION_DESCRIPTOR*>(&method)));
    fun->RemovePrototype();
    hObjectTemplate->Set(
        fxv8::NewStringHelper(pIsolate, method.name), fun,
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }

  if (bIsJSGlobal) {
    v8::Local<v8::FunctionTemplate> fn = v8::FunctionTemplate::New(
        pIsolate, Context_GlobalObjToString,
        v8::External::New(
            pIsolate, const_cast<FXJSE_CLASS_DESCRIPTOR*>(pClassDescriptor)));
    fn->RemovePrototype();
    hObjectTemplate->Set(fxv8::NewStringHelper(pIsolate, "toString"), fn);
  }
  pClass->func_template_.Reset(pContext->GetIsolate(), hFunctionTemplate);
  CFXJSE_Class* pResult = pClass.get();
  pContext->AddClass(std::move(pClass));
  return pResult;
}

CFXJSE_Class::CFXJSE_Class(const CFXJSE_Context* pContext)
    : context_(pContext) {}

CFXJSE_Class::~CFXJSE_Class() = default;

v8::Local<v8::FunctionTemplate> CFXJSE_Class::GetTemplate(
    v8::Isolate* pIsolate) {
  return v8::Local<v8::FunctionTemplate>::New(pIsolate, func_template_);
}
