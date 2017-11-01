// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_signaturepseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cscript_signaturepseudomodel.h"

CJX_SignaturePseudoModel::CJX_SignaturePseudoModel(
    CScript_SignaturePseudoModel* model)
    : CJX_Object(model) {}

CJX_SignaturePseudoModel::~CJX_SignaturePseudoModel() {}

void CJX_SignaturePseudoModel::Verify(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 4) {
    ThrowParamCountMismatchException(L"verify");
    return;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetInteger(0);
}

void CJX_SignaturePseudoModel::Sign(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 3 || iLength > 7) {
    ThrowParamCountMismatchException(L"sign");
    return;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(false);
}

void CJX_SignaturePseudoModel::Enumerate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"enumerate");
    return;
  }
  return;
}

void CJX_SignaturePseudoModel::Clear(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowParamCountMismatchException(L"clear");
    return;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(false);
}
