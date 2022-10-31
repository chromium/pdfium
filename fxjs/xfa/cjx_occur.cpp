// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_occur.h"

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cxfa_occur.h"

CJX_Occur::CJX_Occur(CXFA_Occur* node) : CJX_Node(node) {}

CJX_Occur::~CJX_Occur() = default;

bool CJX_Occur::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Occur::max(v8::Isolate* pIsolate,
                    v8::Local<v8::Value>* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {
  CXFA_Occur* occur = static_cast<CXFA_Occur*>(GetXFANode());
  if (!bSetting) {
    *pValue = fxv8::NewNumberHelper(pIsolate, occur->GetMax());
    return;
  }
  occur->SetMax(fxv8::ReentrantToInt32Helper(pIsolate, *pValue));
}

// NOLINTNEXTLINE(build/include_what_you_use)
void CJX_Occur::min(v8::Isolate* pIsolate,
                    v8::Local<v8::Value>* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {
  CXFA_Occur* occur = static_cast<CXFA_Occur*>(GetXFANode());
  if (!bSetting) {
    *pValue = fxv8::NewNumberHelper(pIsolate, occur->GetMin());
    return;
  }
  occur->SetMin(fxv8::ReentrantToInt32Helper(pIsolate, *pValue));
}
