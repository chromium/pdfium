// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_DEFINE_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_DEFINE_H_

#include "../jsapi/fxjs_v8.h"
#include "resource.h"
#include "JS_Object.h"
#include "JS_Value.h"

struct JSConstSpec {
  const wchar_t* pName;
  double number;
  const wchar_t* string;
  uint8_t t;  // 0:double 1:str
};

struct JSPropertySpec {
  const wchar_t* pName;
  v8::AccessorGetterCallback pPropGet;
  v8::AccessorSetterCallback pPropPut;
};

struct JSMethodSpec {
  const wchar_t* pName;
  v8::FunctionCallback pMethodCall;
};

/* ====================================== PUBLIC DEFINE SPEC
 * ============================================== */
#define JS_WIDESTRING(widestring) L## #widestring

#define BEGIN_JS_STATIC_CONST(js_class_name) \
  JSConstSpec js_class_name::JS_Class_Consts[] = {
#define JS_STATIC_CONST_ENTRY_NUMBER(const_name, pValue) \
  { const_name, pValue, L"", 0 }                         \
  ,
#define JS_STATIC_CONST_ENTRY_STRING(const_name, pValue) \
  { const_name, 0, pValue, 1 }                           \
  ,
#define END_JS_STATIC_CONST() \
  { 0, 0, 0, 0 }              \
  }                           \
  ;

#define BEGIN_JS_STATIC_PROP(js_class_name) \
  JSPropertySpec js_class_name::JS_Class_Properties[] = {
#define JS_STATIC_PROP_ENTRY(prop_name)                                          \
  {                                                                              \
    JS_WIDESTRING(prop_name), get_##prop_name##_static, set_##prop_name##_static \
  }                                                                              \
  ,
#define END_JS_STATIC_PROP() \
  { 0, 0, 0 }                \
  }                          \
  ;

#define BEGIN_JS_STATIC_METHOD(js_class_name) \
  JSMethodSpec js_class_name::JS_Class_Methods[] = {
#define JS_STATIC_METHOD_ENTRY(method_name)            \
  { JS_WIDESTRING(method_name), method_name##_static } \
  ,
#define END_JS_STATIC_METHOD() \
  { 0, 0 }                     \
  }                            \
  ;

/* ======================================== PROP CALLBACK
 * ============================================ */

template <class C,
          FX_BOOL (C::*M)(IFXJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError)>
void JSPropGetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* pContext = pRuntime->GetCurrentContext();
  CJS_Object* pJSObj = (CJS_Object*)FXJS_GetPrivate(isolate, info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  CJS_PropValue value(isolate);
  value.StartGetting();
  if (!(pObj->*M)(pContext, value, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(class_name_string, prop_name_string,
                                            sError));
    return;
  }
  info.GetReturnValue().Set((v8::Local<v8::Value>)value);
}

template <class C,
          FX_BOOL (C::*M)(IFXJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError)>
void JSPropSetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* pContext = pRuntime->GetCurrentContext();
  CJS_Object* pJSObj = (CJS_Object*)FXJS_GetPrivate(isolate, info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  CJS_PropValue propValue(CJS_Value(isolate, value, CJS_Value::VT_unknown));
  propValue.StartSetting();
  if (!(pObj->*M)(pContext, propValue, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(class_name_string, prop_name_string,
                                            sError));
  }
}

#define JS_STATIC_PROP(prop_name, class_name)                                 \
  static void get_##prop_name##_static(                                       \
      v8::Local<v8::String> property,                                         \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                      \
    JSPropGetter<class_name, &class_name::prop_name>(#prop_name, #class_name, \
                                                     property, info);         \
  }                                                                           \
  static void set_##prop_name##_static(                                       \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,             \
      const v8::PropertyCallbackInfo<void>& info) {                           \
    JSPropSetter<class_name, &class_name::prop_name>(#prop_name, #class_name, \
                                                     property, value, info);  \
  }

/* ========================================= METHOD CALLBACK
 * =========================================== */

template <class C,
          FX_BOOL (C::*M)(IFXJS_Context* cc,
                          const CJS_Parameters& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* cc = pRuntime->GetCurrentContext();
  CJS_Parameters parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(isolate, info[i], CJS_Value::VT_unknown));
  }
  CJS_Value valueRes(isolate);
  CJS_Object* pJSObj = (CJS_Object*)FXJS_GetPrivate(isolate, info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  CFX_WideString sError;
  if (!(pObj->*M)(cc, parameters, valueRes, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(class_name_string,
                                            method_name_string, sError));
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

#define JS_SPECIAL_STATIC_METHOD(method_name, class_alternate, class_name) \
  static void method_name##_static(                                        \
      const v8::FunctionCallbackInfo<v8::Value>& info) {                   \
    JSMethod<class_alternate, &class_alternate::method_name>(              \
        #method_name, #class_name, info);                                  \
  }

/* ===================================== JS CLASS
 * =============================================== */

#define DECLARE_JS_CLASS(js_class_name)                                     \
  static void JSConstructor(IFXJS_Context* cc, v8::Local<v8::Object> obj,   \
                            v8::Local<v8::Object> global);                  \
  static void JSDestructor(v8::Local<v8::Object> obj);                      \
  static void DefineJSObjects(v8::Isolate* pIsolate, FXJSOBJTYPE eObjType); \
  static JSConstSpec JS_Class_Consts[];                                     \
  static JSPropertySpec JS_Class_Properties[];                              \
  static JSMethodSpec JS_Class_Methods[];                                   \
  static const wchar_t* m_pClassName

#define IMPLEMENT_JS_CLASS_RICH(js_class_name, class_alternate, class_name)  \
  const wchar_t* js_class_name::m_pClassName = JS_WIDESTRING(class_name);    \
  void js_class_name::JSConstructor(IFXJS_Context* cc,                       \
                                    v8::Local<v8::Object> obj,               \
                                    v8::Local<v8::Object> global) {          \
    CJS_Object* pObj = new js_class_name(obj);                               \
    pObj->SetEmbedObject(new class_alternate(pObj));                         \
    FXJS_SetPrivate(NULL, obj, (void*)pObj);                                 \
    pObj->InitInstance(cc);                                                  \
  }                                                                          \
                                                                             \
  void js_class_name::JSDestructor(v8::Local<v8::Object> obj) {              \
    js_class_name* pObj = (js_class_name*)FXJS_GetPrivate(NULL, obj);        \
    pObj->ExitInstance();                                                    \
    delete pObj;                                                             \
  }                                                                          \
                                                                             \
  void js_class_name::DefineJSObjects(v8::Isolate* pIsolate,                 \
                                      FXJSOBJTYPE eObjType) {                \
    int nObjDefnID = FXJS_DefineObj(pIsolate, js_class_name::m_pClassName,   \
                                    eObjType, JSConstructor, JSDestructor);  \
    for (int i = 0; i < FX_ArraySize(JS_Class_Properties) - 1; ++i) {        \
      FXJS_DefineObjProperty(                                                \
          pIsolate, nObjDefnID, JS_Class_Properties[i].pName,                \
          JS_Class_Properties[i].pPropGet, JS_Class_Properties[i].pPropPut); \
    }                                                                        \
    for (int i = 0; i < FX_ArraySize(JS_Class_Methods) - 1; ++i) {           \
      FXJS_DefineObjMethod(pIsolate, nObjDefnID, JS_Class_Methods[i].pName,  \
                           JS_Class_Methods[i].pMethodCall);                 \
    }                                                                        \
  }

#define IMPLEMENT_JS_CLASS(js_class_name, class_name) \
  IMPLEMENT_JS_CLASS_RICH(js_class_name, class_name, class_name)

/* ======================================== CONST CLASS
 * ============================================ */

#define DECLARE_JS_CLASS_CONST()                                            \
  static void DefineJSObjects(v8::Isolate* pIsolate, FXJSOBJTYPE eObjType); \
  static JSConstSpec JS_Class_Consts[];                                     \
  static const wchar_t* m_pClassName

#define IMPLEMENT_JS_CLASS_CONST(js_class_name, class_name)                \
  const wchar_t* js_class_name::m_pClassName = JS_WIDESTRING(class_name);  \
  void js_class_name::DefineJSObjects(v8::Isolate* pIsolate,               \
                                      FXJSOBJTYPE eObjType) {              \
    int nObjDefnID = FXJS_DefineObj(pIsolate, js_class_name::m_pClassName, \
                                    eObjType, NULL, NULL);                 \
    for (int i = 0; i < FX_ArraySize(JS_Class_Consts) - 1; ++i) {          \
      FXJS_DefineObjConst(                                                 \
          pIsolate, nObjDefnID, JS_Class_Consts[i].pName,                  \
          JS_Class_Consts[i].t == 0                                        \
              ? FXJS_NewNumber(pIsolate, JS_Class_Consts[i].number)        \
              : FXJS_NewString(pIsolate, JS_Class_Consts[i].string));      \
    }                                                                      \
  }

/* ===================================== SPECIAL JS CLASS
 * =============================================== */

template <class Alt>
void JSSpecialPropQuery(const char*,
                        v8::Local<v8::String> property,
                        const v8::PropertyCallbackInfo<v8::Integer>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname =
      CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());
  CJS_Object* pJSObj =
      reinterpret_cast<CJS_Object*>(FXJS_GetPrivate(isolate, info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  FX_BOOL bRet = pObj->QueryProperty(propname.c_str());
  info.GetReturnValue().Set(bRet ? 4 : 0);
}

template <class Alt>
void JSSpecialPropGet(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_Object* pJSObj =
      reinterpret_cast<CJS_Object*>(FXJS_GetPrivate(isolate, info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname =
      CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());
  CFX_WideString sError;
  CJS_PropValue value(isolate);
  value.StartGetting();
  if (!pObj->DoProperty(pRuntimeContext, propname.c_str(), value, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(class_name, "GetProperty", sError));
    return;
  }
  info.GetReturnValue().Set((v8::Local<v8::Value>)value);
}

template <class Alt>
void JSSpecialPropPut(const char* class_name,
                      v8::Local<v8::String> property,
                      v8::Local<v8::Value> value,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_Object* pJSObj =
      reinterpret_cast<CJS_Object*>(FXJS_GetPrivate(isolate, info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname =
      CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());
  CFX_WideString sError;
  CJS_PropValue PropValue(CJS_Value(isolate, value, CJS_Value::VT_unknown));
  PropValue.StartSetting();
  if (!pObj->DoProperty(pRuntimeContext, propname.c_str(), PropValue, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(class_name, "PutProperty", sError));
  }
}

template <class Alt>
void JSSpecialPropDel(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_Object* pJSObj =
      reinterpret_cast<CJS_Object*>(FXJS_GetPrivate(isolate, info.Holder()));
  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  CFX_WideString propname =
      CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());
  CFX_WideString sError;
  if (!pObj->DelProperty(pRuntimeContext, propname.c_str(), sError)) {
    CFX_ByteString cbName;
    cbName.Format("%s.%s", class_name, "DelProperty");
    // Probably a missing call to JSFX_Error().
  }
}

#define DECLARE_SPECIAL_JS_CLASS(js_class_name)                             \
  static void JSConstructor(IFXJS_Context* cc, v8::Local<v8::Object> obj,   \
                            v8::Local<v8::Object> global);                  \
  static void JSDestructor(v8::Local<v8::Object> obj);                      \
  static JSConstSpec JS_Class_Consts[];                                     \
  static JSPropertySpec JS_Class_Properties[];                              \
  static JSMethodSpec JS_Class_Methods[];                                   \
  static void DefineJSObjects(v8::Isolate* pIsolate, FXJSOBJTYPE eObjType); \
  static const wchar_t* m_pClassName;                                       \
  static void queryprop_##js_class_name##_static(                           \
      v8::Local<v8::String> property,                                       \
      const v8::PropertyCallbackInfo<v8::Integer>& info);                   \
  static void getprop_##js_class_name##_static(                             \
      v8::Local<v8::String> property,                                       \
      const v8::PropertyCallbackInfo<v8::Value>& info);                     \
  static void putprop_##js_class_name##_static(                             \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,           \
      const v8::PropertyCallbackInfo<v8::Value>& info);                     \
  static void delprop_##js_class_name##_static(                             \
      v8::Local<v8::String> property,                                       \
      const v8::PropertyCallbackInfo<v8::Boolean>& info)

#define IMPLEMENT_SPECIAL_JS_CLASS(js_class_name, class_alternate, class_name) \
  const wchar_t* js_class_name::m_pClassName = JS_WIDESTRING(class_name);      \
  void js_class_name::queryprop_##js_class_name##_static(                      \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Integer>& info) {                     \
    JSSpecialPropQuery<class_alternate>(#class_name, property, info);          \
  }                                                                            \
  void js_class_name::getprop_##js_class_name##_static(                        \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                       \
    JSSpecialPropGet<class_alternate>(#class_name, property, info);            \
  }                                                                            \
  void js_class_name::putprop_##js_class_name##_static(                        \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,              \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                       \
    JSSpecialPropPut<class_alternate>(#class_name, property, value, info);     \
  }                                                                            \
  void js_class_name::delprop_##js_class_name##_static(                        \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Boolean>& info) {                     \
    JSSpecialPropDel<class_alternate>(#class_name, property, info);            \
  }                                                                            \
  void js_class_name::JSConstructor(IFXJS_Context* cc,                         \
                                    v8::Local<v8::Object> obj,                 \
                                    v8::Local<v8::Object> global) {            \
    CJS_Object* pObj = new js_class_name(obj);                                 \
    pObj->SetEmbedObject(new class_alternate(pObj));                           \
    FXJS_SetPrivate(NULL, obj, (void*)pObj);                                   \
    pObj->InitInstance(cc);                                                    \
  }                                                                            \
                                                                               \
  void js_class_name::JSDestructor(v8::Local<v8::Object> obj) {                \
    js_class_name* pObj = (js_class_name*)FXJS_GetPrivate(NULL, obj);          \
    ASSERT(pObj != NULL);                                                      \
    pObj->ExitInstance();                                                      \
    delete pObj;                                                               \
  }                                                                            \
                                                                               \
  void js_class_name::DefineJSObjects(v8::Isolate* pIsolate,                   \
                                      FXJSOBJTYPE eObjType) {                  \
    int nObjDefnID = FXJS_DefineObj(pIsolate, js_class_name::m_pClassName,     \
                                    eObjType, JSConstructor, JSDestructor);    \
    for (int i = 0; i < FX_ArraySize(JS_Class_Properties) - 1; ++i) {          \
      FXJS_DefineObjProperty(                                                  \
          pIsolate, nObjDefnID, JS_Class_Properties[i].pName,                  \
          JS_Class_Properties[i].pPropGet, JS_Class_Properties[i].pPropPut);   \
    }                                                                          \
                                                                               \
    for (int i = 0; i < FX_ArraySize(JS_Class_Methods) - 1; ++i) {             \
      FXJS_DefineObjMethod(pIsolate, nObjDefnID, JS_Class_Methods[i].pName,    \
                           JS_Class_Methods[i].pMethodCall);                   \
    }                                                                          \
    FXJS_DefineObjAllProperties(                                               \
        pIsolate, nObjDefnID,                                                  \
        js_class_name::queryprop_##js_class_name##_static,                     \
        js_class_name::getprop_##js_class_name##_static,                       \
        js_class_name::putprop_##js_class_name##_static,                       \
        js_class_name::delprop_##js_class_name##_static);                      \
  }

/* ======================================== GLOBAL METHODS
 * ============================================ */

template <FX_BOOL (*F)(IFXJS_Context* cc,
                       const CJS_Parameters& params,
                       CJS_Value& vRet,
                       CFX_WideString& sError)>
void JSGlobalFunc(const char* func_name_string,
                  const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> v = context->GetEmbedderData(1);
  if (v.IsEmpty())
    return;
  v8::Local<v8::External> field = v8::Local<v8::External>::Cast(v);
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)field->Value();
  IFXJS_Context* cc = pRuntime->GetCurrentContext();
  CJS_Parameters parameters;
  for (unsigned int i = 0; i < (unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(isolate, info[i], CJS_Value::VT_unknown));
  }
  CJS_Value valueRes(isolate);
  CFX_WideString sError;
  if (!(*F)(cc, parameters, valueRes, sError)) {
    FXJS_Error(isolate, JSFormatErrorString(func_name_string, nullptr, sError));
    return;
  }
  info.GetReturnValue().Set(valueRes.ToV8Value());
}

#define JS_STATIC_GLOBAL_FUN(fun_name)                   \
  static void fun_name##_static(                         \
      const v8::FunctionCallbackInfo<v8::Value>& info) { \
    JSGlobalFunc<fun_name>(#fun_name, info);             \
  }

#define JS_STATIC_DECLARE_GLOBAL_FUN()  \
  static JSMethodSpec global_methods[]; \
  static void DefineJSObjects(v8::Isolate* pIsolate)

#define BEGIN_JS_STATIC_GLOBAL_FUN(js_class_name) \
  JSMethodSpec js_class_name::global_methods[] = {
#define JS_STATIC_GLOBAL_FUN_ENTRY(method_name) \
  JS_STATIC_METHOD_ENTRY(method_name)

#define END_JS_STATIC_GLOBAL_FUN() END_JS_STATIC_METHOD()

#define IMPLEMENT_JS_STATIC_GLOBAL_FUN(js_class_name)                        \
  void js_class_name::DefineJSObjects(v8::Isolate* pIsolate) {               \
    for (int i = 0; i < FX_ArraySize(global_methods) - 1; ++i) {             \
      FXJS_DefineGlobalMethod(pIsolate,                                      \
                              js_class_name::global_methods[i].pName,        \
                              js_class_name::global_methods[i].pMethodCall); \
    }                                                                        \
  }

CJS_Value::Type GET_VALUE_TYPE(v8::Local<v8::Value> p);

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_DEFINE_H_
