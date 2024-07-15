// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_list.h"

#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_list.h"
#include "xfa/fxfa/parser/cxfa_node.h"

const CJX_MethodSpec CJX_List::MethodSpecs[] = {{"append", append_static},
                                                {"insert", insert_static},
                                                {"item", item_static},
                                                {"remove", remove_static}};

CJX_List::CJX_List(CXFA_List* list) : CJX_Object(list) {
  DefineMethods(MethodSpecs);
}

CJX_List::~CJX_List() = default;

bool CJX_List::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CXFA_List* CJX_List::GetXFAList() {
  return ToList(GetXFAObject());
}

CJS_Result CJX_List::append(CFXJSE_Engine* runtime,
                            pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  auto* pNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNode)
    return CJS_Result::Failure(JSMessage::kValueError);

  if (!GetXFAList()->Append(pNode))
    return CJS_Result::Failure(JSMessage::kWouldBeCyclic);

  return CJS_Result::Success();
}

CJS_Result CJX_List::insert(CFXJSE_Engine* runtime,
                            pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  auto* pNewNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNewNode)
    return CJS_Result::Failure(JSMessage::kValueError);

  auto* pBeforeNode = ToNode(runtime->ToXFAObject(params[1]));
  if (!GetXFAList()->Insert(pNewNode, pBeforeNode))
    return CJS_Result::Failure(JSMessage::kValueError);

  return CJS_Result::Success();
}

CJS_Result CJX_List::remove(CFXJSE_Engine* runtime,
                            pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  auto* pNode = ToNode(runtime->ToXFAObject(params[0]));
  if (!pNode)
    return CJS_Result::Failure(JSMessage::kValueError);

  GetXFAList()->Remove(pNode);
  return CJS_Result::Success();
}

CJS_Result CJX_List::item(CFXJSE_Engine* runtime,
                          pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  int32_t index = runtime->ToInt32(params[0]);
  size_t cast_index = static_cast<size_t>(index);
  if (index < 0 || cast_index >= GetXFAList()->GetLength())
    return CJS_Result::Failure(JSMessage::kInvalidInputError);

  return CJS_Result::Success(
      runtime->NewNormalXFAObject(GetXFAList()->Item(cast_index)));
}

void CJX_List::length(v8::Isolate* pIsolate,
                      v8::Local<v8::Value>* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  *pValue = fxv8::NewNumberHelper(
      pIsolate, pdfium::checked_cast<int32_t>(GetXFAList()->GetLength()));
}
