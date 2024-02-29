// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_color.h"

#include <optional>
#include <utility>

#include "core/fpdfapi/page/cpdf_patterncs.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"

CPDF_Color::CPDF_Color() = default;

CPDF_Color::CPDF_Color(const CPDF_Color& that) {
  *this = that;
}

CPDF_Color::~CPDF_Color() = default;

bool CPDF_Color::IsPattern() const {
  return m_pCS && IsPatternInternal();
}

bool CPDF_Color::IsPatternInternal() const {
  return m_pCS->GetFamily() == CPDF_ColorSpace::Family::kPattern;
}

void CPDF_Color::SetColorSpace(RetainPtr<CPDF_ColorSpace> colorspace) {
  m_pCS = std::move(colorspace);
  if (IsPatternInternal()) {
    m_Buffer.clear();
    m_pValue = std::make_unique<PatternValue>();
  } else {
    m_Buffer = m_pCS->CreateBufAndSetDefaultColor();
    m_pValue.reset();
  }
}

void CPDF_Color::SetValueForNonPattern(std::vector<float> values) {
  CHECK(!IsPatternInternal());
  CHECK_LE(m_pCS->ComponentCount(), values.size());
  m_Buffer = std::move(values);
}

void CPDF_Color::SetValueForPattern(RetainPtr<CPDF_Pattern> pattern,
                                    pdfium::span<float> values) {
  if (values.size() > kMaxPatternColorComps)
    return;

  if (!IsPattern()) {
    SetColorSpace(
        CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kPattern));
  }
  m_pValue->SetPattern(std::move(pattern));
  m_pValue->SetComps(values);
}

CPDF_Color& CPDF_Color::operator=(const CPDF_Color& that) {
  if (this == &that)
    return *this;

  m_Buffer = that.m_Buffer;
  m_pValue =
      that.m_pValue ? std::make_unique<PatternValue>(*that.m_pValue) : nullptr;
  m_pCS = that.m_pCS;
  return *this;
}

uint32_t CPDF_Color::ComponentCount() const {
  return m_pCS->ComponentCount();
}

bool CPDF_Color::IsColorSpaceRGB() const {
  return m_pCS ==
         CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);
}

std::optional<FX_COLORREF> CPDF_Color::GetRGB() const {
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
      result = m_pCS->GetRGB(m_Buffer, &r, &g, &b);
  }
  if (!result) {
    return std::nullopt;
  }

  return FXSYS_BGR(static_cast<int32_t>(b * 255 + 0.5f),
                   static_cast<int32_t>(g * 255 + 0.5f),
                   static_cast<int32_t>(r * 255 + 0.5f));
}

RetainPtr<CPDF_Pattern> CPDF_Color::GetPattern() const {
  DCHECK(IsPattern());
  return m_pValue ? m_pValue->GetPattern() : nullptr;
}
