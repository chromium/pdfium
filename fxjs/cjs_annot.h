// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_ANNOT_H_
#define FXJS_CJS_ANNOT_H_

#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fxjs/JS_Define.h"

class Annot : public CJS_EmbedObj {
 public:
  explicit Annot(CJS_Object* pJSObject);
  ~Annot() override;

  CJS_Return get_hidden(CJS_Runtime* pRuntime);
  CJS_Return set_hidden(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_name(CJS_Runtime* pRuntime);
  CJS_Return set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_type(CJS_Runtime* pRuntime);
  CJS_Return set_type(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  void SetSDKAnnot(CPDFSDK_BAAnnot* annot);

 private:
  CPDFSDK_Annot::ObservedPtr m_pAnnot;
};

class CJS_Annot : public CJS_Object {
 public:
  static int GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_Annot(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Annot() override {}

  JS_STATIC_PROP(hidden, hidden, Annot);
  JS_STATIC_PROP(name, name, Annot);
  JS_STATIC_PROP(type, type, Annot);

 private:
  static int ObjDefnID;
  static const JSPropertySpec PropertySpecs[];
};

#endif  // FXJS_CJS_ANNOT_H_
