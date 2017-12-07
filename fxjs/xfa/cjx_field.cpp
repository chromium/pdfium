// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_field.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
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

void CJX_Field::clearItems(CFXJSE_Arguments* pArguments) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  pWidgetData->DeleteItem(-1, true, false);
}

void CJX_Field::execEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  int32_t iRet = execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::Field);
  if (eventString != "validate")
    return;

  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Field::execInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                false);
}

void CJX_Field::deleteItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"deleteItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  int32_t iIndex = pArguments->GetInt32(0);
  bool bValue = pWidgetData->DeleteItem(iIndex, true, true);
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(bValue);
}

void CJX_Field::getSaveItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getSaveItem");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, true);
  if (!value) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  pArguments->GetReturnValue()->SetString(value->UTF8Encode().AsStringView());
}

void CJX_Field::boundItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"boundItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  ByteString bsValue = pArguments->GetUTF8String(0);
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  WideString wsBoundValue = pWidgetData->GetItemValue(wsValue.AsStringView());
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetString(wsBoundValue.UTF8Encode().AsStringView());
}

void CJX_Field::getItemState(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getItemState");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(pWidgetData->GetItemState(pArguments->GetInt32(0)));
}

void CJX_Field::execCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                false);
}

void CJX_Field::getDisplayItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getDisplayItem");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, false);
  if (!value) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  pArguments->GetReturnValue()->SetString(value->UTF8Encode().AsStringView());
}

void CJX_Field::setItemState(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    ThrowParamCountMismatchException(L"setItemState");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  int32_t iIndex = pArguments->GetInt32(0);
  if (pArguments->GetInt32(1) != 0) {
    pWidgetData->SetItemState(iIndex, true, true, true, true);
    return;
  }

  if (pWidgetData->GetItemState(iIndex))
    pWidgetData->SetItemState(iIndex, false, true, true, true);
}

void CJX_Field::addItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowParamCountMismatchException(L"addItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  WideString wsLabel;
  if (iLength >= 1) {
    ByteString bsLabel = pArguments->GetUTF8String(0);
    wsLabel = WideString::FromUTF8(bsLabel.AsStringView());
  }

  WideString wsValue;
  if (iLength >= 2) {
    ByteString bsValue = pArguments->GetUTF8String(1);
    wsValue = WideString::FromUTF8(bsValue.AsStringView());
  }

  pWidgetData->InsertItem(wsLabel, wsValue, true);
}

void CJX_Field::execValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet = pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate,
                                               false, false);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}
