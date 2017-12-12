// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_icon.h"

const JSPropertySpec CJS_Icon::PropertySpecs[] = {
    {"name", get_name_static, set_name_static}};

int CJS_Icon::ObjDefnID = -1;

// static
int CJS_Icon::GetObjDefnID() {
  return ObjDefnID;
}

// static
void CJS_Icon::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("Icon", FXJSOBJTYPE_DYNAMIC,
                         JSConstructor<CJS_Icon, Icon>, JSDestructor<CJS_Icon>);
  DefineProps(pEngine, ObjDefnID, PropertySpecs, FX_ArraySize(PropertySpecs));
}

Icon::Icon(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_swIconName(L"") {}

Icon::~Icon() {}

CJS_Return Icon::get_name(CJS_Runtime* pRuntime) {
  return CJS_Return(pRuntime->NewString(m_swIconName.c_str()));
}

CJS_Return Icon::set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return CJS_Return(false);
}
