// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_GLOBAL_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_GLOBAL_H_

#include <map>

#include "JS_Define.h"

class CJS_GlobalData;
class CJS_GlobalVariableArray;
class CJS_KeyValue;

struct JSGlobalData {
  JSGlobalData() {
    nType = 0;
    dData = 0;
    bData = FALSE;
    sData = "";
    bPersistent = FALSE;
    bDeleted = FALSE;
  }

  ~JSGlobalData() { pData.Reset(); }
  int nType;  // 0:int 1:bool 2:string 3:obj
  double dData;
  bool bData;
  CFX_ByteString sData;
  v8::Global<v8::Object> pData;
  bool bPersistent;
  bool bDeleted;
};

class JSGlobalAlternate : public CJS_EmbedObj {
 public:
  JSGlobalAlternate(CJS_Object* pJSObject);
  ~JSGlobalAlternate() override;

  FX_BOOL setPersistent(IFXJS_Context* cc,
                        const CJS_Parameters& params,
                        CJS_Value& vRet,
                        CFX_WideString& sError);
  FX_BOOL QueryProperty(const FX_WCHAR* propname);
  FX_BOOL DoProperty(IFXJS_Context* cc,
                     const FX_WCHAR* propname,
                     CJS_PropValue& vp,
                     CFX_WideString& sError);
  FX_BOOL DelProperty(IFXJS_Context* cc,
                      const FX_WCHAR* propname,
                      CFX_WideString& sError);
  void Initial(CPDFDoc_Environment* pApp);

 private:
  void UpdateGlobalPersistentVariables();
  void CommitGlobalPersisitentVariables();
  void DestroyGlobalPersisitentVariables();
  FX_BOOL SetGlobalVariables(const FX_CHAR* propname,
                             int nType,
                             double dData,
                             bool bData,
                             const CFX_ByteString& sData,
                             JSObject pData,
                             bool bDefaultPersistent);

  void ObjectToArray(v8::Local<v8::Object> pObj,
                     CJS_GlobalVariableArray& array);
  void PutObjectProperty(v8::Local<v8::Object> obj, CJS_KeyValue* pData);

 private:
  std::map<CFX_ByteString, JSGlobalData*> m_mapGlobal;
  CFX_WideString m_sFilePath;
  CJS_GlobalData* m_pGlobalData;
  CPDFDoc_Environment* m_pApp;
};

class CJS_Global : public CJS_Object {
 public:
  explicit CJS_Global(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Global() override {}

  // CJS_Object
  FX_BOOL InitInstance(IFXJS_Context* cc) override;

  DECLARE_SPECIAL_JS_CLASS(CJS_Global);

  JS_SPECIAL_STATIC_METHOD(setPersistent, JSGlobalAlternate, global);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_GLOBAL_H_
