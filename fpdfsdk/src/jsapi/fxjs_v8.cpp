// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../core/include/fxcrt/fx_basic.h"
#include "../../include/fsdk_define.h"
#include "../../include/jsapi/fxjs_v8.h"

const wchar_t kFXJSValueNameString[] = L"string";
const wchar_t kFXJSValueNameNumber[] = L"number";
const wchar_t kFXJSValueNameBoolean[] = L"boolean";
const wchar_t kFXJSValueNameDate[] = L"date";
const wchar_t kFXJSValueNameObject[] = L"object";
const wchar_t kFXJSValueNameFxobj[] = L"fxobj";
const wchar_t kFXJSValueNameNull[] = L"null";
const wchar_t kFXJSValueNameUndefined[] = L"undefined";

static unsigned int g_embedderDataSlot = 1u;

class CFXJS_PrivateData {
 public:
  CFXJS_PrivateData(int nObjDefID) : ObjDefID(nObjDefID), pPrivate(NULL) {}

  int ObjDefID;
  void* pPrivate;
};

class CFXJS_ObjDefinition {
 public:
  static int MaxID(v8::Isolate* pIsolate) {
    return static_cast<int>(
        FXJS_PerIsolateData::Get(pIsolate)->m_ObjectDefnArray.GetSize());
  }
  static CFXJS_ObjDefinition* ForID(v8::Isolate* pIsolate, int id) {
    // Note: GetAt() halts if out-of-range even in release builds.
    return static_cast<CFXJS_ObjDefinition*>(
        FXJS_PerIsolateData::Get(pIsolate)->m_ObjectDefnArray.GetAt(id));
  }
  CFXJS_ObjDefinition(v8::Isolate* isolate,
                      const wchar_t* sObjName,
                      FXJSOBJTYPE eObjType,
                      FXJS_CONSTRUCTOR pConstructor,
                      FXJS_DESTRUCTOR pDestructor)
      : objName(sObjName),
        objType(eObjType),
        m_pConstructor(pConstructor),
        m_pDestructor(pDestructor),
        m_bSetAsGlobalObject(FALSE) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::ObjectTemplate> objTemplate =
        v8::ObjectTemplate::New(isolate);
    objTemplate->SetInternalFieldCount(2);
    m_objTemplate.Reset(isolate, objTemplate);

    // Document as the global object.
    if (FXSYS_wcscmp(sObjName, L"Document") == 0) {
      m_bSetAsGlobalObject = TRUE;
    }
  }
  ~CFXJS_ObjDefinition() {
    m_objTemplate.Reset();
    m_StaticObj.Reset();
  }

  const wchar_t* objName;
  FXJSOBJTYPE objType;
  FXJS_CONSTRUCTOR m_pConstructor;
  FXJS_DESTRUCTOR m_pDestructor;
  FX_BOOL m_bSetAsGlobalObject;

  v8::Global<v8::ObjectTemplate> m_objTemplate;
  v8::Global<v8::Object> m_StaticObj;
};

void* FXJS_ArrayBufferAllocator::Allocate(size_t length) {
  return calloc(1, length);
}

void* FXJS_ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  return malloc(length);
}

void FXJS_ArrayBufferAllocator::Free(void* data, size_t length) {
  free(data);
}

// static
void FXJS_PerIsolateData::SetUp(v8::Isolate* pIsolate) {
  if (!pIsolate->GetData(g_embedderDataSlot))
    pIsolate->SetData(g_embedderDataSlot, new FXJS_PerIsolateData());
}

// static
FXJS_PerIsolateData* FXJS_PerIsolateData::Get(v8::Isolate* pIsolate) {
  return static_cast<FXJS_PerIsolateData*>(
      pIsolate->GetData(g_embedderDataSlot));
}

int FXJS_DefineObj(v8::Isolate* pIsolate,
                   const wchar_t* sObjName,
                   FXJSOBJTYPE eObjType,
                   FXJS_CONSTRUCTOR pConstructor,
                   FXJS_DESTRUCTOR pDestructor) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  FXJS_PerIsolateData::SetUp(pIsolate);
  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate);
  pData->m_ObjectDefnArray.Add(new CFXJS_ObjDefinition(
      pIsolate, sObjName, eObjType, pConstructor, pDestructor));
  return pData->m_ObjectDefnArray.GetSize() - 1;
}

void FXJS_DefineObjMethod(v8::Isolate* pIsolate,
                          int nObjDefnID,
                          const wchar_t* sMethodName,
                          v8::FunctionCallback pMethodCall) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFX_ByteString bsMethodName = CFX_WideString(sMethodName).UTF8Encode();
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(pIsolate, nObjDefnID);
  v8::Local<v8::ObjectTemplate> objTemp =
      v8::Local<v8::ObjectTemplate>::New(pIsolate, pObjDef->m_objTemplate);

  objTemp->Set(
      v8::String::NewFromUtf8(pIsolate, bsMethodName.c_str(),
                              v8::NewStringType::kNormal).ToLocalChecked(),
      v8::FunctionTemplate::New(pIsolate, pMethodCall), v8::ReadOnly);
  pObjDef->m_objTemplate.Reset(pIsolate, objTemp);
}

void FXJS_DefineObjProperty(v8::Isolate* pIsolate,
                            int nObjDefnID,
                            const wchar_t* sPropName,
                            v8::AccessorGetterCallback pPropGet,
                            v8::AccessorSetterCallback pPropPut) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFX_ByteString bsPropertyName = CFX_WideString(sPropName).UTF8Encode();
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(pIsolate, nObjDefnID);
  v8::Local<v8::ObjectTemplate> objTemp =
      v8::Local<v8::ObjectTemplate>::New(pIsolate, pObjDef->m_objTemplate);
  objTemp->SetAccessor(
      v8::String::NewFromUtf8(pIsolate, bsPropertyName.c_str(),
                              v8::NewStringType::kNormal).ToLocalChecked(),
      pPropGet, pPropPut);
  pObjDef->m_objTemplate.Reset(pIsolate, objTemp);
}

void FXJS_DefineObjAllProperties(v8::Isolate* pIsolate,
                                 int nObjDefnID,
                                 v8::NamedPropertyQueryCallback pPropQurey,
                                 v8::NamedPropertyGetterCallback pPropGet,
                                 v8::NamedPropertySetterCallback pPropPut,
                                 v8::NamedPropertyDeleterCallback pPropDel) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(pIsolate, nObjDefnID);
  v8::Local<v8::ObjectTemplate> objTemp =
      v8::Local<v8::ObjectTemplate>::New(pIsolate, pObjDef->m_objTemplate);
  objTemp->SetNamedPropertyHandler(pPropGet, pPropPut, pPropQurey, pPropDel);
  pObjDef->m_objTemplate.Reset(pIsolate, objTemp);
}

void FXJS_DefineObjConst(v8::Isolate* pIsolate,
                         int nObjDefnID,
                         const wchar_t* sConstName,
                         v8::Local<v8::Value> pDefault) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFX_ByteString bsConstName = CFX_WideString(sConstName).UTF8Encode();
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(pIsolate, nObjDefnID);
  v8::Local<v8::ObjectTemplate> objTemp =
      v8::Local<v8::ObjectTemplate>::New(pIsolate, pObjDef->m_objTemplate);
  objTemp->Set(pIsolate, bsConstName.c_str(), pDefault);
  pObjDef->m_objTemplate.Reset(pIsolate, objTemp);
}

static v8::Global<v8::ObjectTemplate>& _getGlobalObjectTemplate(
    v8::Isolate* pIsolate) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  int maxID = CFXJS_ObjDefinition::MaxID(pIsolate);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(pIsolate, i);
    if (pObjDef->m_bSetAsGlobalObject)
      return pObjDef->m_objTemplate;
  }
  static v8::Global<v8::ObjectTemplate> gloabalObjectTemplate;
  return gloabalObjectTemplate;
}

void FXJS_DefineGlobalMethod(v8::Isolate* pIsolate,
                             const wchar_t* sMethodName,
                             v8::FunctionCallback pMethodCall) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFX_ByteString bsMethodName = CFX_WideString(sMethodName).UTF8Encode();
  v8::Local<v8::FunctionTemplate> funTempl =
      v8::FunctionTemplate::New(pIsolate, pMethodCall);
  v8::Local<v8::ObjectTemplate> objTemp;

  v8::Global<v8::ObjectTemplate>& globalObjTemp =
      _getGlobalObjectTemplate(pIsolate);
  if (globalObjTemp.IsEmpty())
    objTemp = v8::ObjectTemplate::New(pIsolate);
  else
    objTemp = v8::Local<v8::ObjectTemplate>::New(pIsolate, globalObjTemp);
  objTemp->Set(
      v8::String::NewFromUtf8(pIsolate, bsMethodName.c_str(),
                              v8::NewStringType::kNormal).ToLocalChecked(),
      funTempl, v8::ReadOnly);

  globalObjTemp.Reset(pIsolate, objTemp);
}

void FXJS_DefineGlobalConst(v8::Isolate* pIsolate,
                            const wchar_t* sConstName,
                            v8::Local<v8::Value> pDefault) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  CFX_WideString ws = CFX_WideString(sConstName);
  CFX_ByteString bsConst = ws.UTF8Encode();

  v8::Local<v8::ObjectTemplate> objTemp;

  v8::Global<v8::ObjectTemplate>& globalObjTemp =
      _getGlobalObjectTemplate(pIsolate);
  if (globalObjTemp.IsEmpty())
    objTemp = v8::ObjectTemplate::New(pIsolate);
  else
    objTemp = v8::Local<v8::ObjectTemplate>::New(pIsolate, globalObjTemp);
  objTemp->Set(
      v8::String::NewFromUtf8(pIsolate, bsConst.c_str(),
                              v8::NewStringType::kNormal).ToLocalChecked(),
      pDefault, v8::ReadOnly);

  globalObjTemp.Reset(pIsolate, objTemp);
}

void FXJS_InitializeRuntime(v8::Isolate* pIsolate,
                            IFXJS_Runtime* pFXRuntime,
                            IFXJS_Context* context,
                            v8::Global<v8::Context>& v8PersistentContext) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);

  v8::Global<v8::ObjectTemplate>& globalObjTemp =
      _getGlobalObjectTemplate(pIsolate);
  v8::Local<v8::Context> v8Context = v8::Context::New(
      pIsolate, NULL,
      v8::Local<v8::ObjectTemplate>::New(pIsolate, globalObjTemp));
  v8::Context::Scope context_scope(v8Context);

  FXJS_PerIsolateData::SetUp(pIsolate);
  v8::Local<v8::External> ptr = v8::External::New(pIsolate, pFXRuntime);
  v8Context->SetEmbedderData(1, ptr);

  int maxID = CFXJS_ObjDefinition::MaxID(pIsolate);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(pIsolate, i);
    CFX_WideString ws = CFX_WideString(pObjDef->objName);
    CFX_ByteString bs = ws.UTF8Encode();
    v8::Local<v8::String> objName =
        v8::String::NewFromUtf8(pIsolate, bs.c_str(),
                                v8::NewStringType::kNormal,
                                bs.GetLength()).ToLocalChecked();

    if (pObjDef->objType == FXJS_DYNAMIC) {
      // Document is set as global object, need to construct it first.
      if (ws.Equal(L"Document")) {
        v8Context->Global()
            ->GetPrototype()
            ->ToObject(v8Context)
            .ToLocalChecked()
            ->SetAlignedPointerInInternalField(0, new CFXJS_PrivateData(i));

        if (pObjDef->m_pConstructor)
          pObjDef->m_pConstructor(context, v8Context->Global()
                                               ->GetPrototype()
                                               ->ToObject(v8Context)
                                               .ToLocalChecked(),
                                  v8Context->Global()
                                      ->GetPrototype()
                                      ->ToObject(v8Context)
                                      .ToLocalChecked());
      }
    } else {
      v8::Local<v8::Object> obj = FXJS_NewFxDynamicObj(pIsolate, context, i);
      v8Context->Global()->Set(v8Context, objName, obj).FromJust();
      pObjDef->m_StaticObj.Reset(pIsolate, obj);
    }
  }
  v8PersistentContext.Reset(pIsolate, v8Context);
}

void FXJS_ReleaseRuntime(v8::Isolate* pIsolate,
                         v8::Global<v8::Context>& v8PersistentContext) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::HandleScope handle_scope(pIsolate);
  v8::Local<v8::Context> context =
      v8::Local<v8::Context>::New(pIsolate, v8PersistentContext);
  v8::Context::Scope context_scope(context);

  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate);
  if (!pData)
    return;

  int maxID = CFXJS_ObjDefinition::MaxID(pIsolate);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(pIsolate, i);
    if (!pObjDef->m_StaticObj.IsEmpty()) {
      v8::Local<v8::Object> pObj =
          v8::Local<v8::Object>::New(pIsolate, pObjDef->m_StaticObj);
      if (pObjDef->m_pDestructor)
        pObjDef->m_pDestructor(pObj);
      FXJS_FreePrivate(pObj);
    }
    delete pObjDef;
  }

  pIsolate->SetData(g_embedderDataSlot, nullptr);
  delete pData;
}

void FXJS_Initialize(unsigned int embedderDataSlot) {
  g_embedderDataSlot = embedderDataSlot;
}

void FXJS_Release() {
}

int FXJS_Execute(v8::Isolate* pIsolate,
                 IFXJS_Context* pJSContext,
                 const wchar_t* script,
                 long length,
                 FXJSErr* pError) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::TryCatch try_catch(pIsolate);

  CFX_WideString wsScript(script);
  CFX_ByteString bsScript = wsScript.UTF8Encode();

  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  v8::Local<v8::Script> compiled_script;
  if (!v8::Script::Compile(
           context, v8::String::NewFromUtf8(
                        pIsolate, bsScript.c_str(), v8::NewStringType::kNormal,
                        bsScript.GetLength()).ToLocalChecked())
           .ToLocal(&compiled_script)) {
    v8::String::Utf8Value error(try_catch.Exception());
    // TODO(tsepez): return error via pError->message.
    return -1;
  }

  v8::Local<v8::Value> result;
  if (!compiled_script->Run(context).ToLocal(&result)) {
    v8::String::Utf8Value error(try_catch.Exception());
    // TODO(tsepez): return error via pError->message.
    return -1;
  }
  return 0;
}

v8::Local<v8::Object> FXJS_NewFxDynamicObj(v8::Isolate* pIsolate,
                                           IFXJS_Context* pJSContext,
                                           int nObjDefnID) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  if (nObjDefnID == -1) {
    v8::Local<v8::ObjectTemplate> objTempl = v8::ObjectTemplate::New(pIsolate);
    v8::Local<v8::Object> obj;
    if (objTempl->NewInstance(context).ToLocal(&obj))
      return obj;
    return v8::Local<v8::Object>();
  }

  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(pIsolate);
  if (!pData)
    return v8::Local<v8::Object>();

  if (nObjDefnID < 0 || nObjDefnID >= CFXJS_ObjDefinition::MaxID(pIsolate))
    return v8::Local<v8::Object>();

  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(pIsolate, nObjDefnID);

  v8::Local<v8::ObjectTemplate> objTemp =
      v8::Local<v8::ObjectTemplate>::New(pIsolate, pObjDef->m_objTemplate);
  v8::Local<v8::Object> obj;
  if (!objTemp->NewInstance(context).ToLocal(&obj))
    return v8::Local<v8::Object>();

  obj->SetAlignedPointerInInternalField(0, new CFXJS_PrivateData(nObjDefnID));
  if (pObjDef->m_pConstructor)
    pObjDef->m_pConstructor(
        pJSContext, obj,
        context->Global()->GetPrototype()->ToObject(context).ToLocalChecked());

  return obj;
}

v8::Local<v8::Object> FXJS_GetThisObj(v8::Isolate* pIsolate) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  if (!FXJS_PerIsolateData::Get(pIsolate))
    return v8::Local<v8::Object>();

  // Return the global object.
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return context->Global()->GetPrototype()->ToObject(context).ToLocalChecked();
}

int FXJS_GetObjDefnID(v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty() || !pObj->InternalFieldCount())
    return -1;
  CFXJS_PrivateData* pPrivateData =
      (CFXJS_PrivateData*)pObj->GetAlignedPointerFromInternalField(0);
  if (pPrivateData)
    return pPrivateData->ObjDefID;
  return -1;
}

v8::Isolate* FXJS_GetRuntime(v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return NULL;
  v8::Local<v8::Context> context = pObj->CreationContext();
  if (context.IsEmpty())
    return NULL;
  return context->GetIsolate();
}

int FXJS_GetObjDefnID(v8::Isolate* pIsolate, const wchar_t* pObjName) {
  v8::Isolate::Scope isolate_scope(pIsolate);
  if (!FXJS_PerIsolateData::Get(pIsolate))
    return -1;

  int maxID = CFXJS_ObjDefinition::MaxID(pIsolate);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(pIsolate, i);
    if (FXSYS_wcscmp(pObjDef->objName, pObjName) == 0)
      return i;
  }
  return -1;
}

void FXJS_Error(v8::Isolate* pIsolate, const CFX_WideString& message) {
  // Conversion from pdfium's wchar_t wide-strings to v8's uint16_t
  // wide-strings isn't handled by v8, so use UTF8 as a common
  // intermediate format.
  CFX_ByteString utf8_message = message.UTF8Encode();
  pIsolate->ThrowException(
      v8::String::NewFromUtf8(pIsolate, utf8_message.c_str(),
                              v8::NewStringType::kNormal).ToLocalChecked());
}

const wchar_t* FXJS_GetTypeof(v8::Local<v8::Value> pObj) {
  if (pObj.IsEmpty())
    return NULL;
  if (pObj->IsString())
    return kFXJSValueNameString;
  if (pObj->IsNumber())
    return kFXJSValueNameNumber;
  if (pObj->IsBoolean())
    return kFXJSValueNameBoolean;
  if (pObj->IsDate())
    return kFXJSValueNameDate;
  if (pObj->IsObject())
    return kFXJSValueNameObject;
  if (pObj->IsNull())
    return kFXJSValueNameNull;
  if (pObj->IsUndefined())
    return kFXJSValueNameUndefined;
  return NULL;
}

void FXJS_SetPrivate(v8::Isolate* pIsolate,
                     v8::Local<v8::Object> pObj,
                     void* p) {
  if (pObj.IsEmpty() || !pObj->InternalFieldCount())
    return;
  CFXJS_PrivateData* pPrivateData =
      (CFXJS_PrivateData*)pObj->GetAlignedPointerFromInternalField(0);
  if (!pPrivateData)
    return;
  pPrivateData->pPrivate = p;
}

void* FXJS_GetPrivate(v8::Isolate* pIsolate, v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return nullptr;
  CFXJS_PrivateData* pPrivateData = nullptr;
  if (pObj->InternalFieldCount()) {
    pPrivateData =
        (CFXJS_PrivateData*)pObj->GetAlignedPointerFromInternalField(0);
  } else {
    // It could be a global proxy object.
    v8::Local<v8::Value> v = pObj->GetPrototype();
    v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
    if (v->IsObject())
      pPrivateData = (CFXJS_PrivateData*)v->ToObject(context)
                         .ToLocalChecked()
                         ->GetAlignedPointerFromInternalField(0);
  }
  return pPrivateData ? pPrivateData->pPrivate : nullptr;
}

void FXJS_FreePrivate(void* pPrivateData) {
  delete (CFXJS_PrivateData*)pPrivateData;
}

void FXJS_FreePrivate(v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty() || !pObj->InternalFieldCount())
    return;
  FXJS_FreePrivate(pObj->GetAlignedPointerFromInternalField(0));
  pObj->SetAlignedPointerInInternalField(0, NULL);
}

v8::Local<v8::String> FXJS_WSToJSString(v8::Isolate* pIsolate,
                                        const wchar_t* PropertyName,
                                        int Len) {
  CFX_WideString ws = CFX_WideString(PropertyName, Len);
  CFX_ByteString bs = ws.UTF8Encode();
  if (!pIsolate)
    pIsolate = v8::Isolate::GetCurrent();
  return v8::String::NewFromUtf8(pIsolate, bs.c_str(),
                                 v8::NewStringType::kNormal).ToLocalChecked();
}

v8::Local<v8::Value> FXJS_GetObjectElement(v8::Isolate* pIsolate,
                                           v8::Local<v8::Object> pObj,
                                           const wchar_t* PropertyName) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Value>();
  v8::Local<v8::Value> val;
  if (!pObj->Get(pIsolate->GetCurrentContext(),
                 FXJS_WSToJSString(pIsolate, PropertyName)).ToLocal(&val))
    return v8::Local<v8::Value>();
  return val;
}

v8::Local<v8::Array> FXJS_GetObjectElementNames(v8::Isolate* pIsolate,
                                                v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Array>();
  v8::Local<v8::Array> val;
  if (!pObj->GetPropertyNames(pIsolate->GetCurrentContext()).ToLocal(&val))
    return v8::Local<v8::Array>();
  return val;
}

void FXJS_PutObjectString(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          const wchar_t* sValue)  // VT_string
{
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            FXJS_WSToJSString(pIsolate, sValue)).FromJust();
}

void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          int nValue) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            v8::Int32::New(pIsolate, nValue)).FromJust();
}

void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          float fValue) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            v8::Number::New(pIsolate, (double)fValue)).FromJust();
}

void FXJS_PutObjectNumber(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          double dValue) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            v8::Number::New(pIsolate, (double)dValue)).FromJust();
}

void FXJS_PutObjectBoolean(v8::Isolate* pIsolate,
                           v8::Local<v8::Object> pObj,
                           const wchar_t* PropertyName,
                           bool bValue) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            v8::Boolean::New(pIsolate, bValue)).FromJust();
}

void FXJS_PutObjectObject(v8::Isolate* pIsolate,
                          v8::Local<v8::Object> pObj,
                          const wchar_t* PropertyName,
                          v8::Local<v8::Object> pPut) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName), pPut).FromJust();
}

void FXJS_PutObjectNull(v8::Isolate* pIsolate,
                        v8::Local<v8::Object> pObj,
                        const wchar_t* PropertyName) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(pIsolate->GetCurrentContext(),
            FXJS_WSToJSString(pIsolate, PropertyName),
            v8::Local<v8::Object>()).FromJust();
}

v8::Local<v8::Array> FXJS_NewArray(v8::Isolate* pIsolate) {
  return v8::Array::New(pIsolate);
}

unsigned FXJS_PutArrayElement(v8::Isolate* pIsolate,
                              v8::Local<v8::Array> pArray,
                              unsigned index,
                              v8::Local<v8::Value> pValue) {
  if (pArray.IsEmpty())
    return 0;
  if (pArray->Set(pIsolate->GetCurrentContext(), index, pValue).IsNothing())
    return 0;
  return 1;
}

v8::Local<v8::Value> FXJS_GetArrayElement(v8::Isolate* pIsolate,
                                          v8::Local<v8::Array> pArray,
                                          unsigned index) {
  if (pArray.IsEmpty())
    return v8::Local<v8::Value>();
  v8::Local<v8::Value> val;
  if (!pArray->Get(pIsolate->GetCurrentContext(), index).ToLocal(&val))
    return v8::Local<v8::Value>();
  return val;
}

unsigned FXJS_GetArrayLength(v8::Local<v8::Array> pArray) {
  if (pArray.IsEmpty())
    return 0;
  return pArray->Length();
}

v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, int number) {
  return v8::Int32::New(pIsolate, number);
}

v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, double number) {
  return v8::Number::New(pIsolate, number);
}

v8::Local<v8::Value> FXJS_NewNumber(v8::Isolate* pIsolate, float number) {
  return v8::Number::New(pIsolate, (float)number);
}

v8::Local<v8::Value> FXJS_NewBoolean(v8::Isolate* pIsolate, bool b) {
  return v8::Boolean::New(pIsolate, b);
}

v8::Local<v8::Value> FXJS_NewObject(v8::Isolate* pIsolate,
                                    v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Value>();
  return pObj->Clone();
}

v8::Local<v8::Value> FXJS_NewObject2(v8::Isolate* pIsolate,
                                     v8::Local<v8::Array> pObj) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Value>();
  return pObj->Clone();
}

v8::Local<v8::Value> FXJS_NewString(v8::Isolate* pIsolate,
                                    const wchar_t* string) {
  return FXJS_WSToJSString(pIsolate, string);
}

v8::Local<v8::Value> FXJS_NewNull() {
  return v8::Local<v8::Value>();
}

v8::Local<v8::Value> FXJS_NewDate(v8::Isolate* pIsolate, double d) {
  return v8::Date::New(pIsolate->GetCurrentContext(), d).ToLocalChecked();
}

int FXJS_ToInt32(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0;
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return pValue->ToInt32(context).ToLocalChecked()->Value();
}

bool FXJS_ToBoolean(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return false;
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return pValue->ToBoolean(context).ToLocalChecked()->Value();
}

double FXJS_ToNumber(v8::Isolate* pIsolate, v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0.0;
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return pValue->ToNumber(context).ToLocalChecked()->Value();
}

v8::Local<v8::Object> FXJS_ToObject(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return v8::Local<v8::Object>();
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return pValue->ToObject(context).ToLocalChecked();
}

CFX_WideString FXJS_ToString(v8::Isolate* pIsolate,
                             v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return L"";
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  v8::String::Utf8Value s(pValue->ToString(context).ToLocalChecked());
  return CFX_WideString::FromUTF8(*s, s.length());
}

v8::Local<v8::Array> FXJS_ToArray(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return v8::Local<v8::Array>();
  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  return v8::Local<v8::Array>::Cast(pValue->ToObject(context).ToLocalChecked());
}

void FXJS_ValueCopy(v8::Local<v8::Value>& pTo, v8::Local<v8::Value> pFrom) {
  pTo = pFrom;
}


