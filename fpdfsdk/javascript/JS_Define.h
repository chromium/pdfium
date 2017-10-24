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

struct JSConstSpec {
  enum Type { Number = 0, String = 1 };

  const char* pName;
  Type eType;
  double number;
  const char* pStr;
};

struct JSPropertySpec {
  const char* pName;
  v8::AccessorGetterCallback pPropGet;
  v8::AccessorSetterCallback pPropPut;
};

struct JSMethodSpec {
  const char* pName;
  v8::FunctionCallback pMethodCall;
};

template <class C, bool (C::*M)(CJS_Runtime*, CJS_Value*, WideString*)>
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
  WideString sError;

  CJS_Value prop_value;
  if (!(pObj->*M)(pRuntime, &prop_value, &sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, prop_name_string, sError));
    return;
  }
  info.GetReturnValue().Set(prop_value.ToV8Value());
}

template <class C, bool (C::*M)(CJS_Runtime*, const CJS_Value&, WideString*)>
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
  WideString sError;

  CJS_Value prop_value(value);
  if (!(pObj->*M)(pRuntime, prop_value, &sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, prop_name_string, sError));
  }
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

template <class C,
          bool (C::*M)(CJS_Runtime*,
                       const std::vector<CJS_Value>&,
                       CJS_Value&,
                       WideString&)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  std::vector<CJS_Value> parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++)
    parameters.push_back(CJS_Value(info[i]));

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  WideString sError;
  CJS_Value valueRes;
  if (!(pObj->*M)(pRuntime, parameters, valueRes, sError)) {
    pRuntime->Error(
        JSFormatErrorString(class_name_string, method_name_string, sError));
    return;
  }
  info.GetReturnValue().Set(valueRes.ToV8Value());
}

#define JS_STATIC_METHOD(method_name, class_name)                             \
  static void method_name##_static(                                           \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                      \
    JSMethod<class_name, &class_name::method_name>(#method_name, #class_name, \
                                                   info);                     \
  }

// All JS classes have a name, an object defintion ID, and the ability to
// register themselves with FXJS_V8. We never make a BASE class on its own
// because it can't really do anything.
#define DECLARE_JS_CLASS_BASE_PART() \
  static const char* g_pClassName;   \
  static int g_nObjDefnID;           \
  static void DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType);

#define IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name) \
  const char* js_class_name::g_pClassName = #class_name;        \
  int js_class_name::g_nObjDefnID = -1;

#define DECLARE_JS_CLASS_CONST_PART() \
  static JSConstSpec ConstSpecs[];    \
  static void DefineConsts(CFXJS_Engine* pEngine);

#define IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)         \
  void js_class_name::DefineConsts(CFXJS_Engine* pEngine) {              \
    for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {          \
      pEngine->DefineObjConst(                                           \
          g_nObjDefnID, ConstSpecs[i].pName,                             \
          ConstSpecs[i].eType == JSConstSpec::Number                     \
              ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>() \
              : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>()); \
    }                                                                    \
  }

// Rich JS classes provide constants, methods, properties, and the ability
// to construct native object state.
#define DECLARE_JS_CLASS()      \
  DECLARE_JS_CLASS_BASE_PART()  \
  DECLARE_JS_CLASS_CONST_PART() \
  DECLARE_JS_CLASS_PART()

#define IMPLEMENT_JS_CLASS(js_class_name, class_alternate, class_name)       \
  IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)                    \
  IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)                   \
  IMPLEMENT_JS_CLASS_PART(js_class_name, class_alternate, class_name)        \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine,                 \
                                      FXJSOBJTYPE eObjType) {                \
    g_nObjDefnID = pEngine->DefineObj(js_class_name::g_pClassName, eObjType, \
                                      JSConstructor, JSDestructor);          \
    DefineConsts(pEngine);                                                   \
    DefineProps(pEngine);                                                    \
    DefineMethods(pEngine);                                                  \
  }

#define DECLARE_JS_CLASS_PART()                                                \
  static void JSConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj); \
  static void JSDestructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj);  \
  static void DefineProps(CFXJS_Engine* pEngine);                              \
  static void DefineMethods(CFXJS_Engine* pEngine);                            \
  static JSPropertySpec PropertySpecs[];                                       \
  static JSMethodSpec MethodSpecs[];

#define IMPLEMENT_JS_CLASS_PART(js_class_name, class_alternate, class_name) \
  void js_class_name::JSConstructor(CFXJS_Engine* pEngine,                  \
                                    v8::Local<v8::Object> obj) {            \
    CJS_Object* pObj = new js_class_name(obj);                              \
    pObj->SetEmbedObject(new class_alternate(pObj));                        \
    pEngine->SetObjectPrivate(obj, pObj);                                   \
    pObj->InitInstance(static_cast<CJS_Runtime*>(pEngine));                 \
  }                                                                         \
  void js_class_name::JSDestructor(CFXJS_Engine* pEngine,                   \
                                   v8::Local<v8::Object> obj) {             \
    delete static_cast<js_class_name*>(pEngine->GetObjectPrivate(obj));     \
  }                                                                         \
  void js_class_name::DefineProps(CFXJS_Engine* pEngine) {                  \
    for (size_t i = 0; i < FX_ArraySize(PropertySpecs) - 1; ++i) {          \
      pEngine->DefineObjProperty(g_nObjDefnID, PropertySpecs[i].pName,      \
                                 PropertySpecs[i].pPropGet,                 \
                                 PropertySpecs[i].pPropPut);                \
    }                                                                       \
  }                                                                         \
  void js_class_name::DefineMethods(CFXJS_Engine* pEngine) {                \
    for (size_t i = 0; i < FX_ArraySize(MethodSpecs) - 1; ++i) {            \
      pEngine->DefineObjMethod(g_nObjDefnID, MethodSpecs[i].pName,          \
                               MethodSpecs[i].pMethodCall);                 \
    }                                                                       \
  }

#endif  // FPDFSDK_JAVASCRIPT_JS_DEFINE_H_
