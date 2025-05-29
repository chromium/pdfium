// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_expintfunc.h"

#include <math.h>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/stl_util.h"

CPDF_ExpIntFunc::CPDF_ExpIntFunc()
    : CPDF_Function(Type::kType2ExponentialInterpolation) {}

CPDF_ExpIntFunc::~CPDF_ExpIntFunc() = default;

bool CPDF_ExpIntFunc::v_Init(const CPDF_Object* pObj, VisitedSet* pVisited) {
  CHECK(pObj->IsDictionary() || pObj->IsStream());
  RetainPtr<const CPDF_Dictionary> dict = pObj->GetDict();
  RetainPtr<const CPDF_Number> pExponent = dict->GetNumberFor("N");
  if (!pExponent) {
    return false;
  }

  exponent_ = pExponent->GetNumber();

  RetainPtr<const CPDF_Array> pArray0 = dict->GetArrayFor("C0");
  if (pArray0 && outputs_ == 0) {
    outputs_ = fxcrt::CollectionSize<uint32_t>(*pArray0);
  }
  if (outputs_ == 0) {
    outputs_ = 1;
  }

  RetainPtr<const CPDF_Array> pArray1 = dict->GetArrayFor("C1");
  begin_values_ = DataVector<float>(Fx2DSizeOrDie(outputs_, 2));
  end_values_ = DataVector<float>(begin_values_.size());
  for (uint32_t i = 0; i < outputs_; i++) {
    begin_values_[i] = pArray0 ? pArray0->GetFloatAt(i) : 0.0f;
    end_values_[i] = pArray1 ? pArray1->GetFloatAt(i) : 1.0f;
  }

  FX_SAFE_UINT32 nOutputs = outputs_;
  nOutputs *= inputs_;
  if (!nOutputs.IsValid()) {
    return false;
  }

  orig_outputs_ = outputs_;
  outputs_ = nOutputs.ValueOrDie();
  return true;
}

bool CPDF_ExpIntFunc::v_Call(pdfium::span<const float> inputs,
                             pdfium::span<float> results) const {
  for (uint32_t i = 0; i < inputs_; i++) {
    for (uint32_t j = 0; j < orig_outputs_; j++) {
      results[i * orig_outputs_ + j] =
          begin_values_[j] +
          powf(inputs[i], exponent_) * (end_values_[j] - begin_values_[j]);
    }
  }
  return true;
}
