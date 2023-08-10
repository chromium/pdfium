// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_scalewhen.h"

const JSConstSpec CJS_ScaleWhen::ConstSpecs[] = {
    {"always", JSConstSpec::Number, 0, nullptr},
    {"never", JSConstSpec::Number, 1, nullptr},
    {"tooBig", JSConstSpec::Number, 2, nullptr},
    {"tooSmall", JSConstSpec::Number, 3, nullptr}};

uint32_t CJS_ScaleWhen::ObjDefnID = 0;

// static
void CJS_ScaleWhen::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("scaleWhen", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, ObjDefnID, ConstSpecs);
}
