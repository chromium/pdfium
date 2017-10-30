// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_object.h"

// static
void CJS_Object::DefineConsts(CFXJS_Engine* pEngine,
                              int objId,
                              const JSConstSpec consts[]) {
  for (size_t i = 0; consts[i].pName != 0; ++i) {
    pEngine->DefineObjConst(
        objId, consts[i].pName,
        consts[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(consts[i].number).As<v8::Value>()
            : pEngine->NewString(consts[i].pStr).As<v8::Value>());
  }
}

// static
void CJS_Object::DefineProps(CFXJS_Engine* pEngine,
                             int objId,
                             const JSPropertySpec props[]) {
  for (size_t i = 0; props[i].pName != 0; ++i) {
    pEngine->DefineObjProperty(objId, props[i].pName, props[i].pPropGet,
                               props[i].pPropPut);
  }
}

// static
void CJS_Object::DefineMethods(CFXJS_Engine* pEngine,
                               int objId,
                               const JSMethodSpec methods[]) {
  for (size_t i = 0; methods[i].pName != 0; ++i)
    pEngine->DefineObjMethod(objId, methods[i].pName, methods[i].pMethodCall);
}

CJS_Object::CJS_Object(v8::Local<v8::Object> pObject) {
  m_pIsolate = pObject->GetIsolate();
  m_pV8Object.Reset(m_pIsolate, pObject);
}

CJS_Object::~CJS_Object() {}

void CJS_Object::InitInstance(IJS_Runtime* pIRuntime) {}
