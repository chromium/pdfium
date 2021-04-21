// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_iconfit.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/fx_string.h"

namespace {

constexpr float kDefaultPosition = 0.5f;

}  // namespace

CPDF_IconFit::CPDF_IconFit(const CPDF_Dictionary* pDict) : m_pDict(pDict) {}

CPDF_IconFit::CPDF_IconFit(const CPDF_IconFit& that) = default;

CPDF_IconFit::~CPDF_IconFit() = default;

CPDF_IconFit::ScaleMethod CPDF_IconFit::GetScaleMethod() const {
  if (!m_pDict)
    return ScaleMethod::kAlways;

  ByteString csSW = m_pDict->GetStringFor("SW", "A");
  if (csSW == "B")
    return ScaleMethod::kBigger;
  if (csSW == "S")
    return ScaleMethod::kSmaller;
  if (csSW == "N")
    return ScaleMethod::kNever;
  return ScaleMethod::kAlways;
}

bool CPDF_IconFit::IsProportionalScale() const {
  return !m_pDict || m_pDict->GetStringFor("S", "P") != "A";
}

CFX_PointF CPDF_IconFit::GetIconBottomLeftPosition() const {
  float fLeft = kDefaultPosition;
  float fBottom = kDefaultPosition;
  if (!m_pDict)
    return {fLeft, fBottom};

  const CPDF_Array* pA = m_pDict->GetArrayFor("A");
  if (!pA)
    return {fLeft, fBottom};

  size_t dwCount = pA->size();
  if (dwCount > 0)
    fLeft = pA->GetNumberAt(0);
  if (dwCount > 1)
    fBottom = pA->GetNumberAt(1);
  return {fLeft, fBottom};
}

bool CPDF_IconFit::GetFittingBounds() const {
  return m_pDict && m_pDict->GetBooleanFor("FB", false);
}

CFX_PointF CPDF_IconFit::GetIconPosition() const {
  if (!m_pDict)
    return CFX_PointF();

  const CPDF_Array* pA = m_pDict->GetArrayFor("A");
  if (!pA)
    return CFX_PointF();

  size_t dwCount = pA->size();
  return {dwCount > 0 ? pA->GetNumberAt(0) : 0.0f,
          dwCount > 1 ? pA->GetNumberAt(1) : 0.0f};
}

std::pair<float, float> CPDF_IconFit::GetScale(
    const CFX_SizeF& image_size,
    const CFX_FloatRect& rcPlate) const {
  float fHScale = 1.0f;
  float fVScale = 1.0f;
  float fPlateWidth = rcPlate.Width();
  float fPlateHeight = rcPlate.Height();
  float fImageWidth = image_size.width;
  float fImageHeight = image_size.height;
  ScaleMethod scale_method = GetScaleMethod();
  switch (scale_method) {
    case CPDF_IconFit::ScaleMethod::kAlways:
      fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case CPDF_IconFit::ScaleMethod::kBigger:
      if (fPlateWidth < fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight < fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case CPDF_IconFit::ScaleMethod::kSmaller:
      if (fPlateWidth > fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight > fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case CPDF_IconFit::ScaleMethod::kNever:
      break;
  }

  if (IsProportionalScale()) {
    float min_scale = std::min(fHScale, fVScale);
    fHScale = min_scale;
    fVScale = min_scale;
  }
  return {fHScale, fVScale};
}

std::pair<float, float> CPDF_IconFit::GetImageOffset(
    const CFX_SizeF& image_size,
    const CFX_FloatRect& rcPlate) const {
  CFX_PointF icon_position = GetIconPosition();
  float fLeft = icon_position.x;
  float fBottom = icon_position.y;
  float fImageWidth = image_size.width;
  float fImageHeight = image_size.height;

  float fHScale, fVScale;
  std::tie(fHScale, fVScale) = GetScale(image_size, rcPlate);

  float fImageFactWidth = fImageWidth * fHScale;
  float fImageFactHeight = fImageHeight * fVScale;
  float fPlateWidth = rcPlate.Width();
  float fPlateHeight = rcPlate.Height();

  return {(fPlateWidth - fImageFactWidth) * fLeft,
          (fPlateHeight - fImageFactHeight) * fBottom};
}
