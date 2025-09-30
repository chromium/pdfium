// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfxjs_engine.h"

#include <memory>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cfx_v8_array_buffer_allocator.h"
#include "fxjs/cjs_object.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_runtimedata.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-exception.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-message.h"
#include "v8/include/v8-primitive.h"
#include "v8/include/v8-script.h"
#include "v8/include/v8-util.h"

namespace {

unsigned int g_embedderDataSlot = 1u;
v8::Isolate* g_isolate = nullptr;
size_t g_isolate_ref_count = 0;
CFX_V8ArrayBufferAllocator* g_arrayBufferAllocator = nullptr;
v8::Global<v8::ObjectTemplate>* g_DefaultGlobalObjectTemplate = nullptr;

// TODO(pdfium): Define and use type-specific type tags for aligned pointers
// stored in V8 objects. The type tags should not overlap with the ones used by
// Blink, as defined in gin/public/gin_embedders.h.
constexpr v8::EmbedderDataTypeTag kDefaultPDFiumTag = 0;

// Only the address matters, values are for humans debugging. ASLR should
// ensure that these values are unlikely to arise otherwise. Keep these
// wchar_t to prevent the compiler from doing something clever, like
// aligning them on a byte boundary to save space, which would make them
// incompatible for use as V8 aligned pointers.
const wchar_t kPerObjectDataTag[] = L"CFXJS_PerObjectData";
const wchar_t kPerIsolateDataTag[] = L"CFXJS_PerIsolateData";

void* GetAlignedPointerForPerObjectDataTag() {
  return const_cast<void*>(static_cast<const void*>(kPerObjectDataTag));
}

std::pair<int, int> GetLineAndColumnFromError(v8::Local<v8::Message> message,
                                              v8::Local<v8::Context> context) {
  if (message.IsEmpty()) {
    return std::make_pair(-1, -1);
  }
  return std::make_pair(message->GetLineNumber(context).FromMaybe(-1),
                        message->GetStartColumn());
}

}  // namespace

// static
void CFXJS_PerObjectData::SetNewDataInObject(uint32_t nObjDefnID,
                                             v8::Local<v8::Object> pObj) {
  if (pObj->InternalFieldCount() == 2) {
    pObj->SetAlignedPointerInInternalField(
        0, GetAlignedPointerForPerObjectDataTag(),
        kDefaultPDFiumTag);
    pObj->SetAlignedPointerInInternalField(
        1, new CFXJS_PerObjectData(nObjDefnID), kDefaultPDFiumTag);
  }
}

// static
CFXJS_PerObjectData* CFXJS_PerObjectData::GetFromObject(
    v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty()) {
    return nullptr;
  }
  if (!HasInternalFields(pObj)) {
    return nullptr;
  }
  return ExtractFromObject(pObj);
}

//  static
bool CFXJS_PerObjectData::HasInternalFields(v8::Local<v8::Object> pObj) {
  return pObj->InternalFieldCount() == 2 &&
         pObj->GetAlignedPointerFromInternalField(
             0, kDefaultPDFiumTag) ==
             GetAlignedPointerForPerObjectDataTag();
}

//  static
CFXJS_PerObjectData* CFXJS_PerObjectData::ExtractFromObject(
    v8::Local<v8::Object> pObj) {
  return static_cast<CFXJS_PerObjectData*>(
      pObj->GetAlignedPointerFromInternalField(1,
                                               kDefaultPDFiumTag));
}

CFXJS_PerObjectData::CFXJS_PerObjectData(uint32_t nObjDefnID)
    : obj_defn_id_(nObjDefnID) {}

CFXJS_PerObjectData::~CFXJS_PerObjectData() = default;

// Global weak map to save dynamic objects.
class V8TemplateMapTraits final
    : public v8::StdMapTraits<CFXJS_PerObjectData*, v8::Object> {
 public:
  using WeakCallbackDataType = CFXJS_PerObjectData;
  using MapType = v8::
      GlobalValueMap<WeakCallbackDataType*, v8::Object, V8TemplateMapTraits>;

  static const v8::PersistentContainerCallbackType kCallbackType =
      v8::kWeakWithInternalFields;

  static WeakCallbackDataType* WeakCallbackParameter(
      MapType* map,
      WeakCallbackDataType* key,
      v8::Local<v8::Object> value) {
    return key;
  }
  static MapType* MapFromWeakCallbackInfo(
      const v8::WeakCallbackInfo<WeakCallbackDataType>&);
  static WeakCallbackDataType* KeyFromWeakCallbackInfo(
      const v8::WeakCallbackInfo<WeakCallbackDataType>& data) {
    return data.GetParameter();
  }
  static void OnWeakCallback(
      const v8::WeakCallbackInfo<WeakCallbackDataType>& data) {}
  static void DisposeWeak(
      const v8::WeakCallbackInfo<WeakCallbackDataType>& data);
  static void Dispose(v8::Isolate* isolate,
                      v8::Global<v8::Object> value,
                      WeakCallbackDataType* key);
  static void DisposeCallbackData(WeakCallbackDataType* callbackData) {}
};

class V8TemplateMap {
 public:
  using WeakCallbackDataType = CFXJS_PerObjectData;
  using MapType = v8::
      GlobalValueMap<WeakCallbackDataType*, v8::Object, V8TemplateMapTraits>;

  explicit V8TemplateMap(v8::Isolate* isolate) : map_(isolate) {}
  ~V8TemplateMap() = default;

  void SetAndMakeWeak(v8::Local<v8::Object> handle) {
    WeakCallbackDataType* key = CFXJS_PerObjectData::GetFromObject(handle);
    DCHECK(!map_.Contains(key));

    // Inserting an object into a GlobalValueMap with the appropriate traits
    // has the side-effect of making the object weak deep in the guts of V8,
    // and arranges for it to be cleaned up by the methods in the traits.
    map_.Set(key, handle);
  }

  MapType* GetMap() { return &map_; }

 private:
  MapType map_;
};

class CFXJS_ObjDefinition {
 public:
  CFXJS_ObjDefinition(v8::Isolate* isolate,
                      const char* sObjName,
                      FXJSOBJTYPE eObjType,
                      CFXJS_Engine::Constructor pConstructor,
                      CFXJS_Engine::Destructor pDestructor)
      : obj_name_(sObjName),
        obj_type_(eObjType),
        constructor_(pConstructor),
        destructor_(pDestructor),
        isolate_(isolate) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> fn = v8::FunctionTemplate::New(isolate);
    fn->InstanceTemplate()->SetInternalFieldCount(2);
    fn->InstanceTemplate()->SetImmutableProto();
    fn->SetCallHandler(CallHandler, v8::Number::New(isolate, eObjType));
    if (eObjType == FXJSOBJTYPE_GLOBAL) {
      fn->InstanceTemplate()->Set(v8::Symbol::GetToStringTag(isolate),
                                  fxv8::NewStringHelper(isolate, "global"));
    }
    function_template_.Reset(isolate, fn);
    signature_.Reset(isolate, v8::Signature::New(isolate, fn));
  }

  static void CallHandler(const v8::FunctionCallbackInfo<v8::Value>& info) {
    v8::Isolate* isolate = info.GetIsolate();
    if (!info.IsConstructCall()) {
      fxv8::ThrowExceptionHelper(isolate, "illegal constructor");
      return;
    }
    if (info.Data().As<v8::Int32>()->Value() != FXJSOBJTYPE_DYNAMIC) {
      fxv8::ThrowExceptionHelper(isolate, "not a dynamic object");
      return;
    }
    v8::Local<v8::Object> holder = info.This();
    DCHECK_EQ(holder->InternalFieldCount(), 2);
    holder->SetAlignedPointerInInternalField(0, nullptr,
                                             kDefaultPDFiumTag);
    holder->SetAlignedPointerInInternalField(1, nullptr,
                                             kDefaultPDFiumTag);
  }

  FXJSOBJTYPE GetObjType() const { return obj_type_; }
  const char* GetObjName() const { return obj_name_; }
  v8::Isolate* GetIsolate() const { return isolate_; }

  void DefineConst(const char* sConstName, v8::Local<v8::Value> pDefault) {
    GetInstanceTemplate()->Set(GetIsolate(), sConstName, pDefault);
  }

  void DefineProperty(v8::Local<v8::String> sPropName,
                      v8::AccessorNameGetterCallback pPropGet,
                      v8::AccessorNameSetterCallback pPropPut) {
    GetInstanceTemplate()->SetNativeDataProperty(sPropName, pPropGet, pPropPut);
  }

  void DefineMethod(v8::Local<v8::String> sMethodName,
                    v8::FunctionCallback pMethodCall) {
    v8::Local<v8::FunctionTemplate> fun = v8::FunctionTemplate::New(
        GetIsolate(), pMethodCall, v8::Local<v8::Value>(), GetSignature());
    fun->RemovePrototype();
    GetInstanceTemplate()->Set(sMethodName, fun, v8::ReadOnly);
  }

  void DefineAllProperties(v8::NamedPropertyQueryCallback pPropQurey,
                           v8::NamedPropertyGetterCallback pPropGet,
                           v8::NamedPropertySetterCallback pPropPut,
                           v8::NamedPropertyDeleterCallback pPropDel,
                           v8::NamedPropertyEnumeratorCallback pPropEnum) {
    GetInstanceTemplate()->SetHandler(v8::NamedPropertyHandlerConfiguration(
        pPropGet, pPropPut, pPropQurey, pPropDel, pPropEnum,
        v8::Local<v8::Value>(),
        v8::PropertyHandlerFlags::kOnlyInterceptStrings));
  }

  v8::Local<v8::ObjectTemplate> GetInstanceTemplate() {
    v8::EscapableHandleScope scope(GetIsolate());
    v8::Local<v8::FunctionTemplate> function =
        function_template_.Get(GetIsolate());
    return scope.Escape(function->InstanceTemplate());
  }

  v8::Local<v8::Signature> GetSignature() {
    v8::EscapableHandleScope scope(GetIsolate());
    return scope.Escape(signature_.Get(GetIsolate()));
  }

  void RunConstructor(CFXJS_Engine* pEngine, v8::Local<v8::Object> obj) {
    if (constructor_) {
      constructor_(pEngine, obj);
    }
  }

  void RunDestructor(v8::Local<v8::Object> obj) {
    if (destructor_) {
      destructor_(obj);
    }
  }

 private:
  UnownedPtr<const char> const obj_name_;
  const FXJSOBJTYPE obj_type_;
  const CFXJS_Engine::Constructor constructor_;
  const CFXJS_Engine::Destructor destructor_;
  UnownedPtr<v8::Isolate> isolate_;
  v8::Global<v8::FunctionTemplate> function_template_;
  v8::Global<v8::Signature> signature_;
};

static v8::Local<v8::ObjectTemplate> GetGlobalObjectTemplate(
    v8::Isolate* pIsolate) {
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(pIsolate);
  for (uint32_t i = 1; i <= pIsolateData->CurrentMaxObjDefinitionID(); ++i) {
    CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(i);
    if (pObjDef->GetObjType() == FXJSOBJTYPE_GLOBAL) {
      return pObjDef->GetInstanceTemplate();
    }
  }
  if (!g_DefaultGlobalObjectTemplate) {
    v8::Local<v8::ObjectTemplate> hGlobalTemplate =
        v8::ObjectTemplate::New(pIsolate);
    hGlobalTemplate->Set(v8::Symbol::GetToStringTag(pIsolate),
                         fxv8::NewStringHelper(pIsolate, "global"));
    g_DefaultGlobalObjectTemplate =
        new v8::Global<v8::ObjectTemplate>(pIsolate, hGlobalTemplate);
  }
  return g_DefaultGlobalObjectTemplate->Get(pIsolate);
}

void V8TemplateMapTraits::Dispose(v8::Isolate* isolate,
                                  v8::Global<v8::Object> value,
                                  WeakCallbackDataType* key) {
  v8::Local<v8::Object> obj = value.Get(isolate);
  if (obj.IsEmpty()) {
    return;
  }
  uint32_t id = CFXJS_Engine::GetObjDefnID(obj);
  if (id == 0) {
    return;
  }
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(isolate);
  CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(id);
  if (!pObjDef) {
    return;
  }
  pObjDef->RunDestructor(obj);
  CFXJS_Engine::FreePerObjectData(obj);
}

void V8TemplateMapTraits::DisposeWeak(
    const v8::WeakCallbackInfo<WeakCallbackDataType>& data) {
  // TODO(tsepez): this is expected be called during GC.
}

V8TemplateMapTraits::MapType* V8TemplateMapTraits::MapFromWeakCallbackInfo(
    const v8::WeakCallbackInfo<WeakCallbackDataType>& info) {
  auto* pIsolateData = CFXJS_PerIsolateData::Get(info.GetIsolate());
  V8TemplateMap* pObjsMap = pIsolateData->GetDynamicObjsMap();
  return pObjsMap ? pObjsMap->GetMap() : nullptr;
}

void FXJS_Initialize(unsigned int embedderDataSlot, v8::Isolate* pIsolate) {
  if (g_isolate) {
    DCHECK_EQ(g_embedderDataSlot, embedderDataSlot);
    DCHECK_EQ(g_isolate, pIsolate);
    return;
  }
  g_embedderDataSlot = embedderDataSlot;
  g_isolate = pIsolate;
}

void FXJS_Release() {
  DCHECK(!g_isolate || g_isolate_ref_count == 0);
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
  if (!g_arrayBufferAllocator) {
    g_arrayBufferAllocator = new CFX_V8ArrayBufferAllocator();
  }
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = g_arrayBufferAllocator;
  *pResultIsolate = v8::Isolate::New(params);
  return true;
}

size_t FXJS_GlobalIsolateRefCount() {
  return g_isolate_ref_count;
}

// static
void CFXJS_PerIsolateData::SetUp(v8::Isolate* pIsolate) {
  if (!pIsolate->GetData(g_embedderDataSlot)) {
    pIsolate->SetData(g_embedderDataSlot, new CFXJS_PerIsolateData(pIsolate));
  }
}

// static
CFXJS_PerIsolateData* CFXJS_PerIsolateData::Get(v8::Isolate* pIsolate) {
  auto* result =
      static_cast<CFXJS_PerIsolateData*>(pIsolate->GetData(g_embedderDataSlot));
  CHECK(result->tag_ == kPerIsolateDataTag);
  return result;
}

CFXJS_PerIsolateData::CFXJS_PerIsolateData(v8::Isolate* pIsolate)
    : tag_(kPerIsolateDataTag),
      dynamic_objs_map_(std::make_unique<V8TemplateMap>(pIsolate)) {}

CFXJS_PerIsolateData::~CFXJS_PerIsolateData() = default;

uint32_t CFXJS_PerIsolateData::CurrentMaxObjDefinitionID() const {
  return fxcrt::CollectionSize<uint32_t>(object_defn_array_);
}

CFXJS_ObjDefinition* CFXJS_PerIsolateData::ObjDefinitionForID(
    uint32_t id) const {
  return id > 0 && id <= CurrentMaxObjDefinitionID()
             ? object_defn_array_[id - 1].get()
             : nullptr;
}

uint32_t CFXJS_PerIsolateData::AssignIDForObjDefinition(
    std::unique_ptr<CFXJS_ObjDefinition> pDefn) {
  object_defn_array_.push_back(std::move(pDefn));
  return CurrentMaxObjDefinitionID();
}

CFXJS_Engine::CFXJS_Engine() : CFX_V8(nullptr) {}

CFXJS_Engine::CFXJS_Engine(v8::Isolate* pIsolate) : CFX_V8(pIsolate) {}

CFXJS_Engine::~CFXJS_Engine() = default;

// static
uint32_t CFXJS_Engine::GetObjDefnID(v8::Local<v8::Object> pObj) {
  CFXJS_PerObjectData* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  return pData ? pData->GetObjDefnID() : 0;
}

// static
void CFXJS_Engine::SetBinding(v8::Local<v8::Object> pObj,
                              std::unique_ptr<CFXJS_PerObjectData::Binding> p) {
  CFXJS_PerObjectData* pPerObjectData =
      CFXJS_PerObjectData::GetFromObject(pObj);
  if (pPerObjectData) {
    pPerObjectData->SetBinding(std::move(p));
  }
}

// static
void CFXJS_Engine::FreePerObjectData(v8::Local<v8::Object> pObj) {
  CFXJS_PerObjectData* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  pObj->SetAlignedPointerInInternalField(0, nullptr,
                                         kDefaultPDFiumTag);
  pObj->SetAlignedPointerInInternalField(1, nullptr,
                                         kDefaultPDFiumTag);
  delete pData;
}

uint32_t CFXJS_Engine::DefineObj(const char* sObjName,
                                 FXJSOBJTYPE eObjType,
                                 CFXJS_Engine::Constructor pConstructor,
                                 CFXJS_Engine::Destructor pDestructor) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_PerIsolateData::SetUp(GetIsolate());
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  return pIsolateData->AssignIDForObjDefinition(
      std::make_unique<CFXJS_ObjDefinition>(GetIsolate(), sObjName, eObjType,
                                            pConstructor, pDestructor));
}

void CFXJS_Engine::DefineObjMethod(uint32_t nObjDefnID,
                                   const char* sMethodName,
                                   v8::FunctionCallback pMethodCall) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(nObjDefnID);
  pObjDef->DefineMethod(NewString(sMethodName), pMethodCall);
}

void CFXJS_Engine::DefineObjProperty(uint32_t nObjDefnID,
                                     const char* sPropName,
                                     v8::AccessorNameGetterCallback pPropGet,
                                     v8::AccessorNameSetterCallback pPropPut) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(nObjDefnID);
  pObjDef->DefineProperty(NewString(sPropName), pPropGet, pPropPut);
}

void CFXJS_Engine::DefineObjAllProperties(
    uint32_t nObjDefnID,
    v8::NamedPropertyQueryCallback pPropQurey,
    v8::NamedPropertyGetterCallback pPropGet,
    v8::NamedPropertySetterCallback pPropPut,
    v8::NamedPropertyDeleterCallback pPropDel,
    v8::NamedPropertyEnumeratorCallback pPropEnum) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(nObjDefnID);
  pObjDef->DefineAllProperties(pPropQurey, pPropGet, pPropPut, pPropDel,
                               pPropEnum);
}

void CFXJS_Engine::DefineObjConst(uint32_t nObjDefnID,
                                  const char* sConstName,
                                  v8::Local<v8::Value> pDefault) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(nObjDefnID);
  pObjDef->DefineConst(sConstName, pDefault);
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
  if (GetIsolate() == g_isolate) {
    ++g_isolate_ref_count;
  }

  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());

  // This has to happen before we call GetGlobalObjectTemplate because that
  // method gets the PerIsolateData from GetIsolate().
  CFXJS_PerIsolateData::SetUp(GetIsolate());

  v8::Local<v8::Context> v8Context = v8::Context::New(
      GetIsolate(), nullptr, GetGlobalObjectTemplate(GetIsolate()));

  // May not have the internal fields when called from tests, so clear these
  // in case we don't process a FXJSOBJTYPE_GLOBAL below.
  v8::Local<v8::Object> pThis = v8Context->Global();
  if (pThis->InternalFieldCount() == 2) {
    pThis->SetAlignedPointerInInternalField(0, nullptr,
                                            kDefaultPDFiumTag);
    pThis->SetAlignedPointerInInternalField(1, nullptr,
                                            kDefaultPDFiumTag);
  }

  v8::Context::Scope context_scope(v8Context);
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  uint32_t maxID = pIsolateData->CurrentMaxObjDefinitionID();
  static_objects_.resize(maxID + 1);
  for (uint32_t i = 1; i <= maxID; ++i) {
    CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(i);
    if (pObjDef->GetObjType() == FXJSOBJTYPE_GLOBAL) {
      CFXJS_PerObjectData::SetNewDataInObject(i, pThis);
      pObjDef->RunConstructor(this, pThis);

    } else if (pObjDef->GetObjType() == FXJSOBJTYPE_STATIC) {
      v8::Local<v8::String> pObjName = NewString(pObjDef->GetObjName());
      v8::Local<v8::Object> obj = NewFXJSBoundObject(i, FXJSOBJTYPE_STATIC);
      if (!obj.IsEmpty()) {
        v8Context->Global()->Set(v8Context, pObjName, obj).FromJust();
        static_objects_[i] = v8::Global<v8::Object>(GetIsolate(), obj);
      }
    }
  }

  v8_context_.Reset(GetIsolate(), v8Context);
}

void CFXJS_Engine::ReleaseEngine() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::HandleScope handle_scope(GetIsolate());
  v8::Local<v8::Context> context = GetV8Context();
  v8::Context::Scope context_scope(context);
  CFXJS_PerIsolateData* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
  if (!pIsolateData) {
    return;
  }

  const_arrays_.clear();

  for (uint32_t i = 1; i <= pIsolateData->CurrentMaxObjDefinitionID(); ++i) {
    CFXJS_ObjDefinition* pObjDef = pIsolateData->ObjDefinitionForID(i);
    v8::Local<v8::Object> pObj;
    if (pObjDef->GetObjType() == FXJSOBJTYPE_GLOBAL) {
      pObj = context->Global();
    } else if (!static_objects_[i].IsEmpty()) {
      pObj = v8::Local<v8::Object>::New(GetIsolate(), static_objects_[i]);
      static_objects_[i].Reset();
    }
    if (!pObj.IsEmpty()) {
      pObjDef->RunDestructor(pObj);
      FreePerObjectData(pObj);
    }
  }

  v8_context_.Reset();

  if (GetIsolate() == g_isolate && --g_isolate_ref_count > 0) {
    return;
  }

  delete pIsolateData;
  GetIsolate()->SetData(g_embedderDataSlot, nullptr);
}

std::optional<IJS_Runtime::JS_Error> CFXJS_Engine::Execute(
    const WideString& script) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::TryCatch try_catch(GetIsolate());
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  v8::Local<v8::Script> compiled_script;
  if (!v8::Script::Compile(context, NewString(script.AsStringView()))
           .ToLocal(&compiled_script)) {
    v8::String::Utf8Value error(GetIsolate(), try_catch.Exception());
    v8::Local<v8::Message> msg = try_catch.Message();
    auto [line, column] = GetLineAndColumnFromError(msg, context);
    return IJS_Runtime::JS_Error(line, column, WideString::FromUTF8(*error));
  }

  v8::Local<v8::Value> result;
  if (!compiled_script->Run(context).ToLocal(&result)) {
    v8::String::Utf8Value error(GetIsolate(), try_catch.Exception());
    auto msg = try_catch.Message();
    auto [line, column] = GetLineAndColumnFromError(msg, context);
    return IJS_Runtime::JS_Error(line, column, WideString::FromUTF8(*error));
  }
  return std::nullopt;
}

v8::Local<v8::Object> CFXJS_Engine::NewFXJSBoundObject(uint32_t nObjDefnID,
                                                       FXJSOBJTYPE type) {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  CFXJS_PerIsolateData* pData = CFXJS_PerIsolateData::Get(GetIsolate());
  if (!pData) {
    return v8::Local<v8::Object>();
  }

  CFXJS_ObjDefinition* pObjDef = pData->ObjDefinitionForID(nObjDefnID);
  if (!pObjDef) {
    return v8::Local<v8::Object>();
  }

  v8::Local<v8::Object> obj;
  if (!pObjDef->GetInstanceTemplate()->NewInstance(context).ToLocal(&obj)) {
    return v8::Local<v8::Object>();
  }

  CFXJS_PerObjectData::SetNewDataInObject(nObjDefnID, obj);
  pObjDef->RunConstructor(this, obj);
  if (type == FXJSOBJTYPE_DYNAMIC) {
    auto* pIsolateData = CFXJS_PerIsolateData::Get(GetIsolate());
    V8TemplateMap* pObjsMap = pIsolateData->GetDynamicObjsMap();
    if (pObjsMap) {
      pObjsMap->SetAndMakeWeak(obj);
    }
  }
  return obj;
}

v8::Local<v8::Object> CFXJS_Engine::GetThisObj() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  if (!CFXJS_PerIsolateData::Get(GetIsolate())) {
    return v8::Local<v8::Object>();
  }

  // Return the global object.
  v8::Local<v8::Context> context = GetIsolate()->GetCurrentContext();
  return context->Global();
}

void CFXJS_Engine::Error(const WideString& message) {
  fxv8::ThrowExceptionHelper(GetIsolate(), message.AsStringView());
}

v8::Local<v8::Context> CFXJS_Engine::GetV8Context() {
  return v8::Local<v8::Context>::New(GetIsolate(), v8_context_);
}

// static
CFXJS_PerObjectData::Binding* CFXJS_Engine::GetBinding(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObj) {
  auto* pData = CFXJS_PerObjectData::GetFromObject(pObj);
  return pData ? pData->GetBinding() : nullptr;
}

v8::Local<v8::Array> CFXJS_Engine::GetConstArray(const WideString& name) {
  return v8::Local<v8::Array>::New(GetIsolate(), const_arrays_[name]);
}

void CFXJS_Engine::SetConstArray(const WideString& name,
                                 v8::Local<v8::Array> array) {
  const_arrays_[name] = v8::Global<v8::Array>(GetIsolate(), array);
}
