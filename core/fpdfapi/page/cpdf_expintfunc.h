// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_
#define CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_

#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fxcrt/data_vector.h"

#if defined(PDF_USE_SKIA)
#include "core/fxcrt/span.h"
#endif

class CPDF_ExpIntFunc final : public CPDF_Function {
 public:
  CPDF_ExpIntFunc();
  ~CPDF_ExpIntFunc() override;

  // CPDF_Function:
  bool v_Init(const CPDF_Object* pObj, VisitedSet* pVisited) override;
  bool v_Call(pdfium::span<const float> inputs,
              pdfium::span<float> results) const override;

  uint32_t GetOrigOutputs() const { return orig_outputs_; }
  float GetExponent() const { return exponent_; }

#if defined(PDF_USE_SKIA)
  pdfium::span<const float> GetBeginValues() const { return begin_values_; }
  pdfium::span<const float> GetEndValues() const { return end_values_; }
#endif

 private:
  uint32_t orig_outputs_ = 0;
  float exponent_ = 0.0f;
  DataVector<float> begin_values_;
  DataVector<float> end_values_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_
