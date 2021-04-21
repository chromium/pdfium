// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_icon.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "core/fpdfdoc/cpdf_icon.h"
#include "core/fpdfdoc/cpdf_iconfit.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "third_party/base/check.h"

CPWL_Icon::CPWL_Icon(const CreateParams& cp,
                     CPDF_Icon* pIcon,
                     CPDF_IconFit* pFit)
    : CPWL_Wnd(cp, nullptr), m_pIcon(pIcon), m_pIconFit(pFit) {
  DCHECK(m_pIcon);
  DCHECK(m_pIconFit);
}

CPWL_Icon::~CPWL_Icon() = default;

std::pair<float, float> CPWL_Icon::GetScale() {
  float fHScale = 1.0f;
  float fVScale = 1.0f;

  CFX_FloatRect rcPlate = GetClientRect();
  float fPlateWidth = rcPlate.Width();
  float fPlateHeight = rcPlate.Height();

  CFX_SizeF image_size = m_pIcon->GetImageSize();
  float fImageWidth = image_size.width;
  float fImageHeight = image_size.height;

  CPDF_IconFit::ScaleMethod scale_method = m_pIconFit->GetScaleMethod();
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

  if (m_pIconFit->IsProportionalScale()) {
    float min_scale = std::min(fHScale, fVScale);
    fHScale = min_scale;
    fVScale = min_scale;
  }
  return {fHScale, fVScale};
}

std::pair<float, float> CPWL_Icon::GetImageOffset() {
  CFX_PointF icon_position = m_pIconFit->GetIconPosition();
  float fLeft = icon_position.x;
  float fBottom = icon_position.y;

  CFX_SizeF image_size = m_pIcon->GetImageSize();
  float fImageWidth = image_size.width;
  float fImageHeight = image_size.height;

  float fHScale, fVScale;
  std::tie(fHScale, fVScale) = GetScale();

  float fImageFactWidth = fImageWidth * fHScale;
  float fImageFactHeight = fImageHeight * fVScale;

  CFX_FloatRect rcPlate = GetClientRect();
  float fPlateWidth = rcPlate.Width();
  float fPlateHeight = rcPlate.Height();

  return {(fPlateWidth - fImageFactWidth) * fLeft,
          (fPlateHeight - fImageFactHeight) * fBottom};
}
