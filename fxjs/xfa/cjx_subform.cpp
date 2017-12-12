// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_subform.h"

#include <vector>

#include "fxjs/cfxjse_value.h"
#include "fxjs/js_resources.h"
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
  DefineMethods(MethodSpecs, FX_ArraySize(MethodSpecs));
}

CJX_Subform::~CJX_Subform() {}

CJS_Return CJX_Subform::execEvent(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  execSingleEventByName(runtime->ToWideString(params[0]).AsStringView(),
                        XFA_Element::Subform);
  return CJS_Return(true);
}

CJS_Return CJX_Subform::execInitialize(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
  return CJS_Return(true);
}

CJS_Return CJX_Subform::execCalculate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
  return CJS_Return(true);
}

CJS_Return CJX_Subform::execValidate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Return(runtime->NewBoolean(false));

  int32_t iRet =
      pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  return CJS_Return(runtime->NewBoolean(iRet != XFA_EVENTERROR_Error));
}
