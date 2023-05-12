// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_exclgroup.h"

#include <vector>

#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_exclgroup.h"

const CJX_MethodSpec CJX_ExclGroup::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execEvent", execEvent_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"selectedMember", selectedMember_static}};

CJX_ExclGroup::CJX_ExclGroup(CXFA_ExclGroup* group) : CJX_Node(group) {
  DefineMethods(MethodSpecs);
}

CJX_ExclGroup::~CJX_ExclGroup() = default;

bool CJX_ExclGroup::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_ExclGroup::execEvent(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  execSingleEventByName(runtime->ToWideString(params[0]).AsStringView(),
                        XFA_Element::ExclGroup);
  return CJS_Result::Success();
}

CJS_Result CJX_ExclGroup::execInitialize(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_ExclGroup::execCalculate(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_ExclGroup::execValidate(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* notify = GetDocument()->GetNotify();
  if (!notify)
    return CJS_Result::Success(runtime->NewBoolean(false));

  XFA_EventError iRet = notify->ExecEventByDeepFirst(
      GetXFANode(), XFA_EVENT_Validate, false, true);
  return CJS_Result::Success(
      runtime->NewBoolean(iRet != XFA_EventError::kError));
}

CJS_Result CJX_ExclGroup::selectedMember(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success(runtime->NewNull());

  CXFA_Node* pReturnNode = nullptr;
  if (params.empty()) {
    pReturnNode = node->GetSelectedMember();
  } else {
    pReturnNode = node->SetSelectedMember(
        runtime->ToWideString(params[0]).AsStringView());
  }
  if (!pReturnNode)
    return CJS_Result::Success(runtime->NewNull());

  return CJS_Result::Success(runtime->GetOrCreateJSBindingFromMap(pReturnNode));
}

void CJX_ExclGroup::defaultValue(v8::Isolate* pIsolate,
                                 v8::Local<v8::Value>* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute) {
  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return;

  if (bSetting) {
    node->SetSelectedMemberByValue(
        fxv8::ReentrantToWideStringHelper(pIsolate, *pValue).AsStringView(),
        true, true, true);
    return;
  }

  WideString wsValue = GetContent(true);
  XFA_VERSION curVersion = GetDocument()->GetCurVersionMode();
  if (wsValue.IsEmpty() && curVersion >= XFA_VERSION_300) {
    *pValue = fxv8::NewNullHelper(pIsolate);
    return;
  }
  *pValue = fxv8::NewStringHelper(pIsolate, wsValue.ToUTF8().AsStringView());
}

void CJX_ExclGroup::rawValue(v8::Isolate* pIsolate,
                             v8::Local<v8::Value>* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute) {
  defaultValue(pIsolate, pValue, bSetting, eAttribute);
}

void CJX_ExclGroup::transient(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {}

void CJX_ExclGroup::errorText(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting)
    ThrowInvalidPropertyException(pIsolate);
}
