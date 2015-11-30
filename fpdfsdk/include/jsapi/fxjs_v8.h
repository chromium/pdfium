// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

// FXJS_V8 is a layer that makes it easier to define native objects in V8, but
// has no knowledge of PDF-specific native objects. It could in theory be used
// to implement other sets of native objects.

// PDFium code should include this file rather than including V8 headers
// directly.

#ifndef FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_
#define FPDFSDK_INCLUDE_JSAPI_FXJS_V8_H_

#include <v8.h>

#include <vector>

#include "core/include/fxcrt/fx_string.h"

class CFXJS_ObjDefinition;

// FXJS_V8 places no restrictions on these two classes; it merely passes them
// on to caller-provided methods.
class IJS_Context;  // A description of the event that caused JS execution.
class IJS_Runtime;  // A native runtime, typically owns the v8::Context.

#ifdef PDF_ENABLE_XFA
// FXJS_V8 places no interpreation on this calass; it merely passes it
// along to XFA.
class CFXJSE_RuntimeData;
#endif  // PDF_ENABLE_XFA

enum FXJSOBJTYPE {
  FXJSOBJTYPE_DYNAMIC = 0,  // Created by native method and returned to JS.
  FXJSOBJTYPE_STATIC,       // Created by init and hung off of global object.
  FXJSOBJTYPE_GLOBAL,       // The global object itself (may only appear once).
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

  std::vector<CFXJS_ObjDefinition*> m_ObjectDefnArray;
#ifdef PDF_ENABLE_XFA
  CFXJSE_RuntimeData* m_pFXJSERuntimeData;
#endif  // PDF_ENABLE_XFA

 protected:
#ifndef PDF_ENABLE_XFA
  FXJS_PerIsolateData() {}
#else  // PDF_ENABLE_XFA
  FXJS_PerIsolateData() : m_pFXJSERuntimeData(nullptr) {}
#endif  // PDF_ENABLE_XFA
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

using FXJS_CONSTRUCTOR = void (*)(IJS_Runtime* cc, v8::Local<v8::Object> obj);
using FXJS_DESTRUCTOR = void (*)(v8::Local<v8::Object> obj);

// Call before making FXJS_PrepareIsolate call.
void FXJS_Initialize(unsigned int embedderDataSlot, v8::Isolate* pIsolate);
void FXJS_Release();

// Gets the global isolate set by FXJS_Initialize(), or makes a new one each
// time if there is no such isolate. Returns true if a new isolate had to be
// created.
bool FXJS_GetIsolate(v8::Isolate** pResultIsolate);

// Get the global isolate's ref count.
size_t FXJS_GlobalIsolateRefCount();

// Call before making FXJS_Define* calls. Resources allocated here are cleared
// as part of FXJS_ReleaseRuntime().
void FXJS_PrepareIsolate(v8::Isolate* pIsolate);

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
void FXJS_InitializeRuntime(
    v8::Isolate* pIsolate,
    IJS_Runtime* pIRuntime,
    v8::Global<v8::Context>* pV8PersistentContext,
    std::vector<v8::Global<v8::Object>*>* pStaticObjects);
void FXJS_ReleaseRuntime(v8::Isolate* pIsolate,
                         v8::Global<v8::Context>* pV8PersistentContext,
                         std::vector<v8::Global<v8::Object>*>* pStaticObjects);
IJS_Runtime* FXJS_GetRuntimeFromIsolate(v8::Isolate* pIsolate);

#ifdef PDF_ENABLE_XFA
// Called as part of FXJS_InitializeRuntime, exposed so PDF can make its
// own contexts compatible with XFA or vice versa.
void FXJS_SetRuntimeForV8Context(v8::Local<v8::Context> v8Context,
                                 IJS_Runtime* pIRuntime);
#endif  // PDF_ENABLE_XFA

// Called after FXJS_InitializeRuntime call made.
int FXJS_Execute(v8::Isolate* pIsolate,
                 IJS_Context* pJSContext,
                 const wchar_t* script,
                 FXJSErr* perror);

v8::Local<v8::Object> FXJS_NewFxDynamicObj(v8::Isolate* pIsolate,
                                           IJS_Runtime* pJSContext,
                                           int nObjDefnID);
v8::Local<v8::Object> FXJS_GetThisObj(v8::Isolate* pIsolate);
int FXJS_GetObjDefnID(v8::Local<v8::Object> pObj);
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
