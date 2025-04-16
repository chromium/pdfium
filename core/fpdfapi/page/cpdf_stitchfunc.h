// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_STITCHFUNC_H_
#define CORE_FPDFAPI_PAGE_CPDF_STITCHFUNC_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_function.h"

class CPDF_StitchFunc final : public CPDF_Function {
 public:
  CPDF_StitchFunc();
  ~CPDF_StitchFunc() override;

  // CPDF_Function:
  bool v_Init(const CPDF_Object* pObj, VisitedSet* pVisited) override;
  bool v_Call(pdfium::span<const float> inputs,
              pdfium::span<float> results) const override;

  const std::vector<std::unique_ptr<CPDF_Function>>& GetSubFunctions() const {
    return sub_functions_;
  }
  float GetBound(size_t i) const { return bounds_[i]; }
  float GetEncode(size_t i) const { return encode_[i]; }

 private:
  std::vector<std::unique_ptr<CPDF_Function>> sub_functions_;
  std::vector<float> bounds_;
  std::vector<float> encode_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_STITCHFUNC_H_
