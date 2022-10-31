// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_field.h"

#include <vector>

#include "fxjs/cfx_v8.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fgas/crt/cfgas_decimal.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_field.h"
#include "xfa/fxfa/parser/cxfa_value.h"

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
    {"setItemState", setItemState_static}};

CJX_Field::CJX_Field(CXFA_Field* field) : CJX_Container(field) {
  DefineMethods(MethodSpecs);
}

CJX_Field::~CJX_Field() = default;

bool CJX_Field::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Field::clearItems(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_Node* node = GetXFANode();
  if (node->IsWidgetReady())
    node->DeleteItem(-1, true, false);
  return CJS_Result::Success();
}

CJS_Result CJX_Field::execEvent(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString eventString = runtime->ToWideString(params[0]);
  XFA_EventError iRet =
      execSingleEventByName(eventString.AsStringView(), XFA_Element::Field);
  if (!eventString.EqualsASCII("validate"))
    return CJS_Result::Success();

  return CJS_Result::Success(
      runtime->NewBoolean(iRet != XFA_EventError::kError));
}

CJS_Result CJX_Field::execInitialize(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                  false);
  }
  return CJS_Result::Success();
}

CJS_Result CJX_Field::deleteItem(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success();

  bool bValue = node->DeleteItem(runtime->ToInt32(params[0]), true, true);
  return CJS_Result::Success(runtime->NewBoolean(bValue));
}

CJS_Result CJX_Field::getSaveItem(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (iIndex < 0)
    return CJS_Result::Success(runtime->NewNull());

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success(runtime->NewNull());

  absl::optional<WideString> value = node->GetChoiceListItem(iIndex, true);
  if (!value.has_value())
    return CJS_Result::Success(runtime->NewNull());

  return CJS_Result::Success(
      runtime->NewString(value->ToUTF8().AsStringView()));
}

CJS_Result CJX_Field::boundItem(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success();

  WideString value = runtime->ToWideString(params[0]);
  WideString boundValue = node->GetItemValue(value.AsStringView());
  return CJS_Result::Success(
      runtime->NewString(boundValue.ToUTF8().AsStringView()));
}

CJS_Result CJX_Field::getItemState(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success();

  int32_t state = node->GetItemState(runtime->ToInt32(params[0]));
  return CJS_Result::Success(runtime->NewBoolean(state != 0));
}

CJS_Result CJX_Field::execCalculate(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                  false);
  }
  return CJS_Result::Success();
}

CJS_Result CJX_Field::getDisplayItem(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (iIndex < 0)
    return CJS_Result::Success(runtime->NewNull());

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success(runtime->NewNull());

  absl::optional<WideString> value = node->GetChoiceListItem(iIndex, false);
  if (!value.has_value())
    return CJS_Result::Success(runtime->NewNull());

  return CJS_Result::Success(
      runtime->NewString(value->ToUTF8().AsStringView()));
}

CJS_Result CJX_Field::setItemState(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success();

  int32_t iIndex = runtime->ToInt32(params[0]);
  if (runtime->ToInt32(params[1]) != 0) {
    node->SetItemState(iIndex, true, true, true);
    return CJS_Result::Success();
  }
  if (node->GetItemState(iIndex))
    node->SetItemState(iIndex, false, true, true);

  return CJS_Result::Success();
}

CJS_Result CJX_Field::addItem(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1 && params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return CJS_Result::Success();

  WideString label;
  if (params.size() >= 1)
    label = runtime->ToWideString(params[0]);

  WideString value;
  if (params.size() >= 2)
    value = runtime->ToWideString(params[1]);

  node->InsertItem(label, value, true);
  return CJS_Result::Success();
}

CJS_Result CJX_Field::execValidate(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success(runtime->NewBoolean(false));

  XFA_EventError iRet = pNotify->ExecEventByDeepFirst(
      GetXFANode(), XFA_EVENT_Validate, false, false);
  return CJS_Result::Success(
      runtime->NewBoolean(iRet != XFA_EventError::kError));
}

void CJX_Field::defaultValue(v8::Isolate* pIsolate,
                             v8::Local<v8::Value>* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute) {
  CXFA_Node* xfaNode = GetXFANode();
  if (!xfaNode->IsWidgetReady())
    return;

  if (bSetting) {
    if (pValue) {
      xfaNode->SetPreNull(xfaNode->IsNull());
      xfaNode->SetIsNull(fxv8::IsNull(*pValue));
    }

    WideString wsNewText;
    if (pValue && !(fxv8::IsNull(*pValue) || fxv8::IsUndefined(*pValue)))
      wsNewText = fxv8::ReentrantToWideStringHelper(pIsolate, *pValue);
    if (xfaNode->GetUIChildNode()->GetElementType() == XFA_Element::NumericEdit)
      wsNewText = xfaNode->NumericLimit(wsNewText);

    CXFA_Node* pContainerNode = xfaNode->GetContainerNode();
    WideString wsFormatText(wsNewText);
    if (pContainerNode)
      wsFormatText = pContainerNode->GetFormatDataValue(wsNewText);

    SetContent(wsNewText, wsFormatText, true, true, true);
    return;
  }

  WideString content = GetContent(true);
  if (content.IsEmpty()) {
    *pValue = fxv8::NewNullHelper(pIsolate);
    return;
  }

  CXFA_Node* formValue = xfaNode->GetFormValueIfExists();
  CXFA_Node* pNode = formValue ? formValue->GetFirstChild() : nullptr;
  if (pNode && pNode->GetElementType() == XFA_Element::Decimal) {
    if (xfaNode->GetUIChildNode()->GetElementType() ==
            XFA_Element::NumericEdit &&
        (pNode->JSObject()->GetInteger(XFA_Attribute::FracDigits) == -1)) {
      *pValue =
          fxv8::NewStringHelper(pIsolate, content.ToUTF8().AsStringView());
    } else {
      CFGAS_Decimal decimal(content.AsStringView());
      *pValue = fxv8::NewNumberHelper(pIsolate, decimal.ToFloat());
    }
  } else if (pNode && pNode->GetElementType() == XFA_Element::Integer) {
    *pValue = fxv8::NewNumberHelper(pIsolate, FXSYS_wtoi(content.c_str()));
  } else if (pNode && pNode->GetElementType() == XFA_Element::Boolean) {
    *pValue =
        fxv8::NewBooleanHelper(pIsolate, FXSYS_wtoi(content.c_str()) != 0);
  } else if (pNode && pNode->GetElementType() == XFA_Element::Float) {
    CFGAS_Decimal decimal(content.AsStringView());
    *pValue = fxv8::NewNumberHelper(pIsolate, decimal.ToFloat());
  } else {
    *pValue = fxv8::NewStringHelper(pIsolate, content.ToUTF8().AsStringView());
  }
}

void CJX_Field::editValue(v8::Isolate* pIsolate,
                          v8::Local<v8::Value>* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute) {
  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return;

  if (bSetting) {
    node->SetValue(XFA_ValuePicture::kEdit,
                   fxv8::ReentrantToWideStringHelper(pIsolate, *pValue));
    return;
  }
  *pValue = fxv8::NewStringHelper(
      pIsolate,
      node->GetValue(XFA_ValuePicture::kEdit).ToUTF8().AsStringView());
}

void CJX_Field::formatMessage(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  ScriptSomMessage(pIsolate, pValue, bSetting, SOMMessageType::kFormatMessage);
}

void CJX_Field::formattedValue(v8::Isolate* pIsolate,
                               v8::Local<v8::Value>* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {
  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return;

  if (bSetting) {
    node->SetValue(XFA_ValuePicture::kDisplay,
                   fxv8::ReentrantToWideStringHelper(pIsolate, *pValue));
    return;
  }
  *pValue = fxv8::NewStringHelper(
      pIsolate,
      node->GetValue(XFA_ValuePicture::kDisplay).ToUTF8().AsStringView());
}

void CJX_Field::length(v8::Isolate* pIsolate,
                       v8::Local<v8::Value>* pValue,
                       bool bSetting,
                       XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }

  CXFA_Node* node = GetXFANode();
  *pValue = fxv8::NewNumberHelper(
      pIsolate, node->IsWidgetReady() ? pdfium::base::checked_cast<int>(
                                            node->CountChoiceListItems(true))
                                      : 0);
}

void CJX_Field::parentSubform(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  *pValue = fxv8::NewNullHelper(pIsolate);
}

void CJX_Field::selectedIndex(v8::Isolate* pIsolate,
                              v8::Local<v8::Value>* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute) {
  CXFA_Node* node = GetXFANode();
  if (!node->IsWidgetReady())
    return;

  if (!bSetting) {
    *pValue = fxv8::NewNumberHelper(pIsolate, node->GetSelectedItem(0));
    return;
  }

  int32_t iIndex = fxv8::ReentrantToInt32Helper(pIsolate, *pValue);
  if (iIndex == -1) {
    node->ClearAllSelections();
    return;
  }

  node->SetItemState(iIndex, true, true, true);
}

void CJX_Field::rawValue(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute eAttribute) {
  defaultValue(pIsolate, pValue, bSetting, eAttribute);
}
