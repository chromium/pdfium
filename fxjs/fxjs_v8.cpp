// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/fxjs_v8.h"

#include <vector>

#include "third_party/base/allocator/partition_allocator/partition_alloc.h"

// Keep this consistent with the values defined in gin/public/context_holder.h
// (without actually requiring a dependency on gin itself for the standalone
// embedders of PDFIum). The value we want to use is:
//   kPerContextDataStartIndex + kEmbedderPDFium, which is 3.
static const unsigned int kPerContextDataIndex = 3u;
static unsigned int g_embedderDataSlot = 1u;
static v8::Isolate* g_isolate = nullptr;
static size_t g_isolate_ref_count = 0;
static FXJS_ArrayBufferAllocator* g_arrayBufferAllocator = nullptr;
static v8::Global<v8::ObjectTemplate>* g_DefaultGlobalObjectTemplate = nullptr;
static wchar_t kPerObjectDataTag[] = L"CFXJS_PerObjectData";

class CFXJS_PerObjectData {
 public:
  explicit CFXJS_PerObjectData(int nObjDefID)
      : m_ObjDefID(nObjDefID), m_pPrivate(nullptr) {}

  static void SetInObject(CFXJS_PerObjectData* pData,
                          v8::Local<v8::Object> pObj) {
    if (pObj->InternalFieldCount() == 2) {
      pObj->SetAlignedPointerInInternalField(0, pData);
      pObj->SetAlignedPointerInInternalField(
          1, static_cast<void*>(kPerObjectDataTag));
    }
  }

  static CFXJS_PerObjectData* GetFromObject(v8::Local<v8::Object> pObj) {
    if (pObj.IsEmpty() || pObj->InternalFieldCount() != 2 ||
        pObj->GetAlignedPointerFromInternalField(1) !=
            static_cast<void*>(kPerObjectDataTag)) {
      return nullptr;
    }
    return static_cast<CFXJS_PerObjectData*>(
        pObj->GetAlignedPointerFromInternalField(0));
  }

  const int m_ObjDefID;
  void* m_pPrivate;
};

class CFXJS_ObjDefinition {
 public:
  static int MaxID(v8::Isolate* pIsolate) {
    return FXJS_PerIsolateData::Get(pIsolate)->m_ObjectDefnArray.size();
  }

  static CFXJS_ObjDefinition* ForID(v8::Isolate* pIsolate, int id) {
    // Note: GetAt() halts if out-of-range even in release builds.
    return FXJS_PerIsolateData::Get(pIsolate)->m_ObjectDefnArray[id].get();
  }

  CFXJS_ObjDefinition(v8::Isolate* isolate,
                      const char* sObjName,
                      FXJSOBJTYPE eObjType,
                      CFXJS_Engine::Constructor pConstructor,
                      CFXJS_Engine::Destructor pDestructor)
      : m_ObjName(sObjName),
        m_ObjType(eObjType),
        m_pConstructor(pConstructor),
        m_pDestructor(pDestructor),
        m_pIsolate(isolate) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

    v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(isolate);
    fun->InstanceTemplate()->SetInternalFieldCount(2);
    fun->SetCallHandler([](const v8::FunctionCallbackInfo<v8::Value>& info) {
      v8::Local<v8::Object> holder = info.Holder();
      ASSERT(holder->InternalFieldCount() == 2);
      holder->SetAlignedPointerInInternalField(0, nullptr);
      holder->SetAlignedPointerInInternalField(1, nullptr);
    });
    if (eObjType == FXJSOBJTYPE_GLOBAL) {
      fun->InstanceTemplate()->Set(
          v8::Symbol::GetToStringTag(isolate),
          v8::String::NewFromUtf8(isolate, "global", v8::NewStringType::kNormal)
              .ToLocalChecked());
    }
    m_FunctionTemplate.Reset(isolate, fun);

    v8::Local<v8::Signature> sig = v8::Signature::New(isolate, fun);
    m_Signature.Reset(isolate, sig);
  }

  int AssignID() {
    FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(m_pIsolate);
    pData->m_ObjectDefnArray.emplace_back(this);
    return pData->m_ObjectDefnArray.size() - 1;
  }

  v8::Local<v8::ObjectTemplate> GetInstanceTemplate() {
    v8::EscapableHandleScope scope(m_pIsolate);
    v8::Local<v8::FunctionTemplate> function =
        m_FunctionTemplate.Get(m_pIsolate);
    return scope.Escape(function->InstanceTemplate());
  }

  v8::Local<v8::Signature> GetSignature() {
    v8::EscapableHandleScope scope(m_pIsolate);
    return scope.Escape(m_Signature.Get(m_pIsolate));
  }

  const char* const m_ObjName;
  const FXJSOBJTYPE m_ObjType;
  const CFXJS_Engine::Constructor m_pConstructor;
  const CFXJS_Engine::Destructor m_pDestructor;

  v8::Isolate* m_pIsolate;
  v8::Global<v8::FunctionTemplate> m_FunctionTemplate;
  v8::Global<v8::Signature> m_Signature;
};

static v8::Local<v8::ObjectTemplate> GetGlobalObjectTemplate(
    v8::Isolate* pIsolate) {
  int maxID = CFXJS_ObjDefinition::MaxID(pIsolate);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(pIsolate, i);
    if (pObjDef->m_ObjType == FXJSOBJTYPE_GLOBAL)
      return pObjDef->GetInstanceTemplate();
  }
  if (!g_DefaultGlobalObjectTemplate) {
    v8::Local<v8::ObjectTemplate> hGlobalTemplate =
        v8::ObjectTemplate::New(pIsolate);
    hGlobalTemplate->Set(
        v8::Symbol::GetToStringTag(pIsolate),
        v8::String::NewFromUtf8(pIsolate, "global", v8::NewStringType::kNormal)
            .ToLocalChecked());
    g_DefaultGlobalObjectTemplate =
        new v8::Global<v8::ObjectTemplate>(pIsolate, hGlobalTemplate);
  }
  return g_DefaultGlobalObjectTemplate->Get(pIsolate);
}

void* FXJS_ArrayBufferAllocator::Allocate(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  void* p = AllocateUninitialized(length);
  if (p)
    memset(p, 0, length);
  return p;
}

void* FXJS_ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return pdfium::base::PartitionAllocGeneric(
      gArrayBufferPartitionAllocator.root(), length, "FXJS_ArrayBuffer");
}

void FXJS_ArrayBufferAllocator::Free(void* data, size_t length) {
  pdfium::base::PartitionFreeGeneric(gArrayBufferPartitionAllocator.root(),
                                     data);
}

void V8TemplateMapTraits::Dispose(v8::Isolate* isolate,
                                  v8::Global<v8::Object> value,
                                  void* key) {
  v8::Local<v8::Object> obj = value.Get(isolate);
  if (obj.IsEmpty())
    return;
  int id = CFXJS_Engine::GetObjDefnID(obj);
  if (id == -1)
    return;
  CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(isolate, id);
  if (!pObjDef)
    return;
  if (pObjDef->m_pDestructor) {
    pObjDef->m_pDestructor(CFXJS_Engine::CurrentEngineFromIsolate(isolate),
                           obj);
  }
  CFXJS_Engine::FreeObjectPrivate(obj);
}

V8TemplateMapTraits::MapType* V8TemplateMapTraits::MapFromWeakCallbackInfo(
    const v8::WeakCallbackInfo<WeakCallbackDataType>& data) {
  V8TemplateMap* pMap =
      (FXJS_PerIsolateData::Get(data.GetIsolate()))->m_pDynamicObjsMap.get();
  return pMap ? &pMap->m_map : nullptr;
}

void FXJS_Initialize(unsigned int embedderDataSlot, v8::Isolate* pIsolate) {
  if (g_isolate) {
    ASSERT(g_embedderDataSlot == embedderDataSlot);
    ASSERT(g_isolate == pIsolate);
    return;
  }
  g_embedderDataSlot = embedderDataSlot;
  g_isolate = pIsolate;
}

void FXJS_Release() {
  ASSERT(!g_isolate || g_isolate_ref_count == 0);
  delete g_DefaultGlobalObjectTemplate;
  g_DefaultGlobalObjectTemplate = nullptr;
  g_isolate = nullptr;

  delete g_arrayBufferAllocator;
  g_arrayBufferAllocator = nullptr;
}

bool FXJS_GetIsolate(v8::Isolate** pResultIsolate) {
  if (g_isolate) {
    *pResultIsolate = g_isolate;
    return false;
  }
  // Provide backwards compatibility when no external isolate.
  if (!g_arrayBufferAllocator)
    g_arrayBufferAllocator = new FXJS_ArrayBufferAllocator();
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = g_arrayBufferAllocator;
  *pResultIsolate = v8::Isolate::New(params);
  return true;
}

size_t FXJS_GlobalIsolateRefCount() {
  return g_isolate_ref_count;
}

V8TemplateMap::V8TemplateMap(v8::Isolate* isolate) : m_map(isolate) {}

V8TemplateMap::~V8TemplateMap() {}

void V8TemplateMap::set(void* key, v8::Local<v8::Object> handle) {
  ASSERT(!m_map.Contains(key));
  m_map.Set(key, handle);
}

FXJS_PerIsolateData::~FXJS_PerIsolateData() {}

// static
void FXJS_PerIsolateData::SetUp(v8::Isolate* pIsolate) {
  if (!pIsolate->GetData(g_embedderDataSlot))
    pIsolate->SetData(g_embedderDataSlot, new FXJS_PerIsolateData(pIsolate));
}

// static
FXJS_PerIsolateData* FXJS_PerIsolateData::Get(v8::Isolate* pIsolate) {
  return static_cast<FXJS_PerIsolateData*>(
      pIsolate->GetData(g_embedderDataSlot));
}

FXJS_PerIsolateData::FXJS_PerIsolateData(v8::Isolate* pIsolate)
    : m_pDynamicObjsMap(new V8TemplateMap(pIsolate)) {}

CFXJS_Engine::CFXJS_Engine() : CJS_V8(nullptr) {}

CFXJS_Engine::CFXJS_Engine(v8::Isolate* pIsolate) : CJS_V8(pIsolate) {}

CFXJS_Engine::~CFXJS_Engine() = default;

// static
CFXJS_Engine* CFXJS_Engine::CurrentEngineFromIsolate(v8::Isolate* pIsolate) {
  return static_cast<CFXJS_Engine*>(
      pIsolate->GetCurrentContext()->GetAlignedPointerFromEmbedderData(
          kPerContextDataIndex));
}

// static
int CFXJS_Engine::GetObjDefnID(v8::Local<v8::Object> pObj) {
  CFXJS_PerObjectData* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  return pData ? pData->m_ObjDefID : -1;
}

// static
void CFXJS_Engine::FreeObjectPrivate(void* pPerObjectData) {
  delete static_cast<CFXJS_PerObjectData*>(pPerObjectData);
}

// static
void CFXJS_Engine::FreeObjectPrivate(v8::Local<v8::Object> pObj) {
  CFXJS_PerObjectData* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  pObj->SetAlignedPointerInInternalField(0, nullptr);
  pObj->SetAlignedPointerInInternalField(1, nullptr);
  delete pData;
}

int CFXJS_Engine::DefineObj(const char* sObjName,
                            FXJSOBJTYPE eObjType,
                            CFXJS_Engine::Constructor pConstructor,
                            CFXJS_Engine::Destructor pDestructor) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  FXJS_PerIsolateData::SetUp(GetIsolate());
  CFXJS_ObjDefinition* pObjDef = new CFXJS_ObjDefinition(
      GetIsolate(), sObjName, eObjType, pConstructor, pDestructor);
  return pObjDef->AssignID();
}

void CFXJS_Engine::DefineObjMethod(int nObjDefnID,
                                   const char* sMethodName,
                                   v8::FunctionCallback pMethodCall) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(GetIsolate(), nObjDefnID);
  v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(
      GetIsolate(), pMethodCall, v8::Local<v8::Value>(),
      pObjDef->GetSignature());
  fun->RemovePrototype();
  pObjDef->GetInstanceTemplate()->Set(NewString(sMethodName), fun,
                                      v8::ReadOnly);
}

void CFXJS_Engine::DefineObjProperty(int nObjDefnID,
                                     const char* sPropName,
                                     v8::AccessorGetterCallback pPropGet,
                                     v8::AccessorSetterCallback pPropPut) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(GetIsolate(), nObjDefnID);
  pObjDef->GetInstanceTemplate()->SetAccessor(NewString(sPropName), pPropGet,
                                              pPropPut);
}

void CFXJS_Engine::DefineObjAllProperties(
    int nObjDefnID,
    v8::NamedPropertyQueryCallback pPropQurey,
    v8::NamedPropertyGetterCallback pPropGet,
    v8::NamedPropertySetterCallback pPropPut,
    v8::NamedPropertyDeleterCallback pPropDel) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(GetIsolate(), nObjDefnID);
  pObjDef->GetInstanceTemplate()->SetNamedPropertyHandler(pPropGet, pPropPut,
                                                          pPropQurey, pPropDel);
}

void CFXJS_Engine::DefineObjConst(int nObjDefnID,
                                  const char* sConstName,
                                  v8::Local<v8::Value> pDefault) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(GetIsolate(), nObjDefnID);
  pObjDef->GetInstanceTemplate()->Set(GetIsolate(), sConstName, pDefault);
}

void CFXJS_Engine::DefineGlobalMethod(const char* sMethodName,
                                      v8::FunctionCallback pMethodCall) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::FunctionTemplate> fun =
      v8::FunctionTemplate::New(GetIsolate(), pMethodCall);
  fun->RemovePrototype();
  GetGlobalObjectTemplate(GetIsolate())
      ->Set(NewString(sMethodName), fun, v8::ReadOnly);
}

void CFXJS_Engine::DefineGlobalConst(const wchar_t* sConstName,
                                     v8::FunctionCallback pConstGetter) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::FunctionTemplate> fun =
      v8::FunctionTemplate::New(GetIsolate(), pConstGetter);
  fun->RemovePrototype();
  GetGlobalObjectTemplate(GetIsolate())
      ->SetAccessorProperty(NewString(sConstName), fun);
}

void CFXJS_Engine::InitializeEngine() {
  if (GetIsolate() == g_isolate)
    ++g_isolate_ref_count;

  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());

  // This has to happen before we call GetGlobalObjectTemplate because that
  // method gets the PerIsolateData from GetIsolate().
  FXJS_PerIsolateData::SetUp(GetIsolate());

  v8::Local<v8::Context> v8Context = v8::Context::New(
      GetIsolate(), nullptr, GetGlobalObjectTemplate(GetIsolate()));
  v8::Context::Scope context_scope(v8Context);

  v8Context->SetAlignedPointerInEmbedderData(kPerContextDataIndex, this);

  int maxID = CFXJS_ObjDefinition::MaxID(GetIsolate());
  m_StaticObjects.resize(maxID + 1);
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(GetIsolate(), i);
    if (pObjDef->m_ObjType == FXJSOBJTYPE_GLOBAL) {
      CFXJS_PerObjectData::SetInObject(new CFXJS_PerObjectData(i),
                                       v8Context->Global()
                                           ->GetPrototype()
                                           ->ToObject(v8Context)
                                           .ToLocalChecked());
      if (pObjDef->m_pConstructor) {
        pObjDef->m_pConstructor(this, v8Context->Global()
                                          ->GetPrototype()
                                          ->ToObject(v8Context)
                                          .ToLocalChecked());
      }
    } else if (pObjDef->m_ObjType == FXJSOBJTYPE_STATIC) {
      v8::Local<v8::String> pObjName = NewString(pObjDef->m_ObjName);
      v8::Local<v8::Object> obj = NewFxDynamicObj(i, true);
      if (!obj.IsEmpty()) {
        v8Context->Global()->Set(v8Context, pObjName, obj).FromJust();
        m_StaticObjects[i] = new v8::Global<v8::Object>(GetIsolate(), obj);
      } else {
        m_StaticObjects[i] = nullptr;
      }
    }
  }
  ResetPersistentContext(v8Context);
}

void CFXJS_Engine::ReleaseEngine() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::Context> context = NewLocalContext();
  v8::Context::Scope context_scope(context);
  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(GetIsolate());
  if (!pData)
    return;

  ClearConstArray();

  int maxID = CFXJS_ObjDefinition::MaxID(GetIsolate());
  for (int i = 0; i < maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = CFXJS_ObjDefinition::ForID(GetIsolate(), i);
    v8::Local<v8::Object> pObj;
    if (pObjDef->m_ObjType == FXJSOBJTYPE_GLOBAL) {
      pObj =
          context->Global()->GetPrototype()->ToObject(context).ToLocalChecked();
    } else if (m_StaticObjects[i] && !m_StaticObjects[i]->IsEmpty()) {
      pObj = v8::Local<v8::Object>::New(GetIsolate(), *m_StaticObjects[i]);
      delete m_StaticObjects[i];
      m_StaticObjects[i] = nullptr;
    }

    if (!pObj.IsEmpty()) {
      if (pObjDef->m_pDestructor)
        pObjDef->m_pDestructor(this, pObj);
      FreeObjectPrivate(pObj);
    }
  }

  ReleasePersistentContext();

  if (GetIsolate() == g_isolate && --g_isolate_ref_count > 0)
    return;

  delete pData;
  GetIsolate()->SetData(g_embedderDataSlot, nullptr);
}

int CFXJS_Engine::Execute(const WideString& script, FXJSErr* pError) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::TryCatch try_catch(GetIsolate());
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  v8::Local<v8::Script> compiled_script;
  if (!v8::Script::Compile(context, NewString(script.AsStringView()))
           .ToLocal(&compiled_script)) {
    v8::String::Utf8Value error(GetIsolate(), try_catch.Exception());
    // TODO(tsepez): return error via pError->message.
    return -1;
  }

  v8::Local<v8::Value> result;
  if (!compiled_script->Run(context).ToLocal(&result)) {
    v8::String::Utf8Value error(GetIsolate(), try_catch.Exception());
    // TODO(tsepez): return error via pError->message.
    return -1;
  }
  return 0;
}

v8::Local<v8::Object> CFXJS_Engine::NewFxDynamicObj(int nObjDefnID,
                                                    bool bStatic) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  if (nObjDefnID == -1) {
    v8::Local<v8::ObjectTemplate> objTempl =
        v8::ObjectTemplate::New(GetIsolate());
    v8::Local<v8::Object> obj;
    if (!objTempl->NewInstance(context).ToLocal(&obj))
      return v8::Local<v8::Object>();
    return obj;
  }

  FXJS_PerIsolateData* pData = FXJS_PerIsolateData::Get(GetIsolate());
  if (!pData)
    return v8::Local<v8::Object>();

  if (nObjDefnID < 0 || nObjDefnID >= CFXJS_ObjDefinition::MaxID(GetIsolate()))
    return v8::Local<v8::Object>();

  CFXJS_ObjDefinition* pObjDef =
      CFXJS_ObjDefinition::ForID(GetIsolate(), nObjDefnID);
  v8::Local<v8::Object> obj;
  if (!pObjDef->GetInstanceTemplate()->NewInstance(context).ToLocal(&obj))
    return v8::Local<v8::Object>();

  CFXJS_PerObjectData* pObjData = new CFXJS_PerObjectData(nObjDefnID);
  CFXJS_PerObjectData::SetInObject(pObjData, obj);
  if (pObjDef->m_pConstructor)
    pObjDef->m_pConstructor(this, obj);

  if (!bStatic && FXJS_PerIsolateData::Get(GetIsolate())->m_pDynamicObjsMap)
    FXJS_PerIsolateData::Get(GetIsolate())
        ->m_pDynamicObjsMap->set(pObjData, obj);

  return obj;
}

v8::Local<v8::Object> CFXJS_Engine::GetThisObj() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  if (!FXJS_PerIsolateData::Get(GetIsolate()))
    return v8::Local<v8::Object>();

  // Return the global object.
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  return context->Global()->GetPrototype()->ToObject(context).ToLocalChecked();
}

void CFXJS_Engine::Error(const WideString& message) {
  GetIsolate()->ThrowException(NewString(message.AsStringView()));
}

void CFXJS_Engine::SetObjectPrivate(v8::Local<v8::Object> pObj, void* p) {
  CFXJS_PerObjectData* pPerObjectData =
      CFXJS_PerObjectData::GetFromObject(pObj);
  if (!pPerObjectData)
    return;
  pPerObjectData->m_pPrivate = p;
}

void* CFXJS_Engine::GetObjectPrivate(v8::Local<v8::Object> pObj) {
  CFXJS_PerObjectData* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  if (!pData && !pObj.IsEmpty()) {
    // It could be a global proxy object.
    v8::Local<v8::Value> v = pObj->GetPrototype();
    v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
    if (v->IsObject()) {
      pData = CFXJS_PerObjectData::GetFromObject(
          v->ToObject(context).ToLocalChecked());
    }
  }
  return pData ? pData->m_pPrivate : nullptr;
}
