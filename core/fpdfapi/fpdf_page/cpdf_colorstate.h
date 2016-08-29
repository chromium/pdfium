// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_

#include "core/fpdfapi/fpdf_page/cpdf_colorstatedata.h"
#include "core/fxcrt/include/cfx_count_ref.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Color;
class CPDF_ColorSpace;
class CPDF_Pattern;

class CPDF_ColorState : public CFX_CountRef<CPDF_ColorStateData> {
 public:
  const CPDF_Color* GetFillColor() const {
    const CPDF_ColorStateData* pData = GetObject();
    return pData ? &pData->m_FillColor : nullptr;
  }

  const CPDF_Color* GetStrokeColor() const {
    const CPDF_ColorStateData* pData = GetObject();
    return pData ? &pData->m_StrokeColor : nullptr;
  }

  void SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, uint32_t nValues);
  void SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, uint32_t nValues);
  void SetFillPattern(CPDF_Pattern* pattern,
                      FX_FLOAT* pValue,
                      uint32_t nValues);
  void SetStrokePattern(CPDF_Pattern* pattern,
                        FX_FLOAT* pValue,
                        uint32_t nValues);

 private:
  void SetColor(CPDF_Color& color,
                uint32_t& rgb,
                CPDF_ColorSpace* pCS,
                FX_FLOAT* pValue,
                uint32_t nValues);
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
