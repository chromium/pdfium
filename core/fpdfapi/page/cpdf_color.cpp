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
  return cs_ && IsPatternInternal();
}

bool CPDF_Color::IsPatternInternal() const {
  return cs_->GetFamily() == CPDF_ColorSpace::Family::kPattern;
}

void CPDF_Color::SetColorSpace(RetainPtr<CPDF_ColorSpace> colorspace) {
  cs_ = std::move(colorspace);
  if (IsPatternInternal()) {
    buffer_.clear();
    value_ = std::make_unique<PatternValue>();
  } else {
    buffer_ = cs_->CreateBufAndSetDefaultColor();
    value_.reset();
  }
}

void CPDF_Color::SetValueForNonPattern(std::vector<float> values) {
  CHECK(!IsPatternInternal());
  CHECK_LE(cs_->ComponentCount(), values.size());
  buffer_ = std::move(values);
}

void CPDF_Color::SetValueForPattern(RetainPtr<CPDF_Pattern> pattern,
                                    pdfium::span<float> values) {
  if (values.size() > kMaxPatternColorComps) {
    return;
  }

  if (!IsPattern()) {
    SetColorSpace(
        CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kPattern));
  }
  value_->SetPattern(std::move(pattern));
  value_->SetComps(values);
}

CPDF_Color& CPDF_Color::operator=(const CPDF_Color& that) {
  if (this == &that) {
    return *this;
  }

  buffer_ = that.buffer_;
  value_ = that.value_ ? std::make_unique<PatternValue>(*that.value_) : nullptr;
  cs_ = that.cs_;
  return *this;
}

uint32_t CPDF_Color::ComponentCount() const {
  return cs_->ComponentCount();
}

bool CPDF_Color::IsColorSpaceRGB() const {
  return cs_ ==
         CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);
}

bool CPDF_Color::IsColorSpaceGray() const {
  return cs_ ==
         CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray);
}

std::optional<FX_COLORREF> CPDF_Color::GetColorRef() const {
  if (IsPatternInternal()) {
    if (value_) {
      return cs_->AsPatternCS()->GetPatternColorRef(*value_);
    }
  } else {
    if (!buffer_.empty()) {
      return cs_->GetColorRef(buffer_);
    }
  }
  return std::nullopt;
}

RetainPtr<CPDF_Pattern> CPDF_Color::GetPattern() const {
  DCHECK(IsPattern());
  return value_ ? value_->GetPattern() : nullptr;
}
