// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_GLOBAL_H_
#define FXJS_CJS_GLOBAL_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cfx_keyvalue.h"
#include "fxjs/cjs_object.h"
#include "fxjs/cjs_result.h"

class CFX_GlobalData;

// The CJS_Global object is not the V8 global object (i.e. it is not |this|
// in JavaScript outside of a bound function call). It is a facility for
// sharing data amongst documents and persisting data within a document
// between sessions. It is only partially implemented due to security and
// privacy concerns. It provides access via properties in the usual manner,
// execpt that these are stored on the C++ side rather than in V8 itself.
// It is a static object that is available as "global" property of the V8
// global object and can be manipulated from JavaScript as |global['foo']|
// for example.

class CJS_Global final : public CJS_Object {
 public:
  static uint32_t GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);
  static void DefineAllProperties(CFXJS_Engine* pEngine);

  static v8::Intercepted queryprop_static(
      v8::Local<v8::Name> property,
      const v8::PropertyCallbackInfo<v8::Integer>& info);
  static v8::Intercepted getprop_static(
      v8::Local<v8::Name> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);
  static v8::Intercepted putprop_static(
      v8::Local<v8::Name> property,
      v8::Local<v8::Value> value,
      const v8::PropertyCallbackInfo<void>& info);
  static v8::Intercepted delprop_static(
      v8::Local<v8::Name> property,
      const v8::PropertyCallbackInfo<v8::Boolean>& info);
  static void enumprop_static(const v8::PropertyCallbackInfo<v8::Array>& info);

  static void setPersistent_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  CJS_Global(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime);
  ~CJS_Global() override;

 private:
  struct JSGlobalData : public CFX_Value {
   public:
    JSGlobalData();
    ~JSGlobalData();

    v8::Global<v8::Object> pData;
    bool bPersistent = false;
    bool bDeleted = false;
  };

  static uint32_t ObjDefnID;
  static const JSMethodSpec MethodSpecs[];

  void UpdateGlobalPersistentVariables();
  // TODO(crbug.com/pdfium/926): This method is never called.
  void CommitGlobalPersisitentVariables();
  void DestroyGlobalPersisitentVariables();
  CJS_Result SetGlobalVariables(const ByteString& propname,
                                CFX_Value::DataType nType,
                                double dData,
                                bool bData,
                                const ByteString& sData,
                                v8::Local<v8::Object> pData,
                                bool bDefaultPersistent);
  std::vector<std::unique_ptr<CFX_KeyValue>> ObjectToArray(
      CJS_Runtime* pRuntime,
      v8::Local<v8::Object> pObj);
  void PutObjectProperty(v8::Local<v8::Object> obj, CFX_KeyValue* pData);
  CJS_Result setPersistent(CJS_Runtime* pRuntime,
                           pdfium::span<v8::Local<v8::Value>> params);
  bool HasProperty(const ByteString& propname);
  bool DelProperty(const ByteString& propname);
  CJS_Result GetProperty(CJS_Runtime* pRuntime, const ByteString& propname);
  CJS_Result SetProperty(CJS_Runtime* pRuntime,
                         const ByteString& propname,
                         v8::Local<v8::Value> vp);
  void EnumProperties(CJS_Runtime* pRuntime,
                      const v8::PropertyCallbackInfo<v8::Array>& info);

  std::map<ByteString, std::unique_ptr<JSGlobalData>> m_MapGlobal;
  UnownedPtr<CFX_GlobalData> m_pGlobalData;
};

#endif  // FXJS_CJS_GLOBAL_H_
