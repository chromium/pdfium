// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_highlight.h"

JSConstSpec CJS_Highlight::ConstSpecs[] = {
    {"n", JSConstSpec::String, 0, "none"},
    {"i", JSConstSpec::String, 0, "invert"},
    {"p", JSConstSpec::String, 0, "push"},
    {"o", JSConstSpec::String, 0, "outline"},
    {0, JSConstSpec::Number, 0, 0}};

int CJS_Highlight::g_nObjDefnID = -1;

void CJS_Highlight::DefineJSObjects(CFXJS_Engine* pEngine,
                                    FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj("highlight", eObjType, nullptr, nullptr);
  DefineConsts(pEngine, g_nObjDefnID, ConstSpecs);
}
