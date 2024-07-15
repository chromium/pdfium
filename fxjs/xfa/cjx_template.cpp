// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_template.h"

#include "core/fxcrt/span.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_template.h"

const CJX_MethodSpec CJX_Template::MethodSpecs[] = {
    {"execCalculate", execCalculate_static},
    {"execInitialize", execInitialize_static},
    {"execValidate", execValidate_static},
    {"formNodes", formNodes_static},
    {"recalculate", recalculate_static},
    {"remerge", remerge_static}};

CJX_Template::CJX_Template(CXFA_Template* tmpl) : CJX_Model(tmpl) {
  DefineMethods(MethodSpecs);
}

CJX_Template::~CJX_Template() = default;

bool CJX_Template::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Template::formNodes(CFXJSE_Engine* runtime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewBoolean(true));
}

CJS_Result CJX_Template::remerge(CFXJSE_Engine* runtime,
                                 pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  GetDocument()->DoDataRemerge();
  return CJS_Result::Success();
}

CJS_Result CJX_Template::execInitialize(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(
      runtime->NewBoolean(GetXFANode()->IsWidgetReady()));
}

CJS_Result CJX_Template::recalculate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewBoolean(true));
}

CJS_Result CJX_Template::execCalculate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(
      runtime->NewBoolean(GetXFANode()->IsWidgetReady()));
}

CJS_Result CJX_Template::execValidate(
    CFXJSE_Engine* runtime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!params.empty())
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(
      runtime->NewBoolean(GetXFANode()->IsWidgetReady()));
}
