// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_desc.h"

#include "core/fxcrt/span.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fxfa/parser/cxfa_desc.h"

const CJX_MethodSpec CJX_Desc::MethodSpecs[] = {{"metadata", metadata_static}};

CJX_Desc::CJX_Desc(CXFA_Desc* desc) : CJX_Node(desc) {
  DefineMethods(MethodSpecs);
}

CJX_Desc::~CJX_Desc() = default;

bool CJX_Desc::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Desc::metadata(CFXJSE_Engine* runtime,
                              pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() != 0 && params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success(runtime->NewString(""));
}
