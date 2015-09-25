// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

// PDFium wrapper around V8 APIs. PDFium code should include this file rather
// than including V8 headers directly.

#ifndef FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
#define FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_

#include <v8.h>
#include "../../../core/include/fxcrt/fx_basic.h"

// FXJS_V8 places no interpretation on these two classes; it merely
// passes them on to the caller-provided FXJS_CONSTRUCTORs.
class IFXJS_Context;
class IFXJS_Runtime;

enum FXJSOBJTYPE {
  FXJS_DYNAMIC = 0,
  FXJS_STATIC = 1,
};

struct FXJSErr {
  const wchar_t* message;
  const wchar_t* srcline;
  unsigned linnum;
};

class FXJS_PerIsolateData {
 public:
  static void SetUp(v8::Isolate* pIsolate);
  static FXJS_PerIsolateData* Get(v8::Isolate* pIsolate);

  CFX_PtrArray m_ObjectDefnArray;

 protected:
  FXJS_PerIsolateData() {}
};

extern const wchar_t kFXJSValueNameString[];
extern const wchar_t kFXJSValueNameNumber[];
extern const wchar_t kFXJSValueNameBoolean[];
extern const wchar_t kFXJSValueNameDate[];
extern const wchar_t kFXJSValueNameObject[];
extern const wchar_t kFXJSValueNameFxobj[];
extern const wchar_t kFXJSValueNameNull[];
extern const wchar_t kFXJSValueNameUndefined[];


class FXJS_ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
  void* Allocate(size_t length) override;
  void* AllocateUninitialized(size_t length) override;
  void Free(void* data, size_t length) override;
};

using FXJS_CONSTRUCTOR = void (*)(IFXJS_Context* cc,
                                  v8::Local<v8::Object> obj,
                                  v8::Local<v8::Object> global);
using FXJS_DESTRUCTOR = void (*)(v8::Local<v8::Object> obj);

// Call before making FXJS_PrepareIsolate call.
void FXJS_Initialize(unsigned int embedderDataSlot);
void FXJS_Release();

// Call before making FXJS_Define* calls. Resources allocated here are cleared
// as part of FXJS_ReleaseRuntime().
void FXJS_PrepareIsolate(v8::Isolate* pIsolate);

// Call before making JS_PrepareIsolate call.
void JS_Initialize(unsigned int embedderDataSlot);
void JS_Release();

// Call before making JS_Define* calls. Resources allocated here are cleared
// as part of JS_ReleaseRuntime().
void JS_PrepareIsolate(v8::Isolate* pIsolate);

// Always returns a valid, newly-created objDefnID.
int FXJS_DefineObj(v8::Isolate* pIsolate,
                   const wchar_t* sObjName,
                   FXJSOBJTYPE eObjType,
                   FXJS_CONSTRUCTOR pConstructor,
                   FXJS_DESTRUCTOR pDestructor);

void FXJS_DefineObjMethod(v8::Isolate* pIsolate,
                          int nObjDefnID,
                          const wchar_t* sMethodName,
                          v8::FunctionCallback pMethodCall);
void FXJS_DefineObjProperty(v8::Isolate* pIsolate,
                            int nObjDefnID,
                            const wchar_t* sPropName,
                            v8::AccessorGetterCallback pPropGet,
                            v8::AccessorSetterCallback pPropPut);
void FXJS_DefineObjAllProperties(v8::Isolate* pIsolate,
                                 int nObjDefnID,
                                 v8::NamedPropertyQueryCallback pPropQurey,
                                 v8::NamedPropertyGetterCallback pPropGet,
                                 v8::NamedPropertySetterCallback pPropPut,
                                 v8::NamedPropertyDeleterCallback pPropDel);
void FXJS_DefineObjConst(v8::Isolate* pIsolate,
                         int nObjDefnID,
                         const wchar_t* sConstName,
                         v8::Local<v8::Value> pDefault);
void FXJS_DefineGlobalMethod(v8::Isolate* pIsolate,
                             const wchar_t* sMethodName,
                             v8::FunctionCallback pMethodCall);
void FXJS_DefineGlobalConst(v8::Isolate* pIsolate,
                            const wchar_t* sConstName,
                            v8::Local<v8::Value> pDefault);

// Called after FXJS_Define* calls made.
void FXJS_InitializeRuntime(v8::Isolate* pIsolate,
                            IFXJS_Runtime* pFXRuntime,
                            IFXJS_Context* context,
                            v8::Global<v8::Context>& v8PersistentContext);
void FXJS_ReleaseRuntime(v8::Isolate* pIsolate,
                         v8::Global<v8::Context>& v8PersistentContext);

// Called after FXJS_InitializeRuntime call made.
int FXJS_Execute(v8::Isolate* pIsolate,
                 IFXJS_Context* pJSContext,
                 const wchar_t* script,
                 long length,
                 FXJSErr* perror);

v8::Local<v8::Object> FXJS_NewFxDynamicObj(v8::Isolate* pIsolate,
                                           IFXJS_Context* pJSContext,
                                           int nObjDefnID);
v8::Local<v8::Object> FXJS_GetThisObj(v8::Isolate* pIsolate);
int FXJS_GetObjDefnID(v8::Local<v8::Object> pObj);
int FXJS_GetObjDefnID(v8::Isolate* pIsolate, const wchar_t* pObjName);
v8::Isolate* FXJS_GetRuntime(v8::Local<v8::Object> pObj);
const wchar_t* FXJS_GetTypeof(v8::Local<v8::Value> pObj);

void FXJS_SetPrivate(v8::Isolate* pIsolate,
                     v8::Local<v8::Object> pObj,
                     void* p);
void* FXJS_GetPrivate(v8::Isolate* pIsolate, v8::Local<v8::Object> pObj);
void FXJS_FreePrivate(void* p);
void FXJS_FreePrivate(v8::Local<v8::Object> pObj);

void FXJS_Error(v8::Isolate* isolate, const CFX_WideString& message);
v8::Local<v8::String> FXJS_WSToJSString(v8::Isolate* pIsolate,
                                        const wchar_t* PropertyName,
                                        int Len = -1);

v8::Local<v8::Value> FXJS_GetObjectElement(v8::Isolate* pIsolate,
                                           v8::Local<v8::Object> pObj,
                                           const wchar_t* PropertyName);
v8::Local<v8::Array> FXJS_GetObjectElementNames(v8::Isolate* pIsolate,
                                                v8::Local<v8::Object> pObj);

v8::Local<v8::Value> FXJS_GetArrayElement(v8::Isolate* pIsolate,
                                          v8::Local<v8::Array> pArray,
                                          unsigned index);
unsigned FXJS_GetArrayLength(v8::Local<v8::Array> pArray);

void FXJS_PutObjectString(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          const wchar_t* sValue);
void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          int nValue);
void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          float fValue);
void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          double dValue);
void FXJS_PutObjectBoolean(v8::Isolate* pIsolate,
                           v8::Local<v8::Object> pObj,
                           const wchar_t* PropertyName,
                           bool bValue);
void FXJS_PutObjectObject(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          v8::Local<v8::Object> pPut);
void FXJS_PutObjectNull(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName);
unsigned FXJS_PutArrayElement(v8::Isolate* pIsolate,
                              v8::Local<v8::Array> pArray,
                              unsigned index,
                              v8::Local<v8::Value> pValue);

v8::Local<v8::Array> FXJS_NewArray(v8::Isolate* pIsolate);
v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, int number);
v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, double number);
v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, float number);
v8::Local<v8::Value> FXJS_NewBoolean(v8::Isolate* pIsolate, bool b);
v8::Local<v8::Value> FXJS_NewObject(v8::Isolate* pIsolate,
                                    v8::Local<v8::Object> pObj);
v8::Local<v8::Value> FXJS_NewObject2(v8::Isolate* pIsolate,
                                     v8::Local<v8::Array> pObj);
v8::Local<v8::Value> FXJS_NewString(v8::Isolate* pIsolate,
                                    const wchar_t* string);
v8::Local<v8::Value> FXJS_NewNull();
v8::Local<v8::Value> FXJS_NewDate(v8::Isolate* pIsolate, double d);

int FXJS_ToInt32(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
bool FXJS_ToBoolean(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
double FXJS_ToNumber(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue);
v8::Local<v8::Object> FXJS_ToObject(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value> pValue);
CFX_WideString FXJS_ToString(v8::Isolate* pIsolate,
                             v8::Local<v8::Value> pValue);
v8::Local<v8::Array> FXJS_ToArray(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value> pValue);
void FXJS_ValueCopy(v8::Local<v8::Value>& pTo, v8::Local<v8::Value> pFrom);

#endif  // FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
