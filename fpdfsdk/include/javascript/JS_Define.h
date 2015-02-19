// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JS_DEFINE_H_
#define _JS_DEFINE_H_

typedef v8::Value			JSValue;
typedef v8::Handle<v8::Object>	JSObject;
typedef v8::Handle<v8::Object>	JSFXObject;
typedef unsigned		JSBool;

#include "JS_Object.h"
#include "JS_Value.h"

struct JSConstSpec
{
	const wchar_t* pName;
	double number;
	const wchar_t* string;
	FX_BYTE t; //0:double 1:str
};

struct JSPropertySpec
{
	const wchar_t* pName;
	v8::AccessorGetterCallback pPropGet;
	v8::AccessorSetterCallback pPropPut;
};

struct JSMethodSpec
{
	const wchar_t* pName;
	v8::FunctionCallback pMethodCall;
	unsigned nParamNum;
};

typedef CFX_WideString	JS_ErrorString;

#define JS_TRUE			(unsigned)1
#define JS_FALSE		(unsigned)0


#define CJS_PointsArray		CFX_ArrayTemplate<float>
#define CJS_IntArray		CFX_ArrayTemplate<int>

/* ====================================== PUBLIC DEFINE SPEC ============================================== */
#define JS_WIDESTRING(widestring) L###widestring

#define BEGIN_JS_STATIC_CONST(js_class_name) JSConstSpec js_class_name::JS_Class_Consts[] = {
#define JS_STATIC_CONST_ENTRY_NUMBER(const_name, pValue) {JS_WIDESTRING(const_name), pValue, L"", 0},
#define JS_STATIC_CONST_ENTRY_STRING(const_name, pValue) {JS_WIDESTRING(const_name), 0, JS_WIDESTRING(pValue), 1},
#define END_JS_STATIC_CONST() {0, 0, 0, 0}};

#define BEGIN_JS_STATIC_PROP(js_class_name) JSPropertySpec js_class_name::JS_Class_Properties[] = {
#define JS_STATIC_PROP_ENTRY(prop_name) {JS_WIDESTRING(prop_name), get_##prop_name##_static, set_##prop_name##_static},
#define END_JS_STATIC_PROP() {0, 0, 0}};

#define BEGIN_JS_STATIC_METHOD(js_class_name) JSMethodSpec js_class_name::JS_Class_Methods[] = {
#define JS_STATIC_METHOD_ENTRY(method_name, nargs) {JS_WIDESTRING(method_name), method_name##_static, nargs},
#define END_JS_STATIC_METHOD() {0, 0, 0}};

/* ======================================== PROP CALLBACK ============================================ */

template <class C, FX_BOOL (C::*M)(IFXJS_Context*, CJS_PropValue&, JS_ErrorString&)>
void JSPropGetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  const v8::PropertyCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_PropValue value(isolate);
  value.StartGetting();
  CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  JS_ErrorString sError;
  if (!(pObj->*M)(pRuntimeContext, value, sError)) {
    CFX_ByteString cbName;
    cbName.Format("%s.%s", class_name_string, prop_name_string);
    JS_Error(NULL, CFX_WideString::FromLocal(cbName), sError);
    return;
  }
  info.GetReturnValue().Set((v8::Handle<v8::Value>)value);
}

template <class C, FX_BOOL (C::*M)(IFXJS_Context*, CJS_PropValue&, JS_ErrorString&)>
void JSPropSetter(const char* prop_name_string,
                  const char* class_name_string,
                  v8::Local<v8::String> property,
                  v8::Local<v8::Value> value,
                  const v8::PropertyCallbackInfo<void>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_PropValue propValue(CJS_Value(isolate, value, VT_unknown));
  propValue.StartSetting();
  CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  JS_ErrorString sError;
  if (!(pObj->*M)(pRuntimeContext, propValue, sError)) {
    CFX_ByteString cbName;
    cbName.Format("%s.%s", class_name_string, prop_name_string);
    JS_Error(NULL, CFX_WideString::FromLocal(cbName), sError);
  }
}

#define JS_STATIC_PROP(prop_name, class_name) \
  static void get_##prop_name##_static( \
      v8::Local<v8::String> property, \
      const v8::PropertyCallbackInfo<v8::Value>& info) { \
    JSPropGetter<class_name, &class_name::prop_name>( \
        #prop_name, #class_name, property, info); \
  } \
  static void set_##prop_name##_static( \
      v8::Local<v8::String> property, \
      v8::Local<v8::Value> value, \
      const v8::PropertyCallbackInfo<void>& info) { \
    JSPropSetter<class_name, &class_name::prop_name>( \
        #prop_name, #class_name, property, value, info); \
  }

/* ========================================= METHOD CALLBACK =========================================== */

template <class C, FX_BOOL (C::*M)(IFXJS_Context*, const CJS_Parameters&, CJS_Value&, JS_ErrorString&)>
void JSMethod(const char* method_name_string,
              const char* class_name_string,
              const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_Parameters parameters;
  for (unsigned int i = 0; i<(unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(isolate, info[i], VT_unknown));
  }
  CJS_Value valueRes(isolate);
  CJS_Object* pJSObj = (CJS_Object *)JS_GetPrivate(isolate,info.Holder());
  C* pObj = reinterpret_cast<C*>(pJSObj->GetEmbedObject());
  JS_ErrorString sError;
  if (!(pObj->*M)(pRuntimeContext, parameters, valueRes, sError)) {
    CFX_ByteString cbName;
    cbName.Format("%s.%s", class_name_string, method_name_string);
    JS_Error(NULL, CFX_WideString::FromLocal(cbName), sError);
    return;
  }
  info.GetReturnValue().Set(valueRes.ToJSValue());
}

#define JS_STATIC_METHOD(method_name, class_name) \
  static void method_name##_static( \
      const v8::FunctionCallbackInfo<v8::Value>& info) {    \
    JSMethod<class_name, &class_name::method_name>( \
        #class_name, #method_name, info); \
  }

#define JS_SPECIAL_STATIC_METHOD(method_name, class_alternate, class_name) \
  static void method_name##_static( \
      const v8::FunctionCallbackInfo<v8::Value>& info) {    \
    JSMethod<class_alternate, &class_alternate::method_name>( \
        #class_name, #method_name, info); \
  }

/* ===================================== JS CLASS =============================================== */

#define DECLARE_JS_CLASS(js_class_name) \
	static JSBool JSConstructor(IFXJS_Context* cc, JSFXObject obj,JSFXObject global);\
	static JSBool JSDestructor(JSFXObject obj);\
	static int Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType);\
	static void GetConsts(JSConstSpec*& pConsts, int& nSize);\
	static void GetProperties(JSPropertySpec*& pProperties, int& nSize);\
	static void GetMethods(JSMethodSpec*& pMethods, int& nSize);\
	static JSConstSpec JS_Class_Consts[];\
	static JSPropertySpec JS_Class_Properties[];\
	static JSMethodSpec	JS_Class_Methods[];\
	static const wchar_t* m_pClassName

#define IMPLEMENT_JS_CLASS_RICH(js_class_name, class_alternate, class_name) \
const wchar_t* js_class_name::m_pClassName = JS_WIDESTRING(class_name);\
JSBool js_class_name::JSConstructor(IFXJS_Context* cc, JSFXObject obj, JSFXObject global)\
{\
	CJS_Object* pObj = FX_NEW js_class_name(obj);\
	pObj->SetEmbedObject(FX_NEW class_alternate(pObj));\
	JS_SetPrivate(NULL,obj,(void*)pObj); \
	pObj->InitInstance(cc);\
	return JS_TRUE;\
}\
\
JSBool js_class_name::JSDestructor(JSFXObject obj) \
{\
	js_class_name* pObj = (js_class_name*)JS_GetPrivate(NULL,obj);\
	ASSERT(pObj != NULL);\
	pObj->ExitInstance();\
	delete pObj;\
	return JS_TRUE;\
}\
\
int js_class_name::Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType)\
{\
	int nObjDefnID = JS_DefineObj(pRuntime, js_class_name::m_pClassName, eObjType, JSConstructor, JSDestructor, 0);\
	if (nObjDefnID >= 0)\
	{\
		for (int j=0, szj=sizeof(JS_Class_Properties)/sizeof(JSPropertySpec)-1; j<szj; j++)\
		{\
			if (JS_DefineObjProperty(pRuntime, nObjDefnID, JS_Class_Properties[j].pName, JS_Class_Properties[j].pPropGet, JS_Class_Properties[j].pPropPut) < 0) return -1;\
		}\
		for (int k=0, szk=sizeof(JS_Class_Methods)/sizeof(JSMethodSpec)-1; k<szk; k++)\
		{\
			if (JS_DefineObjMethod(pRuntime, nObjDefnID,JS_Class_Methods[k].pName, JS_Class_Methods[k].pMethodCall, JS_Class_Methods[k].nParamNum) < 0) return -1;\
		}\
		return nObjDefnID;\
	}\
	return -1;\
}\
void js_class_name::GetConsts(JSConstSpec*& pConsts, int& nSize)\
{\
	pConsts = JS_Class_Consts;\
	nSize = sizeof(JS_Class_Consts) / sizeof(JSConstSpec) - 1;\
}\
void js_class_name::GetProperties(JSPropertySpec*& pProperties, int& nSize)\
{\
	pProperties = JS_Class_Properties;\
	nSize = sizeof(JS_Class_Properties) / sizeof(JSPropertySpec) - 1;\
}\
void js_class_name::GetMethods(JSMethodSpec*& pMethods, int& nSize)\
{\
	pMethods = JS_Class_Methods;\
	nSize = sizeof(JS_Class_Methods) / sizeof(JSMethodSpec) - 1;\
}

#define IMPLEMENT_JS_CLASS(js_class_name, class_name) IMPLEMENT_JS_CLASS_RICH(js_class_name, class_name, class_name)

/* ======================================== CONST CLASS ============================================ */

#define DECLARE_JS_CLASS_CONST() \
	static int Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType);\
	static void GetConsts(JSConstSpec*& pConsts, int& nSize);\
	static JSConstSpec JS_Class_Consts[];\
	static const wchar_t* m_pClassName

#define IMPLEMENT_JS_CLASS_CONST(js_class_name, class_name) \
const wchar_t* js_class_name::m_pClassName = JS_WIDESTRING(class_name);\
int js_class_name::Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType)\
{\
	int nObjDefnID = JS_DefineObj(pRuntime, js_class_name::m_pClassName, eObjType, NULL, NULL, 0);\
	if (nObjDefnID >=0)\
	{\
		for (int i=0, sz=sizeof(JS_Class_Consts)/sizeof(JSConstSpec)-1; i<sz; i++)\
		{\
			if (JS_Class_Consts[i].t == 0)\
			{\
				if (JS_DefineObjConst(pRuntime, nObjDefnID, JS_Class_Consts[i].pName, JS_NewNumber(pRuntime,JS_Class_Consts[i].number)) < 0) return -1;\
			}\
			else\
			{\
			if (JS_DefineObjConst(pRuntime, nObjDefnID, JS_Class_Consts[i].pName, JS_NewString(pRuntime,JS_Class_Consts[i].string)) < 0) return -1;\
			}\
		}\
		return nObjDefnID;\
	}\
	return -1;\
}\
void js_class_name::GetConsts(JSConstSpec*& pConsts, int& nSize)\
{\
	pConsts = JS_Class_Consts;\
	nSize = sizeof(JS_Class_Consts)/sizeof(JSConstSpec)-1;\
}

/* ===================================== SPECIAL JS CLASS =============================================== */

#define DECLARE_SPECIAL_JS_CLASS(js_class_name) \
	static JSBool JSConstructor(IFXJS_Context* cc, JSFXObject obj, JSFXObject global);\
	static JSBool JSDestructor(JSFXObject obj);\
	static void GetConsts(JSConstSpec*& pConsts, int& nSize);\
	static void GetProperties(JSPropertySpec*& pProperties, int& nSize);\
	static void GetMethods(JSMethodSpec*& pMethods, int& nSize);\
	static JSConstSpec JS_Class_Consts[];\
	static JSPropertySpec JS_Class_Properties[];\
	static JSMethodSpec	JS_Class_Methods[];\
	static int Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType);\
	static const wchar_t* m_pClassName;\
	static void queryprop_##js_class_name##_static(JS_PROPQUERY_ARGS);\
	static void getprop_##js_class_name##_static(JS_NAMED_PROPGET_ARGS);\
	static void putprop_##js_class_name##_static(JS_NAMED_PROPPUT_ARGS);\
	static void delprop_##js_class_name##_static(JS_PROPDEL_ARGS)

#define IMPLEMENT_SPECIAL_JS_CLASS(js_class_name, class_alternate, class_name) \
const wchar_t * js_class_name::m_pClassName = JS_WIDESTRING(class_name);\
	void js_class_name::queryprop_##js_class_name##_static(JS_PROPQUERY_ARGS)\
{\
	v8::Isolate* isolate = info.GetIsolate();\
	v8::String::Utf8Value utf8_value(property);\
	CFX_WideString propname = CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());\
	CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());\
	ASSERT(pJSObj != NULL);\
	class_alternate* pObj = (class_alternate*)pJSObj->GetEmbedObject();\
	ASSERT(pObj != NULL);\
	FX_BOOL bRet = FALSE;\
	bRet = pObj->QueryProperty(propname.c_str());\
	if (bRet)\
	{\
		info.GetReturnValue().Set(0x004);\
		return ;\
	}\
	else\
	{\
		info.GetReturnValue().Set(0);\
		return ;\
	}\
	return ;\
}\
	void js_class_name::getprop_##js_class_name##_static(JS_NAMED_PROPGET_ARGS)\
{\
	v8::Isolate* isolate = info.GetIsolate();\
	v8::Local<v8::Context> context = isolate->GetCurrentContext();\
	IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);\
	if (pRuntime == NULL) return;\
	IFXJS_Context* cc = pRuntime->GetCurrentContext();\
	v8::String::Utf8Value utf8_value(property);\
	CFX_WideString propname = CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());\
	CJS_PropValue value(isolate);\
	value.StartGetting();\
	CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());\
	ASSERT(pJSObj != NULL);\
	class_alternate* pObj = (class_alternate*)pJSObj->GetEmbedObject();\
	ASSERT(pObj != NULL);\
	JS_ErrorString sError;\
	FX_BOOL bRet = FALSE;\
	bRet = pObj->DoProperty(cc, propname.c_str(), value, sError);\
	if (bRet)\
	{\
		info.GetReturnValue().Set((v8::Handle<v8::Value>)value);\
		return ;\
	}\
	else\
	{\
		CFX_ByteString cbName;\
		cbName.Format("%s.%s", #class_name, L"GetProperty");\
		JS_Error(NULL,CFX_WideString::FromLocal(cbName), sError);\
		return ;\
	}\
	JS_Error(NULL,L"GetProperty", L"Embeded object not found!");\
	return ;\
}\
	void js_class_name::putprop_##js_class_name##_static(JS_NAMED_PROPPUT_ARGS)\
{\
	v8::Isolate* isolate = info.GetIsolate();\
	v8::Local<v8::Context> context = isolate->GetCurrentContext();\
	IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);\
	if (pRuntime == NULL) return;\
	IFXJS_Context* cc = pRuntime->GetCurrentContext();\
	v8::String::Utf8Value utf8_value(property);\
	CFX_WideString propname = CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());\
	CJS_PropValue PropValue(CJS_Value(isolate,value,VT_unknown));\
	PropValue.StartSetting();\
	CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());\
	if(!pJSObj) return;\
	class_alternate* pObj = (class_alternate*)pJSObj->GetEmbedObject();\
	ASSERT(pObj != NULL);\
	JS_ErrorString sError;\
	FX_BOOL bRet = FALSE;\
	bRet = pObj->DoProperty(cc, propname.c_str(), PropValue, sError);\
	if (bRet)\
	{\
		return ;\
	}\
	else\
	{\
		CFX_ByteString cbName;\
		cbName.Format("%s.%s", #class_name, "PutProperty");\
		JS_Error(NULL,CFX_WideString::FromLocal(cbName), sError);\
		return ;\
	}\
	JS_Error(NULL,L"PutProperty", L"Embeded object not found!");\
	return ;\
}\
	void js_class_name::delprop_##js_class_name##_static(JS_PROPDEL_ARGS)\
{\
	v8::Isolate* isolate = info.GetIsolate();\
	v8::Local<v8::Context> context = isolate->GetCurrentContext();\
	IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);\
	if (pRuntime == NULL) return;\
	IFXJS_Context* cc = pRuntime->GetCurrentContext();\
	v8::String::Utf8Value utf8_value(property);\
	CFX_WideString propname = CFX_WideString::FromUTF8(*utf8_value, utf8_value.length());\
	CJS_Object* pJSObj = (CJS_Object*)JS_GetPrivate(isolate,info.Holder());\
	ASSERT(pJSObj != NULL);\
	class_alternate* pObj = (class_alternate*)pJSObj->GetEmbedObject();\
	ASSERT(pObj != NULL);\
	JS_ErrorString sError;\
	FX_BOOL bRet = FALSE;\
	bRet = pObj->DelProperty(cc, propname.c_str(), sError);\
	if (bRet)\
	{\
		return ;\
	}\
	else\
	{\
		CFX_ByteString cbName;\
		cbName.Format("%s.%s", #class_name, "DelProperty");\
		return ;\
	}\
	return ;\
}\
JSBool js_class_name::JSConstructor(IFXJS_Context* cc, JSFXObject  obj,JSFXObject  global)\
{\
	CJS_Object* pObj = FX_NEW js_class_name(obj);\
	pObj->SetEmbedObject(FX_NEW class_alternate(pObj));\
	JS_SetPrivate(NULL,obj, (void*)pObj); \
	pObj->InitInstance(cc);\
	return JS_TRUE;\
}\
\
JSBool js_class_name::JSDestructor(JSFXObject obj) \
{\
	js_class_name* pObj = (js_class_name*)JS_GetPrivate(NULL,obj);\
	ASSERT(pObj != NULL);\
	pObj->ExitInstance();\
	delete pObj;\
	return JS_TRUE;\
}\
\
int js_class_name::Init(IJS_Runtime* pRuntime, FXJSOBJTYPE eObjType)\
{\
\
	int nObjDefnID = JS_DefineObj(pRuntime, js_class_name::m_pClassName, eObjType, JSConstructor, JSDestructor, 0);\
\
	if (nObjDefnID >= 0)\
	{\
		for (int j=0, szj=sizeof(JS_Class_Properties)/sizeof(JSPropertySpec)-1; j<szj; j++)\
		{\
			if (JS_DefineObjProperty(pRuntime, nObjDefnID, JS_Class_Properties[j].pName, JS_Class_Properties[j].pPropGet,JS_Class_Properties[j].pPropPut)<0)return -1;\
		}\
\
		for (int k=0, szk=sizeof(JS_Class_Methods)/sizeof(JSMethodSpec)-1; k<szk; k++)\
		{\
			if (JS_DefineObjMethod(pRuntime, nObjDefnID,JS_Class_Methods[k].pName,JS_Class_Methods[k].pMethodCall,JS_Class_Methods[k].nParamNum)<0)return -1;\
		}\
		if (JS_DefineObjAllProperties(pRuntime, nObjDefnID, js_class_name::queryprop_##js_class_name##_static, js_class_name::getprop_##js_class_name##_static,js_class_name::putprop_##js_class_name##_static,js_class_name::delprop_##js_class_name##_static)<0) return -1;\
\
		return nObjDefnID;\
	}\
\
	return -1;\
}\
void js_class_name::GetConsts(JSConstSpec*& pConsts, int& nSize)\
{\
	pConsts = JS_Class_Consts;\
	nSize = sizeof(JS_Class_Consts)/sizeof(JSConstSpec)-1;\
}\
void js_class_name::GetProperties(JSPropertySpec*& pProperties, int& nSize)\
{\
	pProperties = JS_Class_Properties;\
	nSize = sizeof(JS_Class_Properties)/sizeof(JSPropertySpec)-1;\
}\
void js_class_name::GetMethods(JSMethodSpec*& pMethods, int& nSize)\
{\
	pMethods = JS_Class_Methods;\
	nSize = sizeof(JS_Class_Methods)/sizeof(JSMethodSpec)-1;\
}

/* ======================================== GLOBAL METHODS ============================================ */

template <FX_BOOL (*F)(IFXJS_Context*, const CJS_Parameters&, CJS_Value&, JS_ErrorString&)>
void JSGlobalFunc(const char *func_name_string,
                  const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  IFXJS_Runtime* pRuntime = (IFXJS_Runtime*)isolate->GetData(2);
  IFXJS_Context* pRuntimeContext = pRuntime->GetCurrentContext();
  CJS_Parameters parameters;
  for (unsigned int i = 0; i<(unsigned int)info.Length(); i++) {
    parameters.push_back(CJS_Value(isolate, info[i], VT_unknown));
  }
  CJS_Value valueRes(isolate);
  JS_ErrorString sError;
  if (!(*F)(pRuntimeContext, parameters, valueRes, sError))
  {
    JS_Error(NULL, JS_WIDESTRING(fun_name), sError);
    return;
  }
  info.GetReturnValue().Set(valueRes.ToJSValue());
}

#define JS_STATIC_GLOBAL_FUN(fun_name) \
  static void fun_name##_static(const v8::FunctionCallbackInfo<v8::Value>& info) { \
    JSGlobalFunc<fun_name>(#fun_name, info); \
  }

#define JS_STATIC_DECLARE_GLOBAL_FUN() \
static JSMethodSpec	global_methods[]; \
static int Init(IJS_Runtime* pRuntime)

#define BEGIN_JS_STATIC_GLOBAL_FUN(js_class_name) \
JSMethodSpec js_class_name::global_methods[] = {

#define JS_STATIC_GLOBAL_FUN_ENTRY(method_name,nargs) JS_STATIC_METHOD_ENTRY(method_name,nargs)

#define END_JS_STATIC_GLOBAL_FUN() END_JS_STATIC_METHOD()

#define IMPLEMENT_JS_STATIC_GLOBAL_FUN(js_class_name) \
int js_class_name::Init(IJS_Runtime* pRuntime)\
{\
	for (int i=0, sz=sizeof(js_class_name::global_methods)/sizeof(JSMethodSpec)-1; i<sz; i++)\
	{\
		if (JS_DefineGlobalMethod(pRuntime,\
				js_class_name::global_methods[i].pName,\
				js_class_name::global_methods[i].pMethodCall,\
				js_class_name::global_methods[i].nParamNum\
				) < 0\
			)return -1;\
	}\
	return 0;\
}

/* ======================================== GLOBAL CONSTS ============================================ */
#define DEFINE_GLOBAL_CONST(pRuntime, const_name , const_value)\
if (JS_DefineGlobalConst(pRuntime,JS_WIDESTRING(const_name),JS_NewString(pRuntime,JS_WIDESTRING(const_value)))) return -1

/* ======================================== GLOBAL ARRAYS ============================================ */

#define DEFINE_GLOBAL_ARRAY(pRuntime)\
int size = FX_ArraySize(ArrayContent);\
\
CJS_Array array(pRuntime);\
for (int i=0; i<size; i++) array.SetElement(i,CJS_Value(pRuntime,ArrayContent[i]));\
\
CJS_PropValue prop(pRuntime);\
prop << array;\
if (JS_DefineGlobalConst(pRuntime, (const wchar_t*)ArrayName, prop.ToJSValue()) < 0)\
	return -1

/* ============================================================ */

#define VALUE_NAME_STRING		L"string"
#define VALUE_NAME_NUMBER		L"number"
#define VALUE_NAME_BOOLEAN		L"boolean"
#define VALUE_NAME_DATE			L"date"
#define VALUE_NAME_OBJECT		L"object"
#define VALUE_NAME_FXOBJ		L"fxobj"
#define VALUE_NAME_NULL			L"null"
#define VALUE_NAME_UNDEFINED	L"undefined"

FXJSVALUETYPE GET_VALUE_TYPE(v8::Handle<v8::Value> p);

#endif //_JS_DEFINE_H_
