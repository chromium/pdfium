// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_treelist.h"

#include "core/fxcrt/span.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_treelist.h"

const CJX_MethodSpec CJX_TreeList::MethodSpecs[] = {
    {"namedItem", namedItem_static}};

CJX_TreeList::CJX_TreeList(CXFA_TreeList* list) : CJX_List(list) {
  DefineMethods(MethodSpecs);
}

CJX_TreeList::~CJX_TreeList() = default;

bool CJX_TreeList::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CXFA_TreeList* CJX_TreeList::GetXFATreeList() {
  return ToTreeList(GetXFAObject());
}

CJS_Result CJX_TreeList::namedItem(CFXJSE_Engine* runtime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pNode = GetXFATreeList()->NamedItem(
      runtime->ToWideString(params[0]).AsStringView());
  if (!pNode)
    return CJS_Result::Success();

  return CJS_Result::Success(runtime->GetOrCreateJSBindingFromMap(pNode));
}
