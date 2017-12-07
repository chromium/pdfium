// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_manifest.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_manifest.h"

const CJX_MethodSpec CJX_Manifest::MethodSpecs[] = {
    {"evaluate", evaluate_static},
    {"", nullptr}};

CJX_Manifest::CJX_Manifest(CXFA_Manifest* manifest) : CJX_Node(manifest) {
  DefineMethods(MethodSpecs);
}

CJX_Manifest::~CJX_Manifest() {}

void CJX_Manifest::evaluate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"evaluate");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}
