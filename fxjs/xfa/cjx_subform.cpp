// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_subform.h"

#include "core/fxcrt/span.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_delta.h"
#include "xfa/fxfa/parser/cxfa_document.h"

const CJX_MethodSpec CJX_Subform::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execEvent", execEvent_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static}};

CJX_Subform::CJX_Subform(CXFA_Node* node) : CJX_Container(node) {
  DefineMethods(MethodSpecs);
}

CJX_Subform::~CJX_Subform() = default;

bool CJX_Subform::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Subform::execEvent(CFXJSE_Engine* runtime,
                                  pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  execSingleEventByName(runtime->ToWideString(params[0]).AsStringView(),
                        XFA_Element::Subform);
  return CJS_Result::Success();
}

CJS_Result CJX_Subform::execInitialize(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_Subform::execCalculate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_Subform::execValidate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success(runtime->NewBoolean(false));

  XFA_EventError iRet = pNotify->ExecEventByDeepFirst(
      GetXFANode(), XFA_EVENT_Validate, false, true);
  return CJS_Result::Success(
      runtime->NewBoolean(iRet != XFA_EventError::kError));
}

void CJX_Subform::locale(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute eAttribute) {
  if (bSetting) {
    SetCDataImpl(XFA_Attribute::Locale,
                 fxv8::ReentrantToWideStringHelper(pIsolate, *pValue), true,
                 true);
    return;
  }

  WideString wsLocaleName = GetXFANode()->GetLocaleName().value_or(L"");
  *pValue =
      fxv8::NewStringHelper(pIsolate, wsLocaleName.ToUTF8().AsStringView());
}

void CJX_Subform::instanceManager(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }

  WideString wsName = GetCData(XFA_Attribute::Name);
  CXFA_Node* pInstanceMgr = nullptr;
  for (CXFA_Node* pNode = GetXFANode()->GetPrevSibling(); pNode;
       pNode = pNode->GetPrevSibling()) {
    if (pNode->GetElementType() == XFA_Element::InstanceManager) {
      WideString wsInstMgrName =
          pNode->JSObject()->GetCData(XFA_Attribute::Name);
      if (wsInstMgrName.GetLength() >= 1 && wsInstMgrName[0] == '_' &&
          wsInstMgrName.Last(wsInstMgrName.GetLength() - 1) == wsName) {
        pInstanceMgr = pNode;
      }
      break;
    }
  }
  *pValue = pInstanceMgr ? GetDocument()
                               ->GetScriptContext()
                               ->GetOrCreateJSBindingFromMap(pInstanceMgr)
                               .As<v8::Value>()
                         : fxv8::NewNullHelper(pIsolate).As<v8::Value>();
}
