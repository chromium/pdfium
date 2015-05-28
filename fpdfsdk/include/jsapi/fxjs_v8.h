// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

// PDFium wrapper around V8 APIs. PDFium code should include this file rather
// than including V8 headers directly.

#ifndef FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
#define FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_

#include <v8.h>
#include "../../../core/include/fxcrt/fx_string.h"  // For CFX_WideString

typedef v8::Value			JSValue;
typedef v8::Local<v8::Object>	JSObject;
typedef v8::Local<v8::Object>	JSFXObject;

enum FXJSOBJTYPE
{
	JS_DYNAMIC = 0,
	JS_STATIC = 1,
};

enum FXJSVALUETYPE
{
	VT_unknown,
	VT_string,
	VT_number,
	VT_boolean,
	VT_date,
	VT_object,
	VT_fxobject,
	VT_null,
	VT_undefined
};

struct FXJSErr
{
	const wchar_t* message;
    const wchar_t* srcline;
    unsigned linnum;
};

/* --------------------------------------------- API --------------------------------------------- */

typedef v8::Isolate IJS_Runtime;
class IFXJS_Context;
class IFXJS_Runtime;

typedef void (*LP_CONSTRUCTOR)(IFXJS_Context* cc, v8::Local<v8::Object> obj, v8::Local<v8::Object> global);
typedef void (*LP_DESTRUCTOR)(v8::Local<v8::Object> obj);


int								JS_DefineObj(IJS_Runtime* pJSRuntime, const wchar_t* sObjName, FXJSOBJTYPE eObjType, LP_CONSTRUCTOR pConstructor, LP_DESTRUCTOR pDestructor, unsigned bApplyNew);
int								JS_DefineObjMethod(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sMethodName, v8::FunctionCallback pMethodCall);
int								JS_DefineObjProperty(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sPropName, v8::AccessorGetterCallback pPropGet, v8::AccessorSetterCallback pPropPut);
int								JS_DefineObjAllProperties(IJS_Runtime* pJSRuntime, int nObjDefnID, v8::NamedPropertyQueryCallback pPropQurey, v8::NamedPropertyGetterCallback pPropGet, v8::NamedPropertySetterCallback pPropPut, v8::NamedPropertyDeleterCallback pPropDel);
int								JS_DefineObjConst(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sConstName, v8::Local<v8::Value> pDefault);
int								JS_DefineGlobalMethod(IJS_Runtime* pJSRuntime, const wchar_t* sMethodName, v8::FunctionCallback pMethodCall);
int								JS_DefineGlobalConst(IJS_Runtime* pJSRuntime, const wchar_t* sConstName, v8::Local<v8::Value> pDefault);

void							JS_InitialRuntime(IJS_Runtime* pJSRuntime,IFXJS_Runtime* pFXRuntime, IFXJS_Context* context, v8::Global<v8::Context>& v8PersistentContext);
void							JS_ReleaseRuntime(IJS_Runtime* pJSRuntime, v8::Global<v8::Context>& v8PersistentContext);
void							JS_Initial();
void							JS_Release();
int								JS_Parse(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, const wchar_t* script, long length, FXJSErr* perror);
int								JS_Execute(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, const wchar_t* script, long length, FXJSErr* perror);
v8::Local<v8::Object>			JS_NewFxDynamicObj(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, int nObjDefnID);
v8::Local<v8::Object>			JS_GetStaticObj(IJS_Runtime* pJSRuntime, int nObjDefnID);
void							JS_SetThisObj(IJS_Runtime* pJSRuntime, int nThisObjID);
v8::Local<v8::Object>			JS_GetThisObj(IJS_Runtime * pJSRuntime);
int								JS_GetObjDefnID(v8::Local<v8::Object> pObj);
IJS_Runtime*					JS_GetRuntime(v8::Local<v8::Object> pObj);
int								JS_GetObjDefnID(IJS_Runtime * pJSRuntime, const wchar_t* pObjName);
void							JS_Error(v8::Isolate* isolate, const CFX_WideString& message);
unsigned						JS_CalcHash(const wchar_t* main, unsigned nLen);
unsigned						JS_CalcHash(const wchar_t* main);
const wchar_t*					JS_GetTypeof(v8::Local<v8::Value> pObj);
void							JS_SetPrivate(IJS_Runtime* pJSRuntime, v8::Local<v8::Object> pObj, void* p);
void*							JS_GetPrivate(IJS_Runtime* pJSRuntime, v8::Local<v8::Object> pObj);
void							JS_SetPrivate(v8::Local<v8::Object> pObj, void* p);
void*							JS_GetPrivate(v8::Local<v8::Object> pObj);
void							JS_FreePrivate(void* p);
void							JS_FreePrivate(v8::Local<v8::Object> pObj);
v8::Local<v8::Value>			JS_GetObjectValue(v8::Local<v8::Object> pObj);
v8::Local<v8::Value>			JS_GetObjectElement(IJS_Runtime* pJSRuntime, v8::Local<v8::Object> pObj,const wchar_t* PropertyName);
v8::Local<v8::Array>			JS_GetObjectElementNames(IJS_Runtime* pJSRuntime, v8::Local<v8::Object> pObj);
void							JS_PutObjectString(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, const wchar_t* sValue);
void							JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, int nValue);
void							JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, float fValue);
void							JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, double dValue);
void							JS_PutObjectBoolean(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, bool bValue);
void							JS_PutObjectObject(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName, v8::Local<v8::Object> pPut);
void							JS_PutObjectNull(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj, const wchar_t* PropertyName);
unsigned						JS_PutArrayElement(IJS_Runtime* pJSRuntime, v8::Local<v8::Array> pArray,unsigned index,v8::Local<v8::Value> pValue,FXJSVALUETYPE eType);
v8::Local<v8::Value>			JS_GetArrayElement(IJS_Runtime* pJSRuntime, v8::Local<v8::Array> pArray,unsigned index);
unsigned						JS_GetArrayLength(v8::Local<v8::Array> pArray);
v8::Local<v8::Value>			JS_GetListValue(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pList, int index);


v8::Local<v8::Array>			JS_NewArray(IJS_Runtime* pJSRuntime);
v8::Local<v8::Value>			JS_NewNumber(IJS_Runtime* pJSRuntime,int number);
v8::Local<v8::Value>			JS_NewNumber(IJS_Runtime* pJSRuntime,double number);
v8::Local<v8::Value>			JS_NewNumber(IJS_Runtime* pJSRuntime,float number);
v8::Local<v8::Value>			JS_NewBoolean(IJS_Runtime* pJSRuntime,bool b);
v8::Local<v8::Value>			JS_NewObject(IJS_Runtime* pJSRuntime,v8::Local<v8::Object> pObj);
v8::Local<v8::Value>			JS_NewObject2(IJS_Runtime* pJSRuntime,v8::Local<v8::Array> pObj);
v8::Local<v8::Value>			JS_NewString(IJS_Runtime* pJSRuntime,const wchar_t* string);
v8::Local<v8::Value>			JS_NewString(IJS_Runtime* pJSRuntime,const wchar_t* string, unsigned nLen);
v8::Local<v8::Value>			JS_NewNull();
v8::Local<v8::Value>			JS_NewDate(IJS_Runtime* pJSRuntime,double d);
v8::Local<v8::Value>			JS_NewValue(IJS_Runtime* pJSRuntime);


int								JS_ToInt32(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
bool							JS_ToBoolean(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
double							JS_ToNumber(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
v8::Local<v8::Object>			JS_ToObject(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
CFX_WideString					JS_ToString(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
v8::Local<v8::Array>			JS_ToArray(IJS_Runtime* pJSRuntime, v8::Local<v8::Value> pValue);
void							JS_ValueCopy(v8::Local<v8::Value>& pTo, v8::Local<v8::Value> pFrom);

double							JS_GetDateTime();
int								JS_GetYearFromTime(double dt);
int								JS_GetMonthFromTime(double dt);
int								JS_GetDayFromTime(double dt);
int								JS_GetHourFromTime(double dt);
int								JS_GetMinFromTime(double dt);
int								JS_GetSecFromTime(double dt);
double							JS_DateParse(const wchar_t* string);
double							JS_MakeDay(int nYear, int nMonth, int nDay);
double							JS_MakeTime(int nHour, int nMin, int nSec, int nMs);
double							JS_MakeDate(double day, double time);
bool							JS_PortIsNan(double d);
double							JS_LocalTime(double d);

#endif  // FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
