// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_zoomtype.h"

const JSConstSpec CJS_Zoomtype::ConstSpecs[] = {
    {"none", JSConstSpec::String, 0, "NoVary"},
    {"fitP", JSConstSpec::String, 0, "FitPage"},
    {"fitW", JSConstSpec::String, 0, "FitWidth"},
    {"fitH", JSConstSpec::String, 0, "FitHeight"},
    {"fitV", JSConstSpec::String, 0, "FitVisibleWidth"},
    {"pref", JSConstSpec::String, 0, "Preferred"},
    {"refW", JSConstSpec::String, 0, "ReflowWidth"}};

uint32_t CJS_Zoomtype::ObjDefnID = 0;

// static
void CJS_Zoomtype::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("zoomtype", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, ObjDefnID, ConstSpecs);
}
