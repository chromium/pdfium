// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_subform.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_delta.h"
#include "xfa/fxfa/parser/cxfa_document.h"

const CJX_MethodSpec CJX_Subform::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execEvent", execEvent_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"", nullptr}};

CJX_Subform::CJX_Subform(CXFA_Node* node) : CJX_Container(node) {
  DefineMethods(MethodSpecs);
}

CJX_Subform::~CJX_Subform() {}

void CJX_Subform::execEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::Subform);
}

void CJX_Subform::execInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_Subform::execCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_Subform::execValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet =
      pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}
