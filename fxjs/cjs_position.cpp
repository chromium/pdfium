// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_position.h"

const JSConstSpec CJS_Position::ConstSpecs[] = {
    {"textOnly", JSConstSpec::Number, 0, nullptr},
    {"iconOnly", JSConstSpec::Number, 1, nullptr},
    {"iconTextV", JSConstSpec::Number, 2, nullptr},
    {"textIconV", JSConstSpec::Number, 3, nullptr},
    {"iconTextH", JSConstSpec::Number, 4, nullptr},
    {"textIconH", JSConstSpec::Number, 5, nullptr},
    {"overlay", JSConstSpec::Number, 6, nullptr}};

uint32_t CJS_Position::ObjDefnID = 0;

// static
void CJS_Position::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("position", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, ObjDefnID, ConstSpecs);
}
