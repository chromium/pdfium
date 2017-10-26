// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Icon.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"

JSPropertySpec CJS_Icon::PropertySpecs[] = {
    {"name", get_name_static, set_name_static},
    {0, 0, 0}};

const char* CJS_Icon::g_pClassName = "Icon";
int CJS_Icon::g_nObjDefnID = -1;

void CJS_Icon::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID =
      pEngine->DefineObj(CJS_Icon::g_pClassName, eObjType,
                         JSConstructor<CJS_Icon, Icon>, JSDestructor<CJS_Icon>);
  DefineProps(pEngine, g_nObjDefnID, PropertySpecs);
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
