// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_exclgroup.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_exclgroup.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"

const CJX_MethodSpec CJX_ExclGroup::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execEvent", execEvent_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"selectedMember", selectedMember_static},
    {"", nullptr}};

CJX_ExclGroup::CJX_ExclGroup(CXFA_ExclGroup* group) : CJX_Node(group) {
  DefineMethods(MethodSpecs);
}

CJX_ExclGroup::~CJX_ExclGroup() {}

void CJX_ExclGroup::execEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::ExclGroup);
}

void CJX_ExclGroup::execInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_ExclGroup::execCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_ExclGroup::execValidate(CFXJSE_Arguments* pArguments) {
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

void CJX_ExclGroup::selectedMember(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc < 0 || argc > 1) {
    ThrowParamCountMismatchException(L"selectedMember");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_Node* pReturnNode = nullptr;
  if (argc == 0) {
    pReturnNode = pWidgetData->GetSelectedMember();
  } else {
    ByteString szName;
    szName = pArguments->GetUTF8String(0);
    pReturnNode = pWidgetData->SetSelectedMember(
        WideString::FromUTF8(szName.AsStringView()).AsStringView(), true);
  }
  if (!pReturnNode) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pReturnNode));
}
