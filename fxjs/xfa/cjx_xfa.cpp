// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_xfa.h"

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_xfa.h"

CJX_Xfa::CJX_Xfa(CXFA_Xfa* node) : CJX_Model(node) {}

CJX_Xfa::~CJX_Xfa() = default;

bool CJX_Xfa::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Xfa::thisValue(v8::Isolate* pIsolate,
                        v8::Local<v8::Value>* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  if (bSetting)
    return;

  auto* pScriptContext = GetDocument()->GetScriptContext();
  CXFA_Object* pThis = pScriptContext->GetThisObject();
  *pValue =
      pThis ? pScriptContext->GetOrCreateJSBindingFromMap(pThis).As<v8::Value>()
            : fxv8::NewNullHelper(pIsolate);
}
