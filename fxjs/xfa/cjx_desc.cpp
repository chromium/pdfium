// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_desc.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_desc.h"

const CJX_MethodSpec CJX_Desc::MethodSpecs[] = {{"metadata", metadata_static},
                                                {"", nullptr}};

CJX_Desc::CJX_Desc(CXFA_Desc* desc) : CJX_Node(desc) {
  DefineMethods(MethodSpecs);
}

CJX_Desc::~CJX_Desc() {}

void CJX_Desc::metadata(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"metadata");
    return;
  }
  pArguments->GetReturnValue()->SetString("");
}
