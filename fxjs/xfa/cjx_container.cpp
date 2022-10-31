// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_container.h"

#include <vector>

#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/cppgc/allocation.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_field.h"

const CJX_MethodSpec CJX_Container::MethodSpecs[] = {
    {"getDelta", getDelta_static},
    {"getDeltas", getDeltas_static}};

CJX_Container::CJX_Container(CXFA_Node* node) : CJX_Node(node) {
  DefineMethods(MethodSpecs);
}

CJX_Container::~CJX_Container() = default;

bool CJX_Container::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Container::getDelta(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success();
}

CJS_Result CJX_Container::getDeltas(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_Document* pDoc = GetDocument();
  auto* pList = cppgc::MakeGarbageCollected<CXFA_ArrayNodeList>(
      pDoc->GetHeap()->GetAllocationHandle(), pDoc);
  pDoc->GetNodeOwner()->PersistList(pList);
  return CJS_Result::Success(runtime->NewNormalXFAObject(pList));
}
