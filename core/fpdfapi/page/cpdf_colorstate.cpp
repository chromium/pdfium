// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_colorstate.h"

#include <optional>
#include <utility>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_pattern.h"
#include "core/fpdfapi/page/cpdf_tilingpattern.h"
#include "core/fxge/dib/fx_dib.h"

CPDF_ColorState::CPDF_ColorState() = default;

CPDF_ColorState::CPDF_ColorState(const CPDF_ColorState& that) = default;

CPDF_ColorState::~CPDF_ColorState() = default;

void CPDF_ColorState::Emplace() {
  ref_.Emplace();
}

void CPDF_ColorState::SetDefault() {
  ref_.GetPrivateCopy()->SetDefault();
}

FX_COLORREF CPDF_ColorState::GetFillColorRef() const {
  return ref_.GetObject()->fill_color_ref_;
}

void CPDF_ColorState::SetFillColorRef(FX_COLORREF colorref) {
  if (!ref_ || GetFillColorRef() != colorref) {
    ref_.GetPrivateCopy()->fill_color_ref_ = colorref;
  }
}

FX_COLORREF CPDF_ColorState::GetStrokeColorRef() const {
  return ref_.GetObject()->stroke_color_ref_;
}

void CPDF_ColorState::SetStrokeColorRef(FX_COLORREF colorref) {
  if (!ref_ || GetStrokeColorRef() != colorref) {
    ref_.GetPrivateCopy()->stroke_color_ref_ = colorref;
  }
}

const CPDF_Color* CPDF_ColorState::GetFillColor() const {
  const ColorData* data = ref_.GetObject();
  return data ? &data->fill_color_ : nullptr;
}

CPDF_Color* CPDF_ColorState::GetMutableFillColor() {
  return &ref_.GetPrivateCopy()->fill_color_;
}

bool CPDF_ColorState::HasFillColor() const {
  const CPDF_Color* pColor = GetFillColor();
  return pColor && !pColor->IsNull();
}

const CPDF_Color* CPDF_ColorState::GetStrokeColor() const {
  const ColorData* data = ref_.GetObject();
  return data ? &data->stroke_color_ : nullptr;
}

CPDF_Color* CPDF_ColorState::GetMutableStrokeColor() {
  return &ref_.GetPrivateCopy()->stroke_color_;
}

bool CPDF_ColorState::HasStrokeColor() const {
  const CPDF_Color* pColor = GetStrokeColor();
  return pColor && !pColor->IsNull();
}

void CPDF_ColorState::SetFillColor(RetainPtr<CPDF_ColorSpace> colorspace,
                                   std::vector<float> values) {
  ColorData* data = ref_.GetPrivateCopy();
  std::optional<FX_COLORREF> colorref =
      SetColor(std::move(colorspace), std::move(values), data->fill_color_);
  if (colorref.has_value()) {
    data->fill_color_ref_ = colorref.value();
  }
}

void CPDF_ColorState::SetStrokeColor(RetainPtr<CPDF_ColorSpace> colorspace,
                                     std::vector<float> values) {
  ColorData* data = ref_.GetPrivateCopy();
  std::optional<FX_COLORREF> colorref =
      SetColor(std::move(colorspace), std::move(values), data->stroke_color_);
  if (colorref.has_value()) {
    data->stroke_color_ref_ = colorref.value();
  }
}

void CPDF_ColorState::SetFillPattern(RetainPtr<CPDF_Pattern> pattern,
                                     pdfium::span<float> values) {
  ColorData* data = ref_.GetPrivateCopy();
  data->fill_color_ref_ =
      SetPattern(std::move(pattern), values, data->fill_color_);
}

void CPDF_ColorState::SetStrokePattern(RetainPtr<CPDF_Pattern> pattern,
                                       pdfium::span<float> values) {
  ColorData* data = ref_.GetPrivateCopy();
  data->stroke_color_ref_ =
      SetPattern(std::move(pattern), values, data->stroke_color_);
}

std::optional<FX_COLORREF> CPDF_ColorState::SetColor(
    RetainPtr<CPDF_ColorSpace> colorspace,
    std::vector<float> values,
    CPDF_Color& color) {
  if (colorspace) {
    color.SetColorSpace(std::move(colorspace));
  } else if (color.IsNull()) {
    color.SetColorSpace(
        CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray));
  }
  if (color.ComponentCount() > values.size()) {
    return std::nullopt;
  }

  if (!color.IsPattern()) {
    color.SetValueForNonPattern(std::move(values));
  }
  return color.GetColorRef().value_or(0xFFFFFFFF);
}

FX_COLORREF CPDF_ColorState::SetPattern(RetainPtr<CPDF_Pattern> pattern,
                                        pdfium::span<float> values,
                                        CPDF_Color& color) {
  color.SetValueForPattern(pattern, values);
  std::optional<FX_COLORREF> colorref = color.GetColorRef();
  if (colorref.has_value()) {
    return colorref.value();
  }

  CPDF_TilingPattern* tiling = pattern->AsTilingPattern();
  return tiling && tiling->colored() ? 0x00BFBFBF : 0xFFFFFFFF;
}

CPDF_ColorState::ColorData::ColorData() = default;

CPDF_ColorState::ColorData::ColorData(const ColorData& src)
    : fill_color_ref_(src.fill_color_ref_),
      stroke_color_ref_(src.stroke_color_ref_),
      fill_color_(src.fill_color_),
      stroke_color_(src.stroke_color_) {}

CPDF_ColorState::ColorData::~ColorData() = default;

void CPDF_ColorState::ColorData::SetDefault() {
  fill_color_ref_ = 0;
  stroke_color_ref_ = 0;
  fill_color_.SetColorSpace(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray));
  stroke_color_.SetColorSpace(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray));
}

RetainPtr<CPDF_ColorState::ColorData> CPDF_ColorState::ColorData::Clone()
    const {
  return pdfium::MakeRetain<CPDF_ColorState::ColorData>(*this);
}
