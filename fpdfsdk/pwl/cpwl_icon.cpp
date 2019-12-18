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

CPWL_Icon::CPWL_Icon(const CreateParams& cp,
                     std::unique_ptr<CPDF_Icon> pIcon,
                     CPDF_IconFit* pFit)
    : CPWL_Wnd(cp, nullptr), m_pIcon(std::move(pIcon)), m_pIconFit(pFit) {
  ASSERT(m_pIcon);
}

CPWL_Icon::~CPWL_Icon() = default;

CFX_SizeF CPWL_Icon::GetImageSize() {
  return m_pIcon->GetImageSize();
}

CFX_Matrix CPWL_Icon::GetImageMatrix() {
  return m_pIcon->GetImageMatrix();
}

ByteString CPWL_Icon::GetImageAlias() {
  return m_pIcon->GetImageAlias();
}

CFX_PointF CPWL_Icon::GetIconPosition() {
  if (!m_pIconFit)
    return CFX_PointF();

  return m_pIconFit->GetIconPosition();
}

std::pair<float, float> CPWL_Icon::GetScale() {
  float fHScale = 1.0f;
  float fVScale = 1.0f;

  CFX_FloatRect rcPlate = GetClientRect();
  float fPlateWidth = rcPlate.Width();
  float fPlateHeight = rcPlate.Height();

  CFX_SizeF image_size = GetImageSize();
  float fImageWidth = image_size.width;
  float fImageHeight = image_size.height;
  int32_t nScaleMethod = m_pIconFit ? m_pIconFit->GetScaleMethod() : 0;

  switch (nScaleMethod) {
    default:
    case 0:
      fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 1:
      if (fPlateWidth < fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight < fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 2:
      if (fPlateWidth > fImageWidth)
        fHScale = fPlateWidth / std::max(fImageWidth, 1.0f);
      if (fPlateHeight > fImageHeight)
        fVScale = fPlateHeight / std::max(fImageHeight, 1.0f);
      break;
    case 3:
      break;
  }

  float fMinScale;
  if (m_pIconFit && m_pIconFit->IsProportionalScale()) {
    fMinScale = std::min(fHScale, fVScale);
    fHScale = fMinScale;
    fVScale = fMinScale;
  }
  return {fHScale, fVScale};
}

std::pair<float, float> CPWL_Icon::GetImageOffset() {
  CFX_PointF icon_position = GetIconPosition();
  float fLeft = icon_position.x;
  float fBottom = icon_position.y;

  CFX_SizeF image_size = GetImageSize();
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
