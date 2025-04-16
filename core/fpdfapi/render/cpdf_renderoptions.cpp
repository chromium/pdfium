// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_renderoptions.h"

namespace {

constexpr uint32_t kCacheSizeLimitBytes = 100 * 1024 * 1024;

}  // namespace

CPDF_RenderOptions::Options::Options() = default;

CPDF_RenderOptions::Options::Options(const CPDF_RenderOptions::Options& rhs) =
    default;

CPDF_RenderOptions::Options& CPDF_RenderOptions::Options::operator=(
    const CPDF_RenderOptions::Options& rhs) = default;

CPDF_RenderOptions::CPDF_RenderOptions() {
  // TODO(thestig): Make constexpr to initialize |options_| once C++14 is
  // available.
  options_.bClearType = true;
}

CPDF_RenderOptions::CPDF_RenderOptions(const CPDF_RenderOptions& rhs) = default;

CPDF_RenderOptions::~CPDF_RenderOptions() = default;

FX_ARGB CPDF_RenderOptions::TranslateColor(FX_ARGB argb) const {
  if (ColorModeIs(kNormal)) {
    return argb;
  }
  if (ColorModeIs(kAlpha)) {
    return argb;
  }

  const FX_BGRA_STRUCT<uint8_t> bgra = ArgbToBGRAStruct(argb);
  const int gray = FXRGB2GRAY(bgra.red, bgra.green, bgra.blue);
  return ArgbEncode(bgra.alpha, gray, gray, gray);
}

FX_ARGB CPDF_RenderOptions::TranslateObjectFillColor(
    FX_ARGB argb,
    CPDF_PageObject::Type object_type) const {
  if (!ColorModeIs(kForcedColor)) {
    return TranslateColor(argb);
  }
  switch (object_type) {
    case CPDF_PageObject::Type::kPath:
      return color_scheme_.path_fill_color;
    case CPDF_PageObject::Type::kText:
      return color_scheme_.text_fill_color;
    default:
      return argb;
  }
}

FX_ARGB CPDF_RenderOptions::TranslateObjectStrokeColor(
    FX_ARGB argb,
    CPDF_PageObject::Type object_type) const {
  if (!ColorModeIs(kForcedColor)) {
    return TranslateColor(argb);
  }
  switch (object_type) {
    case CPDF_PageObject::Type::kPath:
      return color_scheme_.path_stroke_color;
    case CPDF_PageObject::Type::kText:
      return color_scheme_.text_stroke_color;
    default:
      return argb;
  }
}

uint32_t CPDF_RenderOptions::GetCacheSizeLimit() const {
  return kCacheSizeLimitBytes;
}

bool CPDF_RenderOptions::CheckOCGDictVisible(const CPDF_Dictionary* pOC) const {
  return !oc_context_ || oc_context_->CheckOCGDictVisible(pOC);
}

bool CPDF_RenderOptions::CheckPageObjectVisible(
    const CPDF_PageObject* pPageObj) const {
  return !oc_context_ || oc_context_->CheckPageObjectVisible(pPageObj);
}
