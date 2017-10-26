// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_display.h"

JSConstSpec CJS_Display::ConstSpecs[] = {{"visible", JSConstSpec::Number, 0, 0},
                                         {"hidden", JSConstSpec::Number, 1, 0},
                                         {"noPrint", JSConstSpec::Number, 2, 0},
                                         {"noView", JSConstSpec::Number, 3, 0},
                                         {0, JSConstSpec::Number, 0, 0}};

const char* CJS_Display::g_pClassName = "display";
int CJS_Display::g_nObjDefnID = -1;

void CJS_Display::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID =
      pEngine->DefineObj(CJS_Display::g_pClassName, eObjType, nullptr, nullptr);
  DefineConsts(pEngine, g_nObjDefnID, ConstSpecs);
}
