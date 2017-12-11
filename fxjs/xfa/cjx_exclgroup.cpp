// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_exclgroup.h"

#include <vector>

#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "fxjs/js_resources.h"
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

CJS_Return CJX_ExclGroup::execEvent(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  execSingleEventByName(runtime->ToWideString(params[0]).AsStringView(),
                        XFA_Element::ExclGroup);
  return CJS_Return(true);
}

CJS_Return CJX_ExclGroup::execInitialize(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
  return CJS_Return(true);
}

CJS_Return CJX_ExclGroup::execCalculate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
  return CJS_Return(true);
}

CJS_Return CJX_ExclGroup::execValidate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* notify = GetDocument()->GetNotify();
  if (!notify)
    return CJS_Return(runtime->NewBoolean(false));

  int32_t iRet = notify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  return CJS_Return(runtime->NewBoolean(iRet != XFA_EVENTERROR_Error));
}

CJS_Return CJX_ExclGroup::selectedMember(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(runtime->NewNull());

  CXFA_Node* pReturnNode = nullptr;
  if (params.empty()) {
    pReturnNode = pWidgetData->GetSelectedMember();
  } else {
    pReturnNode = pWidgetData->SetSelectedMember(
        runtime->ToWideString(params[0]).AsStringView(), true);
  }
  if (!pReturnNode)
    return CJS_Return(runtime->NewNull());

  CFXJSE_Value* value =
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pReturnNode);
  if (!value)
    return CJS_Return(runtime->NewNull());

  return CJS_Return(value->DirectGetValue().Get(runtime->GetIsolate()));
}
