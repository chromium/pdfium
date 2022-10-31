// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_boolean.h"

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cxfa_boolean.h"

CJX_Boolean::CJX_Boolean(CXFA_Boolean* node) : CJX_Object(node) {}

CJX_Boolean::~CJX_Boolean() = default;

bool CJX_Boolean::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Boolean::defaultValue(v8::Isolate* pIsolate,
                               v8::Local<v8::Value>* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {
  if (!bSetting) {
    *pValue =
        fxv8::NewBooleanHelper(pIsolate, GetContent(true).EqualsASCII("1"));
    return;
  }

  ByteString newValue;
  if (pValue && !(fxv8::IsNull(*pValue) || fxv8::IsUndefined(*pValue)))
    newValue = fxv8::ReentrantToByteStringHelper(pIsolate, *pValue);

  int32_t iValue = FXSYS_atoi(newValue.c_str());
  WideString wsNewValue(iValue == 0 ? L"0" : L"1");
  WideString wsFormatValue(wsNewValue);
  CXFA_Node* pContainerNode = GetXFANode()->GetContainerNode();
  if (pContainerNode)
    wsFormatValue = pContainerNode->GetFormatDataValue(wsNewValue);

  SetContent(wsNewValue, wsFormatValue, true, true, true);
}

void CJX_Boolean::value(v8::Isolate* pIsolate,
                        v8::Local<v8::Value>* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  defaultValue(pIsolate, pValue, bSetting, eAttribute);
}
