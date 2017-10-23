// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_ICON_H_
#define FPDFSDK_JAVASCRIPT_ICON_H_

#include "fpdfsdk/javascript/JS_Define.h"

class CPDF_Stream;

class Icon : public CJS_EmbedObj {
 public:
  explicit Icon(CJS_Object* pJSObject);
  ~Icon() override;

  bool get_name(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_name(CJS_Runtime* pRuntime, const CJS_Value& vp, WideString* sError);

  WideString GetIconName() const { return m_swIconName; }
  void SetIconName(WideString name) { m_swIconName = name; }

 private:
  WideString m_swIconName;
};

class CJS_Icon : public CJS_Object {
 public:
  explicit CJS_Icon(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Icon() override {}

  DECLARE_JS_CLASS();
  JS_STATIC_PROP(name, name, Icon);
};

#endif  // FPDFSDK_JAVASCRIPT_ICON_H_
