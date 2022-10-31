// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_border.h"

const JSConstSpec CJS_Border::ConstSpecs[] = {
    {"s", JSConstSpec::String, 0, "solid"},
    {"b", JSConstSpec::String, 0, "beveled"},
    {"d", JSConstSpec::String, 0, "dashed"},
    {"i", JSConstSpec::String, 0, "inset"},
    {"u", JSConstSpec::String, 0, "underline"}};

uint32_t CJS_Border::ObjDefnID = 0;

// static
void CJS_Border::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("border", FXJSOBJTYPE_STATIC, nullptr, nullptr);
  DefineConsts(pEngine, ObjDefnID, ConstSpecs);
}
