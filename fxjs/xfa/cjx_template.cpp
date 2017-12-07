// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_template.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_template.h"

const CJX_MethodSpec CJX_Template::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"formNodes", formNodes_static},
    {"recalculate", recalculate_static},
    {"remerge", remerge_static},
    {"", nullptr}};

CJX_Template::CJX_Template(CXFA_Template* tmpl) : CJX_Model(tmpl) {
  DefineMethods(MethodSpecs);
}

CJX_Template::~CJX_Template() {}

void CJX_Template::formNodes(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"formNodes");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Template::remerge(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"remerge");
    return;
  }
  GetDocument()->DoDataRemerge(true);
}

void CJX_Template::execInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Template::recalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"recalculate");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Template::execCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Template::execValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(!!GetXFANode()->GetWidgetData());
}
