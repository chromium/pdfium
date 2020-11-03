// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_draw.h"

#include "fxjs/xfa/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_draw.h"

CJX_Draw::CJX_Draw(CXFA_Draw* node) : CJX_Container(node) {}

CJX_Draw::~CJX_Draw() = default;

bool CJX_Draw::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Draw::rawValue(v8::Isolate* pIsolate,
                        CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  defaultValue(pIsolate, pValue, bSetting, eAttribute);
}

void CJX_Draw::defaultValue(v8::Isolate* pIsolate,
                            CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute) {
  if (!bSetting) {
    WideString content = GetContent(true);
    if (content.IsEmpty())
      pValue->SetNull(pIsolate);
    else
      pValue->SetString(pIsolate, content.ToUTF8().AsStringView());
    return;
  }

  if (!pValue || !pValue->IsString(pIsolate))
    return;

  ASSERT(GetXFANode()->IsWidgetReady());
  if (GetXFANode()->GetFFWidgetType() != XFA_FFWidgetType::kText)
    return;

  WideString wsNewValue = pValue->ToWideString(pIsolate);
  SetContent(wsNewValue, wsNewValue, true, true, true);
}
