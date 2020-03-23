// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_color.h"

#include "core/fpdfapi/page/cpdf_patterncs.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/ptr_util.h"

CPDF_Color::CPDF_Color() = default;

CPDF_Color::CPDF_Color(const CPDF_Color& that) {
  *this = that;
}

CPDF_Color::~CPDF_Color() = default;

bool CPDF_Color::IsPattern() const {
  return m_pCS && IsPatternInternal();
}

bool CPDF_Color::IsPatternInternal() const {
  return m_pCS->GetFamily() == PDFCS_PATTERN;
}

void CPDF_Color::SetColorSpace(const RetainPtr<CPDF_ColorSpace>& pCS) {
  m_pCS = pCS;
  if (IsPatternInternal()) {
    m_Buffer.clear();
    m_pValue = pdfium::MakeUnique<PatternValue>();
  } else {
    m_Buffer = pCS->CreateBufAndSetDefaultColor();
    m_pValue.reset();
  }
}

void CPDF_Color::SetValueForNonPattern(const std::vector<float>& values) {
  ASSERT(!IsPatternInternal());
  ASSERT(m_pCS->CountComponents() <= values.size());
  m_Buffer = values;
}

void CPDF_Color::SetValueForPattern(const RetainPtr<CPDF_Pattern>& pPattern,
                                    const std::vector<float>& values) {
  if (values.size() > kMaxPatternColorComps)
    return;

  if (!IsPattern())
    SetColorSpace(CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN));

  m_pValue->SetPattern(pPattern);
  m_pValue->SetComps(values);
}

CPDF_Color& CPDF_Color::operator=(const CPDF_Color& that) {
  if (this == &that)
    return *this;

  m_Buffer = that.m_Buffer;
  m_pValue = that.m_pValue ? pdfium::MakeUnique<PatternValue>(*that.m_pValue)
                           : nullptr;
  m_pCS = that.m_pCS;
  return *this;
}

uint32_t CPDF_Color::CountComponents() const {
  return m_pCS->CountComponents();
}

bool CPDF_Color::IsColorSpaceRGB() const {
  return m_pCS == CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
}

bool CPDF_Color::GetRGB(int* R, int* G, int* B) const {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  bool result = false;
  if (IsPatternInternal()) {
    if (m_pValue) {
      const CPDF_PatternCS* pPatternCS = m_pCS->AsPatternCS();
      result = pPatternCS->GetPatternRGB(*m_pValue, &r, &g, &b);
    }
  } else {
    if (!m_Buffer.empty())
      result = m_pCS->GetRGB(m_Buffer.data(), &r, &g, &b);
  }
  if (!result)
    return false;

  *R = static_cast<int32_t>(r * 255 + 0.5f);
  *G = static_cast<int32_t>(g * 255 + 0.5f);
  *B = static_cast<int32_t>(b * 255 + 0.5f);
  return true;
}

CPDF_Pattern* CPDF_Color::GetPattern() const {
  ASSERT(IsPattern());
  return m_pValue ? m_pValue->GetPattern() : nullptr;
}
