// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_style.h"

JSConstSpec CJS_Style::ConstSpecs[] = {
    {"ch", JSConstSpec::String, 0, "check"},
    {"cr", JSConstSpec::String, 0, "cross"},
    {"di", JSConstSpec::String, 0, "diamond"},
    {"ci", JSConstSpec::String, 0, "circle"},
    {"st", JSConstSpec::String, 0, "star"},
    {"sq", JSConstSpec::String, 0, "square"},
    {0, JSConstSpec::Number, 0, 0}};

const char* CJS_Style::g_pClassName = "style";
int CJS_Style::g_nObjDefnID = -1;

void CJS_Style::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_Style::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID =
      pEngine->DefineObj(CJS_Style::g_pClassName, eObjType, nullptr, nullptr);
  DefineConsts(pEngine);
}
