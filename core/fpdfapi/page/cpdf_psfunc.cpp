// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_psfunc.h"

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"

CPDF_PSFunc::CPDF_PSFunc() : CPDF_Function(Type::kType4PostScript) {}

CPDF_PSFunc::~CPDF_PSFunc() = default;

bool CPDF_PSFunc::v_Init(const CPDF_Object* pObj, VisitedSet* pVisited) {
  auto pAcc =
      pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(pObj->AsStream()));
  pAcc->LoadAllDataFiltered();
  return ps_.Parse(pAcc->GetSpan());
}

bool CPDF_PSFunc::v_Call(pdfium::span<const float> inputs,
                         pdfium::span<float> results) const {
  ps_.Reset();
  for (uint32_t i = 0; i < inputs_; i++) {
    ps_.Push(inputs[i]);
  }
  ps_.Execute();
  if (ps_.GetStackSize() < outputs_) {
    return false;
  }
  for (uint32_t i = 0; i < outputs_; i++) {
    results[outputs_ - i - 1] = ps_.Pop();
  }
  return true;
}
