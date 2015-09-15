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

enum FXJSOBJTYPE {
  JS_DYNAMIC = 0,
  JS_STATIC = 1,
};

enum FXJSVALUETYPE {
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

struct FXJSErr {
  const wchar_t* message;
  const wchar_t* srcline;
  unsigned linnum;
};

extern const wchar_t kFXJSValueNameString[];
extern const wchar_t kFXJSValueNameNumber[];
extern const wchar_t kFXJSValueNameBoolean[];
extern const wchar_t kFXJSValueNameDate[];
extern const wchar_t kFXJSValueNameObject[];
extern const wchar_t kFXJSValueNameFxobj[];
extern const wchar_t kFXJSValueNameNull[];
extern const wchar_t kFXJSValueNameUndefined[];

// FXJS_V8 places no interpretation on these two classes; it merely
// passes them on to the caller-provided LP_CONSTRUCTORs.
class IFXJS_Context;
class IFXJS_Runtime;

class JS_ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
  void* Allocate(size_t length) override;
  void* AllocateUninitialized(size_t length) override;
  void Free(void* data, size_t length) override;
};

typedef void (*LP_CONSTRUCTOR)(IFXJS_Context* cc,
                               v8::Local<v8::Object> obj,
                               v8::Local<v8::Object> global);
typedef void (*LP_DESTRUCTOR)(v8::Local<v8::Object> obj);

// Call before making JS_PrepareIsolate call.
void JS_Initialize(unsigned int embedderDataSlot);
void JS_Release();

// Call before making JS_Define* calls. Resources allocated here are cleared
// as part of JS_ReleaseRuntime().
void JS_PrepareIsolate(v8::Isolate* pIsolate);

// Always returns a valid, newly-created objDefnID.
int JS_DefineObj(v8::Isolate* pIsolate,
                 const wchar_t* sObjName,
                 FXJSOBJTYPE eObjType,
                 LP_CONSTRUCTOR pConstructor,
                 LP_DESTRUCTOR pDestructor);

void JS_DefineObjMethod(v8::Isolate* pIsolate,
                        int nObjDefnID,
                        const wchar_t* sMethodName,
                        v8::FunctionCallback pMethodCall);
void JS_DefineObjProperty(v8::Isolate* pIsolate,
                          int nObjDefnID,
                          const wchar_t* sPropName,
                          v8::AccessorGetterCallback pPropGet,
                          v8::AccessorSetterCallback pPropPut);
void JS_DefineObjAllProperties(v8::Isolate* pIsolate,
                               int nObjDefnID,
                               v8::NamedPropertyQueryCallback pPropQurey,
                               v8::NamedPropertyGetterCallback pPropGet,
                               v8::NamedPropertySetterCallback pPropPut,
                               v8::NamedPropertyDeleterCallback pPropDel);
void JS_DefineObjConst(v8::Isolate* pIsolate,
                       int nObjDefnID,
                       const wchar_t* sConstName,
                       v8::Local<v8::Value> pDefault);
void JS_DefineGlobalMethod(v8::Isolate* pIsolate,
                           const wchar_t* sMethodName,
                           v8::FunctionCallback pMethodCall);
void JS_DefineGlobalConst(v8::Isolate* pIsolate,
                          const wchar_t* sConstName,
                          v8::Local<v8::Value> pDefault);

// Called after JS_Define* calls made.
void JS_InitializeRuntime(v8::Isolate* pIsolate,
                          IFXJS_Runtime* pFXRuntime,
                          IFXJS_Context* context,
                          v8::Global<v8::Context>& v8PersistentContext);
void JS_ReleaseRuntime(v8::Isolate* pIsolate,
                       v8::Global<v8::Context>& v8PersistentContext);

// Called after JS_InitializeRuntime call made.
int JS_Execute(v8::Isolate* pIsolate,
               IFXJS_Context* pJSContext,
               const wchar_t* script,
               long length,
               FXJSErr* perror);

v8::Local<v8::Object> JS_NewFxDynamicObj(v8::Isolate* pIsolate,
                                         IFXJS_Context* pJSContext,
                                         int nObjDefnID);
v8::Local<v8::Object> JS_GetStaticObj(v8::Isolate* pIsolate, int nObjDefnID);
v8::Local<v8::Object> JS_GetThisObj(v8::Isolate* pIsolate);
int JS_GetObjDefnID(v8::Local<v8::Object> pObj);
v8::Isolate* JS_GetRuntime(v8::Local<v8::Object> pObj);
int JS_GetObjDefnID(v8::Isolate* pIsolate, const wchar_t* pObjName);
void JS_Error(v8::Isolate* isolate, const CFX_WideString& message);
unsigned JS_CalcHash(const wchar_t* main, unsigned nLen);
unsigned JS_CalcHash(const wchar_t* main);
const wchar_t* JS_GetTypeof(v8::Local<v8::Value> pObj);
void JS_SetPrivate(v8::Isolate* pIsolate, v8::Local<v8::Object> pObj, void* p);
void* JS_GetPrivate(v8::Isolate* pIsolate, v8::Local<v8::Object> pObj);
void JS_SetPrivate(v8::Local<v8::Object> pObj, void* p);
void* JS_GetPrivate(v8::Local<v8::Object> pObj);
void JS_FreePrivate(void* p);
void JS_FreePrivate(v8::Local<v8::Object> pObj);
v8::Local<v8::Value> JS_GetObjectValue(v8::Local<v8::Object> pObj);
v8::Local<v8::Value> JS_GetObjectElement(v8::Isolate* pIsolate,
                                         v8::Local<v8::Object> pObj,
                                         const wchar_t* PropertyName);
v8::Local<v8::Array> JS_GetObjectElementNames(v8::Isolate* pIsolate,
                                              v8::Local<v8::Object> pObj);
void JS_PutObjectString(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName,
                        const wchar_t* sValue);
void JS_PutObjectNumber(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName,
                        int nValue);
void JS_PutObjectNumber(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName,
                        float fValue);
void JS_PutObjectNumber(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName,
                        double dValue);
void JS_PutObjectBoolean(v8::Isolate* pIsolate,
                         v8::Local<v8::Object> pObj,
                         const wchar_t* PropertyName,
                         bool bValue);
void JS_PutObjectObject(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName,
                        v8::Local<v8::Object> pPut);
void JS_PutObjectNull(v8::Isolate* pIsolate,
                      v8::Local<v8::Object> pObj,
                      const wchar_t* PropertyName);
unsigned JS_PutArrayElement(v8::Isolate* pIsolate,
                            v8::Local<v8::Array> pArray,
                            unsigned index,
                            v8::Local<v8::Value> pValue,
                            FXJSVALUETYPE eType);
v8::Local<v8::Value> JS_GetArrayElement(v8::Isolate* pIsolate,
                                        v8::Local<v8::Array> pArray,
                                        unsigned index);
unsigned JS_GetArrayLength(v8::Local<v8::Array> pArray);
v8::Local<v8::Value> JS_GetListValue(v8::Isolate* pIsolate,
                                     v8::Local<v8::Value> pList,
                                     int index);

v8::Local<v8::Array> JS_NewArray(v8::Isolate* pIsolate);
v8::Local<v8::Value> JS_NewNumber(v8::Isolate* pIsolate, int number);
v8::Local<v8::Value> JS_NewNumber(v8::Isolate* pIsolate, double number);
v8::Local<v8::Value> JS_NewNumber(v8::Isolate* pIsolate, float number);
v8::Local<v8::Value> JS_NewBoolean(v8::Isolate* pIsolate, bool b);
v8::Local<v8::Value> JS_NewObject(v8::Isolate* pIsolate,
                                  v8::Local<v8::Object> pObj);
v8::Local<v8::Value> JS_NewObject2(v8::Isolate* pIsolate,
                                   v8::Local<v8::Array> pObj);
v8::Local<v8::Value> JS_NewString(v8::Isolate* pIsolate, const wchar_t* string);
v8::Local<v8::Value> JS_NewString(v8::Isolate* pIsolate,
                                  const wchar_t* string,
                                  unsigned nLen);
v8::Local<v8::Value> JS_NewNull();
v8::Local<v8::Value> JS_NewDate(v8::Isolate* pIsolate, double d);
v8::Local<v8::Value> JS_NewValue(v8::Isolate* pIsolate);

int JS_ToInt32(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
bool JS_ToBoolean(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
double JS_ToNumber(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
v8::Local<v8::Object> JS_ToObject(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value> pValue);
CFX_WideString JS_ToString(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
v8::Local<v8::Array> JS_ToArray(v8::Isolate* pIsolate,
                                v8::Local<v8::Value> pValue);
void JS_ValueCopy(v8::Local<v8::Value>& pTo, v8::Local<v8::Value> pFrom);

double JS_GetDateTime();
int JS_GetYearFromTime(double dt);
int JS_GetMonthFromTime(double dt);
int JS_GetDayFromTime(double dt);
int JS_GetHourFromTime(double dt);
int JS_GetMinFromTime(double dt);
int JS_GetSecFromTime(double dt);
double JS_DateParse(const wchar_t* string);
double JS_MakeDay(int nYear, int nMonth, int nDay);
double JS_MakeTime(int nHour, int nMin, int nSec, int nMs);
double JS_MakeDate(double day, double time);
bool JS_PortIsNan(double d);
double JS_LocalTime(double d);

#endif  // FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
