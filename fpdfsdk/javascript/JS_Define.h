// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_JS_DEFINE_H_
#define FPDFSDK_JAVASCRIPT_JS_DEFINE_H_

#include <vector>

#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/resource.h"
#include "fxjs/fxjs_v8.h"

// All JS classes have a name, an object defintion ID, and the ability to
// register themselves with FXJS_V8. We never make a BASE class on its own
// because it can't really do anything.

// Rich JS classes provide constants, methods, properties, and the ability
// to construct native object state.

template <class T, class A>
static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  CJS_Object* pObj = new T(obj);
  pObj->SetEmbedObject(new A(pObj));
  pEngine->SetObjectPrivate(obj, pObj);
  pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));
}

template <class T>
static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
  delete static_cast<T*>(pEngine->GetObjectPrivate(obj));
}

template <class C, CJS_Return (C::*M)(CJS_Runtime*)>
void JSPropGetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, prop_name_string,
                                        result.Error()));
    return;
  }

  if (result.HasReturn())
    info.GetReturnValue().Set(result.Return());
}

template <class C, CJS_Return (C::*M)(CJS_Runtime*, v8::Local<v8::Value>)>
void JSPropSetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime, value);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, prop_name_string,
                                        result.Error()));
  }
}

template <class C,
          CJS_Return (C::*M)(CJS_Runtime*,
                             const std::vector<v8::Local<v8::Value>>&)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  std::vector<v8::Local<v8::Value>> parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++)
    parameters.push_back(info[i]);

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CJS_Return result = (pObj->*M)(pRuntime, parameters);
  if (result.HasError()) {
    pRuntime->Error(JSFormatErrorString(class_name_string, method_name_string,
                                        result.Error()));
    return;
  }

  if (result.HasReturn())
    info.GetReturnValue().Set(result.Return());
}

#define JS_STATIC_PROP(err_name, prop_name, class_name)           \
  static void get_##prop_name##_static(                           \
      v8::Local<v8::String> property,                             \
      const v8::PropertyCallbackInfo<v8::Value>& info) {          \
    JSPropGetter<class_name, &class_name::get_##prop_name>(       \
        #err_name, #class_name, property, info);                  \
  }                                                               \
  static void set_##prop_name##_static(                           \
      v8::Local<v8::String> property, v8::Local<v8::Value> value, \
      const v8::PropertyCallbackInfo<void>& info) {               \
    JSPropSetter<class_name, &class_name::set_##prop_name>(       \
        #err_name, #class_name, property, value, info);           \
  }

#define JS_STATIC_METHOD(method_name, class_name)                             \
  static void method_name##_static(                                           \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                      \
    JSMethod<class_name, &class_name::method_name>(#method_name, #class_name, \
                                                   info);                     \
  }

#endif  // FPDFSDK_JAVASCRIPT_JS_DEFINE_H_
