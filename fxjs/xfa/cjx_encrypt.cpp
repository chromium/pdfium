// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_encrypt.h"

#include "xfa/fxfa/parser/cxfa_encrypt.h"

CJX_Encrypt::CJX_Encrypt(CXFA_Encrypt* node) : CJX_Node(node) {}

CJX_Encrypt::~CJX_Encrypt() = default;

bool CJX_Encrypt::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_Encrypt::format(v8::Isolate* pIsolate,
                         v8::Local<v8::Value>* pValue,
                         bool bSetting,
                         XFA_Attribute eAttribute) {}
