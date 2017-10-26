// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_border.h"

JSConstSpec CJS_Border::ConstSpecs[] = {
    {"s", JSConstSpec::String, 0, "solid"},
    {"b", JSConstSpec::String, 0, "beveled"},
    {"d", JSConstSpec::String, 0, "dashed"},
    {"i", JSConstSpec::String, 0, "inset"},
    {"u", JSConstSpec::String, 0, "underline"},
    {0, JSConstSpec::Number, 0, 0}};

int CJS_Border::g_nObjDefnID = -1;

void CJS_Border::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj("border", eObjType, nullptr, nullptr);
  DefineConsts(pEngine, g_nObjDefnID, ConstSpecs);
}
