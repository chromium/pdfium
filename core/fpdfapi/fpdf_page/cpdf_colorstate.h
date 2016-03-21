// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_

#include "core/fpdfapi/fpdf_page/cpdf_colorstatedata.h"
#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_Color;
class CPDF_ColorSpace;
class CPDF_Pattern;

class CPDF_ColorState : public CFX_CountRef<CPDF_ColorStateData> {
 public:
  CPDF_Color* GetFillColor() const {
    return m_pObject ? &m_pObject->m_FillColor : nullptr;
  }

  CPDF_Color* GetStrokeColor() const {
    return m_pObject ? &m_pObject->m_StrokeColor : nullptr;
  }

  void SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);
  void SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);
  void SetFillPattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);
  void SetStrokePattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);

 private:
  void SetColor(CPDF_Color& color,
                FX_DWORD& rgb,
                CPDF_ColorSpace* pCS,
                FX_FLOAT* pValue,
                int nValues);
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_COLORSTATE_H_
