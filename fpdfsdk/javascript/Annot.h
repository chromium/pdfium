// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_ANNOT_H_
#define FPDFSDK_JAVASCRIPT_ANNOT_H_

#include <memory>

#include "fpdfsdk/cpdfsdk_baannot.h"
#include "fpdfsdk/javascript/JS_Define.h"

class Annot : public CJS_EmbedObj {
 public:
  explicit Annot(CJS_Object* pJSObject);
  ~Annot() override;

  bool get_hidden(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_hidden(CJS_Runtime* pRuntime,
                  const CJS_PropValue& vp,
                  WideString* sError);

  bool get_name(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_name(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_type(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_type(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  void SetSDKAnnot(CPDFSDK_BAAnnot* annot);

 private:
  CPDFSDK_Annot::ObservedPtr m_pAnnot;
};

class CJS_Annot : public CJS_Object {
 public:
  explicit CJS_Annot(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Annot() override {}

  DECLARE_JS_CLASS();
  JS_STATIC_PROP(hidden, hidden, Annot);
  JS_STATIC_PROP(name, name, Annot);
  JS_STATIC_PROP(type, type, Annot);
};

#endif  // FPDFSDK_JAVASCRIPT_ANNOT_H_
