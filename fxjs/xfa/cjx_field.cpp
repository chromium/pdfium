// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_field.h"

#include <vector>

#include "fxjs/cfxjse_value.h"
#include "fxjs/js_resources.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_field.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"

const CJX_MethodSpec CJX_Field::MethodSpecs[] = {
    {"addItem", addItem_static},
    {"boundItem", boundItem_static},
    {"clearItems", clearItems_static},
    {"deleteItem", deleteItem_static},
    {"execCalculate", execCalculate_static},
    {"execEvent", execEvent_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"getDisplayItem", getDisplayItem_static},
    {"getItemState", getItemState_static},
    {"getSaveItem", getSaveItem_static},
    {"setItemState", setItemState_static},
    {"", nullptr}};

CJX_Field::CJX_Field(CXFA_Field* field) : CJX_Container(field) {
  DefineMethods(MethodSpecs);
}

CJX_Field::~CJX_Field() {}

CJS_Return CJX_Field::clearItems(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (pWidgetData)
    pWidgetData->DeleteItem(-1, true, false);
  return CJS_Return(true);
}

CJS_Return CJX_Field::execEvent(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  WideString eventString = runtime->ToWideString(params[0]);
  int32_t iRet =
      execSingleEventByName(eventString.AsStringView(), XFA_Element::Field);
  if (eventString != L"validate")
    return CJS_Return(true);

  return CJS_Return(runtime->NewBoolean(iRet != XFA_EVENTERROR_Error));
}

CJS_Return CJX_Field::execInitialize(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                  false);
  }
  return CJS_Return(true);
}

CJS_Return CJX_Field::deleteItem(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(true);

  bool bValue =
      pWidgetData->DeleteItem(runtime->ToInt32(params[0]), true, true);
  return CJS_Return(runtime->NewBoolean(bValue));
}

CJS_Return CJX_Field::getSaveItem(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (iIndex < 0)
    return CJS_Return(runtime->NewNull());

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(runtime->NewNull());

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, true);
  if (!value)
    return CJS_Return(runtime->NewNull());

  return CJS_Return(runtime->NewString(value->UTF8Encode().AsStringView()));
}

CJS_Return CJX_Field::boundItem(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(true);

  WideString value = runtime->ToWideString(params[0]);
  WideString boundValue = pWidgetData->GetItemValue(value.AsStringView());
  return CJS_Return(runtime->NewString(boundValue.UTF8Encode().AsStringView()));
}

CJS_Return CJX_Field::getItemState(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(true);

  int32_t state = pWidgetData->GetItemState(runtime->ToInt32(params[0]));
  return CJS_Return(runtime->NewBoolean(state != 0));
}

CJS_Return CJX_Field::execCalculate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                  false);
  }
  return CJS_Return(true);
}

CJS_Return CJX_Field::getDisplayItem(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (iIndex < 0)
    return CJS_Return(runtime->NewNull());

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(runtime->NewNull());

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, false);
  if (!value)
    return CJS_Return(runtime->NewNull());

  return CJS_Return(runtime->NewString(value->UTF8Encode().AsStringView()));
}

CJS_Return CJX_Field::setItemState(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(true);

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (runtime->ToInt32(params[1]) != 0) {
    pWidgetData->SetItemState(iIndex, true, true, true, true);
    return CJS_Return(true);
  }
  if (pWidgetData->GetItemState(iIndex))
    pWidgetData->SetItemState(iIndex, false, true, true, true);

  return CJS_Return(true);
}

CJS_Return CJX_Field::addItem(CJS_V8* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1 && params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return CJS_Return(true);

  WideString label;
  if (params.size() >= 1)
    label = runtime->ToWideString(params[0]);

  WideString value;
  if (params.size() >= 2)
    value = runtime->ToWideString(params[1]);

  pWidgetData->InsertItem(label, value, true);
  return CJS_Return(true);
}

CJS_Return CJX_Field::execValidate(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Return(runtime->NewBoolean(false));

  int32_t iRet = pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate,
                                               false, false);
  return CJS_Return(runtime->NewBoolean(iRet != XFA_EVENTERROR_Error));
}
