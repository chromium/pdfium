// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_form.h"

#include <vector>

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_form.h"

const CJX_MethodSpec CJX_Form::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"formNodes", formNodes_static},
    {"recalculate", recalculate_static},
    {"remerge", remerge_static},
    {"", nullptr}};

CJX_Form::CJX_Form(CXFA_Form* form) : CJX_Model(form) {
  DefineMethods(MethodSpecs);
}

CJX_Form::~CJX_Form() {}

void CJX_Form::formNodes(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"formNodes");
    return;
  }

  CXFA_Node* pDataNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pDataNode) {
    ThrowArgumentMismatchException();
    return;
  }

  std::vector<CXFA_Node*> formItems;
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(GetDocument());
  pFormNodes->SetArrayNodeList(formItems);
  pArguments->GetReturnValue()->SetObject(
      pFormNodes, GetDocument()->GetScriptContext()->GetJseNormalClass());
}

void CJX_Form::remerge(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"remerge");
    return;
  }

  GetDocument()->DoDataRemerge(true);
}

void CJX_Form::execInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_Form::recalculate(CFXJSE_Arguments* pArguments) {
  CXFA_EventParam* pEventParam =
      GetDocument()->GetScriptContext()->GetEventParam();
  if (pEventParam->m_eType == XFA_EVENT_Calculate ||
      pEventParam->m_eType == XFA_EVENT_InitCalculate) {
    return;
  }
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"recalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;
  if (pArguments->GetInt32(0) != 0)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Ready, true);
}

void CJX_Form::execCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_Form::execValidate(CFXJSE_Arguments* pArguments) {
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
