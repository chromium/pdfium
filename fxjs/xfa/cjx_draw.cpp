// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_draw.h"

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "third_party/base/check.h"
#include "v8/include/v8-primitive.h"
#include "v8/include/v8-value.h"
#include "xfa/fxfa/parser/cxfa_draw.h"

CJX_Draw::CJX_Draw(CXFA_Draw* node) : CJX_Container(node) {}

CJX_Draw::~CJX_Draw() = default;

bool CJX_Draw::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Draw::rawValue(v8::Isolate* pIsolate,
                        v8::Local<v8::Value>* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  defaultValue(pIsolate, pValue, bSetting, eAttribute);
}

void CJX_Draw::defaultValue(v8::Isolate* pIsolate,
                            v8::Local<v8::Value>* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute) {
  if (!bSetting) {
    ByteString content = GetContent(true).ToUTF8();
    *pValue = content.IsEmpty()
                  ? fxv8::NewNullHelper(pIsolate).As<v8::Value>()
                  : fxv8::NewStringHelper(pIsolate, content.AsStringView())
                        .As<v8::Value>();
    return;
  }

  if (!pValue || !fxv8::IsString(*pValue))
    return;

  DCHECK(GetXFANode()->IsWidgetReady());
  if (GetXFANode()->GetFFWidgetType() != XFA_FFWidgetType::kText)
    return;

  WideString wsNewValue = fxv8::ReentrantToWideStringHelper(pIsolate, *pValue);
  SetContent(wsNewValue, wsNewValue, true, true, true);
}
