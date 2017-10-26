// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/cjs_font.h"

JSConstSpec CJS_Font::ConstSpecs[] = {
    {"Times", JSConstSpec::String, 0, "Times-Roman"},
    {"TimesB", JSConstSpec::String, 0, "Times-Bold"},
    {"TimesI", JSConstSpec::String, 0, "Times-Italic"},
    {"TimesBI", JSConstSpec::String, 0, "Times-BoldItalic"},
    {"Helv", JSConstSpec::String, 0, "Helvetica"},
    {"HelvB", JSConstSpec::String, 0, "Helvetica-Bold"},
    {"HelvI", JSConstSpec::String, 0, "Helvetica-Oblique"},
    {"HelvBI", JSConstSpec::String, 0, "Helvetica-BoldOblique"},
    {"Cour", JSConstSpec::String, 0, "Courier"},
    {"CourB", JSConstSpec::String, 0, "Courier-Bold"},
    {"CourI", JSConstSpec::String, 0, "Courier-Oblique"},
    {"CourBI", JSConstSpec::String, 0, "Courier-BoldOblique"},
    {"Symbol", JSConstSpec::String, 0, "Symbol"},
    {"ZapfD", JSConstSpec::String, 0, "ZapfDingbats"},
    {0, JSConstSpec::Number, 0, 0}};

const char* CJS_Font::g_pClassName = "font";
int CJS_Font::g_nObjDefnID = -1;

void CJS_Font::DefineConsts(CFXJS_Engine* pEngine) {
  for (size_t i = 0; i < FX_ArraySize(ConstSpecs) - 1; ++i) {
    pEngine->DefineObjConst(
        g_nObjDefnID, ConstSpecs[i].pName,
        ConstSpecs[i].eType == JSConstSpec::Number
            ? pEngine->NewNumber(ConstSpecs[i].number).As<v8::Value>()
            : pEngine->NewString(ConstSpecs[i].pStr).As<v8::Value>());
  }
}

void CJS_Font::DefineJSObjects(CFXJS_Engine* pEngine, FXJSOBJTYPE eObjType) {
  g_nObjDefnID =
      pEngine->DefineObj(CJS_Font::g_pClassName, eObjType, nullptr, nullptr);
  DefineConsts(pEngine);
}
