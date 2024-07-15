// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_form.h"

#include "core/fxcrt/span.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
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
    {"remerge", remerge_static}};

CJX_Form::CJX_Form(CXFA_Form* form) : CJX_Model(form) {
  DefineMethods(MethodSpecs);
}

CJX_Form::~CJX_Form() = default;

bool CJX_Form::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Form::formNodes(CFXJSE_Engine* runtime,
                               pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pDataNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pDataNode)
    return CJS_Result::Failure(JSMessage::kValueError);

  CXFA_Document* pDoc = GetDocument();
  auto* pFormNodes = cppgc::MakeGarbageCollected<CXFA_ArrayNodeList>(
      pDoc->GetHeap()->GetAllocationHandle(), pDoc);
  pDoc->GetNodeOwner()->PersistList(pFormNodes);

  v8::Local<v8::Value> value = runtime->GetOrCreateJSBindingFromMap(pFormNodes);
  return CJS_Result::Success(value);
}

CJS_Result CJX_Form::remerge(CFXJSE_Engine* runtime,
                             pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  GetDocument()->DoDataRemerge();
  return CJS_Result::Success();
}

CJS_Result CJX_Form::execInitialize(CFXJSE_Engine* runtime,
                                    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_Form::recalculate(CFXJSE_Engine* runtime,
                                 pdfium::span<v8::Local<v8::Value>> params) {
  CXFA_EventParam* pEventParam = runtime->GetEventParam();
  if (pEventParam && (pEventParam->m_eType == XFA_EVENT_Calculate ||
                      pEventParam->m_eType == XFA_EVENT_InitCalculate)) {
    return CJS_Result::Success();
  }
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify || runtime->ToInt32(params[0]) != 0)
    return CJS_Result::Success();

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false, true);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate, false, true);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Ready, true, true);
  return CJS_Result::Success();
}

CJS_Result CJX_Form::execCalculate(CFXJSE_Engine* runtime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                  true);
  return CJS_Result::Success();
}

CJS_Result CJX_Form::execValidate(CFXJSE_Engine* runtime,
                                  pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 0)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success(runtime->NewBoolean(false));

  XFA_EventError iRet = pNotify->ExecEventByDeepFirst(
      GetXFANode(), XFA_EVENT_Validate, false, true);
  return CJS_Result::Success(
      runtime->NewBoolean(iRet != XFA_EventError::kError));
}

void CJX_Form::checksumS(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute eAttribute) {
  if (bSetting) {
    SetAttributeByEnum(XFA_Attribute::Checksum,
                       fxv8::ReentrantToWideStringHelper(pIsolate, *pValue),
                       false);
    return;
  }

  std::optional<WideString> checksum =
      TryAttribute(XFA_Attribute::Checksum, false);
  *pValue = fxv8::NewStringHelper(
      pIsolate,
      checksum.has_value() ? checksum.value().ToUTF8().AsStringView() : "");
}
