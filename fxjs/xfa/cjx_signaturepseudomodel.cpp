// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_signaturepseudomodel.h"

#include <vector>

#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"

const CJX_MethodSpec CJX_SignaturePseudoModel::MethodSpecs[] = {
    {"verify", verifySignature_static},
    {"sign", sign_static},
    {"enumerate", enumerate_static},
    {"clear", clear_static}};

CJX_SignaturePseudoModel::CJX_SignaturePseudoModel(
    CScript_SignaturePseudoModel* model)
    : CJX_Object(model) {
  DefineMethods(MethodSpecs);
}

CJX_SignaturePseudoModel::~CJX_SignaturePseudoModel() = default;

bool CJX_SignaturePseudoModel::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_SignaturePseudoModel::verifySignature(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 4)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_SignaturePseudoModel::sign(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() < 3 || params.size() > 7)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewBoolean(false));
}

CJS_Result CJX_SignaturePseudoModel::enumerate(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_SignaturePseudoModel::clear(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewBoolean(false));
}
