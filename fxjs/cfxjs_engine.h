// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

// CFXJS_ENGINE is a layer that makes it easier to define native objects in V8,
// but has no knowledge of PDF-specific native objects. It could in theory be
// used to implement other sets of native objects.

// PDFium code should include this file rather than including V8 headers
// directly.

#ifndef FXJS_CFXJS_ENGINE_H_
#define FXJS_CFXJS_ENGINE_H_

#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/widestring.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/ijs_runtime.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-function-callback.h"
#include "v8/include/v8-persistent-handle.h"
#include "v8/include/v8-template.h"

class CFXJS_ObjDefinition;
class CJS_Object;
class V8TemplateMap;

enum FXJSOBJTYPE {
  FXJSOBJTYPE_DYNAMIC = 0,  // Created by native method and returned to JS.
  FXJSOBJTYPE_STATIC,       // Created by init and hung off of global object.
  FXJSOBJTYPE_GLOBAL,       // The global object itself (may only appear once).
};

class FXJS_PerIsolateData {
 public:
  // Hook for XFA's data, when present.
  class ExtensionIface {
   public:
    virtual ~ExtensionIface() = default;
  };

  ~FXJS_PerIsolateData();

  static void SetUp(v8::Isolate* pIsolate);
  static FXJS_PerIsolateData* Get(v8::Isolate* pIsolate);

  uint32_t CurrentMaxObjDefinitionID() const;
  CFXJS_ObjDefinition* ObjDefinitionForID(uint32_t id) const;
  uint32_t AssignIDForObjDefinition(std::unique_ptr<CFXJS_ObjDefinition> pDefn);
  V8TemplateMap* GetDynamicObjsMap() { return m_pDynamicObjsMap.get(); }
  ExtensionIface* GetExtension() { return m_pExtension.get(); }
  void SetExtension(std::unique_ptr<ExtensionIface> extension) {
    m_pExtension = std::move(extension);
  }

 private:
  explicit FXJS_PerIsolateData(v8::Isolate* pIsolate);

  const wchar_t* const m_Tag;  // Raw, always a literal.
  std::vector<std::unique_ptr<CFXJS_ObjDefinition>> m_ObjectDefnArray;
  std::unique_ptr<V8TemplateMap> m_pDynamicObjsMap;
  std::unique_ptr<ExtensionIface> m_pExtension;
};

void FXJS_Initialize(unsigned int embedderDataSlot, v8::Isolate* pIsolate);
void FXJS_Release();

// Gets the global isolate set by FXJS_Initialize(), or makes a new one each
// time if there is no such isolate. Returns true if a new isolate had to be
// created.
bool FXJS_GetIsolate(v8::Isolate** pResultIsolate);

// Get the global isolate's ref count.
size_t FXJS_GlobalIsolateRefCount();

class CFXJS_Engine : public CFX_V8 {
 public:
  explicit CFXJS_Engine(v8::Isolate* pIsolate);
  ~CFXJS_Engine() override;

  using Constructor = std::function<void(CFXJS_Engine* pEngine,
                                         v8::Local<v8::Object> obj,
                                         v8::Local<v8::Object> proxy)>;
  using Destructor = std::function<void(v8::Local<v8::Object> obj)>;

  static uint32_t GetObjDefnID(v8::Local<v8::Object> pObj);
  static CJS_Object* GetObjectPrivate(v8::Isolate* pIsolate,
                                      v8::Local<v8::Object> pObj);
  static void SetObjectPrivate(v8::Local<v8::Object> pObj,
                               std::unique_ptr<CJS_Object> p);
  static void FreeObjectPrivate(v8::Local<v8::Object> pObj);

  // Always returns a valid (i.e. non-zero), newly-created objDefnID.
  uint32_t DefineObj(const char* sObjName,
                     FXJSOBJTYPE eObjType,
                     Constructor pConstructor,
                     Destructor pDestructor);

  void DefineObjMethod(uint32_t nObjDefnID,
                       const char* sMethodName,
                       v8::FunctionCallback pMethodCall);
  void DefineObjProperty(uint32_t nObjDefnID,
                         const char* sPropName,
                         v8::AccessorGetterCallback pPropGet,
                         v8::AccessorSetterCallback pPropPut);
  void DefineObjAllProperties(
      uint32_t nObjDefnID,
      v8::GenericNamedPropertyQueryCallback pPropQurey,
      v8::GenericNamedPropertyGetterCallback pPropGet,
      v8::GenericNamedPropertySetterCallback pPropPut,
      v8::GenericNamedPropertyDeleterCallback pPropDel,
      v8::GenericNamedPropertyEnumeratorCallback pPropEnum);
  void DefineObjConst(uint32_t nObjDefnID,
                      const char* sConstName,
                      v8::Local<v8::Value> pDefault);
  void DefineGlobalMethod(const char* sMethodName,
                          v8::FunctionCallback pMethodCall);
  void DefineGlobalConst(const wchar_t* sConstName,
                         v8::FunctionCallback pConstGetter);

  // Called after FXJS_Define* calls made.
  void InitializeEngine();
  void ReleaseEngine();

  // Called after FXJS_InitializeEngine call made.
  absl::optional<IJS_Runtime::JS_Error> Execute(const WideString& script);

  v8::Local<v8::Object> GetThisObj();
  v8::Local<v8::Object> NewFXJSBoundObject(uint32_t nObjDefnID,
                                           FXJSOBJTYPE type);
  void Error(const WideString& message);

  v8::Local<v8::Context> GetV8Context();

  v8::Local<v8::Array> GetConstArray(const WideString& name);
  void SetConstArray(const WideString& name, v8::Local<v8::Array> array);

 protected:
  CFXJS_Engine();

 private:
  v8::Global<v8::Context> m_V8Context;
  std::vector<v8::Global<v8::Object>> m_StaticObjects;
  std::map<WideString, v8::Global<v8::Array>> m_ConstArrays;
};

#endif  // FXJS_CFXJS_ENGINE_H_
