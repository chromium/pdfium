// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_position.h"

JSConstSpec CJS_Position::ConstSpecs[] = {
    {"textOnly", JSConstSpec::Number, 0, 0},
    {"iconOnly", JSConstSpec::Number, 1, 0},
    {"iconTextV", JSConstSpec::Number, 2, 0},
    {"textIconV", JSConstSpec::Number, 3, 0},
    {"iconTextH", JSConstSpec::Number, 4, 0},
    {"textIconH", JSConstSpec::Number, 5, 0},
    {"overlay", JSConstSpec::Number, 6, 0},
    {0, JSConstSpec::Number, 0, 0}};

const char* CJS_Position::g_pClassName = "position";
int CJS_Position::g_nObjDefnID = -1;

void CJS_Position::DefineJSObjects(CFXJS_Engine* pEngine,
                                   FXJSOBJTYPE eObjType) {
  g_nObjDefnID = pEngine->DefineObj(CJS_Position::g_pClassName, eObjType,
                                    nullptr, nullptr);
  DefineConsts(pEngine, g_nObjDefnID, ConstSpecs);
}
