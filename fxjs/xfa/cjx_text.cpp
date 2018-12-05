// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_text.h"

#include "xfa/fxfa/parser/cxfa_text.h"

CJX_Text::CJX_Text(CXFA_Text* node) : CJX_Content(node) {}

CJX_Text::~CJX_Text() = default;

void CJX_Text::defaultValue(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute) {
  ScriptSomDefaultValue(pValue, bSetting, eAttribute);
}

void CJX_Text::value(CFXJSE_Value* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  ScriptSomDefaultValue(pValue, bSetting, eAttribute);
}
