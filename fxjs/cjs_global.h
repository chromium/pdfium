// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_GLOBAL_H_
#define FXJS_CJS_GLOBAL_H_

#include <map>
#include <memory>
#include <vector>

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
  static int GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);
  static void DefineAllProperties(CFXJS_Engine* pEngine);

  static void queryprop_static(
      v8::Local<v8::Name> property,
      const v8::PropertyCallbackInfo<v8::Integer>& info);
  static void getprop_static(v8::Local<v8::Name> property,
                             const v8::PropertyCallbackInfo<v8::Value>& info);
  static void putprop_static(v8::Local<v8::Name> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<v8::Value>& info);
  static void delprop_static(v8::Local<v8::Name> property,
                             const v8::PropertyCallbackInfo<v8::Boolean>& info);

  static void setPersistent_static(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  CJS_Global(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime);
  ~CJS_Global() override;

  CJS_Result DelProperty(CJS_Runtime* pRuntime, const wchar_t* propname);

  CJS_Result setPersistent(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Result QueryProperty(const wchar_t* propname);
  CJS_Result GetProperty(CJS_Runtime* pRuntime, const wchar_t* propname);
  CJS_Result SetProperty(CJS_Runtime* pRuntime,
                         const wchar_t* propname,
                         v8::Local<v8::Value> vp);

 private:
  struct JSGlobalData : public CFX_Value {
   public:
    JSGlobalData();
    ~JSGlobalData();

    v8::Global<v8::Object> pData;
    bool bPersistent = false;
    bool bDeleted = false;
  };

  static int ObjDefnID;
  static const JSMethodSpec MethodSpecs[];

  void UpdateGlobalPersistentVariables();
  void CommitGlobalPersisitentVariables(CJS_Runtime* pRuntime);
  void DestroyGlobalPersisitentVariables();
  CJS_Result SetGlobalVariables(const ByteString& propname,
                                CFX_Value::DataType nType,
                                double dData,
                                bool bData,
                                const ByteString& sData,
                                v8::Local<v8::Object> pData,
                                bool bDefaultPersistent);
  void ObjectToArray(CJS_Runtime* pRuntime,
                     v8::Local<v8::Object> pObj,
                     std::vector<std::unique_ptr<CFX_KeyValue>>* pArray);
  void PutObjectProperty(v8::Local<v8::Object> obj, CFX_KeyValue* pData);

  std::map<ByteString, std::unique_ptr<JSGlobalData>> m_MapGlobal;
  WideString m_sFilePath;
  CFX_GlobalData* m_pGlobalData;
  ObservedPtr<CPDFSDK_FormFillEnvironment> m_pFormFillEnv;
};

#endif  // FXJS_CJS_GLOBAL_H_
