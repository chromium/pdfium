// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_ICON_H_
#define FXJS_CJS_ICON_H_

#include "fxjs/cjs_object.h"
#include "fxjs/js_define.h"

class CJS_Icon final : public CJS_Object {
 public:
  static uint32_t GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  CJS_Icon(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime);
  ~CJS_Icon() override;

  WideString GetIconName() const { return icon_name_; }
  void SetIconName(WideString name) { icon_name_ = name; }

  JS_STATIC_PROP(name, name, CJS_Icon)

 private:
  static uint32_t ObjDefnID;
  static const char kName[];
  static const JSPropertySpec PropertySpecs[];

  CJS_Result get_name(CJS_Runtime* pRuntime);
  CJS_Result set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  WideString icon_name_;
};

#endif  // FXJS_CJS_ICON_H_
