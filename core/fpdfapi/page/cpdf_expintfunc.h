// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_
#define CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_

#include "core/fpdfapi/page/cpdf_function.h"

class CPDF_ExpIntFunc : public CPDF_Function {
 public:
  CPDF_ExpIntFunc();
  ~CPDF_ExpIntFunc() override;

  // CPDF_Function
  bool v_Init(CPDF_Object* pObj) override;
  bool v_Call(float* inputs, float* results) const override;

  uint32_t m_nOrigOutputs;
  float m_Exponent;
  float* m_pBeginValues;
  float* m_pEndValues;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_EXPINTFUNC_H_
