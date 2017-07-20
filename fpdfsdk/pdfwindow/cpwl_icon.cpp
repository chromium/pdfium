// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_icon.h"

#include <algorithm>
#include <sstream>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

CPWL_Image::CPWL_Image() : m_pPDFStream(nullptr) {}

CPWL_Image::~CPWL_Image() {}

void CPWL_Image::SetPDFStream(CPDF_Stream* pStream) {
  m_pPDFStream = pStream;
}

CPDF_Stream* CPWL_Image::GetPDFStream() const {
  return m_pPDFStream.Get();
}

void CPWL_Image::GetImageSize(float& fWidth, float& fHeight) {
  fWidth = 0.0f;
  fHeight = 0.0f;

  if (m_pPDFStream) {
    if (CPDF_Dictionary* pDict = m_pPDFStream->GetDict()) {
      CFX_FloatRect rect = pDict->GetRectFor("BBox");

      fWidth = rect.right - rect.left;
      fHeight = rect.top - rect.bottom;
    }
  }
}

CFX_Matrix CPWL_Image::GetImageMatrix() {
  if (m_pPDFStream) {
    if (CPDF_Dictionary* pDict = m_pPDFStream->GetDict()) {
      return pDict->GetMatrixFor("Matrix");
    }
  }

  return CFX_Matrix();
}

CFX_ByteString CPWL_Image::GetImageAlias() {
  if (!m_sImageAlias.IsEmpty())
    return m_sImageAlias;

  if (m_pPDFStream) {
    if (CPDF_Dictionary* pDict = m_pPDFStream->GetDict()) {
      return pDict->GetStringFor("Name");
    }
  }

  return CFX_ByteString();
}

void CPWL_Image::SetImageAlias(const char* sImageAlias) {
  m_sImageAlias = sImageAlias;
}

void CPWL_Image::GetScale(float& fHScale, float& fVScale) {
  fHScale = 1.0f;
  fVScale = 1.0f;
}

void CPWL_Image::GetImageOffset(float& x, float& y) {
  x = 0.0f;
  y = 0.0f;
}

CPWL_Icon::CPWL_Icon() : m_pIconFit(nullptr) {}

CPWL_Icon::~CPWL_Icon() {}

CPDF_IconFit* CPWL_Icon::GetIconFit() const {
  return m_pIconFit.Get();
}

int32_t CPWL_Icon::GetScaleMethod() {
  if (m_pIconFit)
    return m_pIconFit->GetScaleMethod();

  return 0;
}

bool CPWL_Icon::IsProportionalScale() {
  if (m_pIconFit)
    return m_pIconFit->IsProportionalScale();

  return false;
}

void CPWL_Icon::GetIconPosition(float& fLeft, float& fBottom) {
  if (m_pIconFit) {
    fLeft = 0.0f;
    fBottom = 0.0f;
    CPDF_Array* pA = m_pIconFit->GetDict()
                         ? m_pIconFit->GetDict()->GetArrayFor("A")
                         : nullptr;
    if (pA) {
      size_t dwCount = pA->GetCount();
      if (dwCount > 0)
        fLeft = pA->GetNumberAt(0);
      if (dwCount > 1)
        fBottom = pA->GetNumberAt(1);
    }
  } else {
    fLeft = 0.0f;
    fBottom = 0.0f;
  }
}

void CPWL_Icon::GetScale(float& fHScale, float& fVScale) {
  fHScale = 1.0f;
  fVScale = 1.0f;

  if (m_pPDFStream) {
    float fImageWidth, fImageHeight;
    float fPlateWidth, fPlateHeight;

    CFX_FloatRect rcPlate = GetClientRect();
    fPlateWidth = rcPlate.right - rcPlate.left;
    fPlateHeight = rcPlate.top - rcPlate.bottom;

    GetImageSize(fImageWidth, fImageHeight);

    int32_t nScaleMethod = GetScaleMethod();

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
    if (IsProportionalScale()) {
      fMinScale = std::min(fHScale, fVScale);
      fHScale = fMinScale;
      fVScale = fMinScale;
    }
  }
}

void CPWL_Icon::GetImageOffset(float& x, float& y) {
  float fLeft, fBottom;

  GetIconPosition(fLeft, fBottom);
  x = 0.0f;
  y = 0.0f;

  float fImageWidth, fImageHeight;
  GetImageSize(fImageWidth, fImageHeight);

  float fHScale, fVScale;
  GetScale(fHScale, fVScale);

  float fImageFactWidth = fImageWidth * fHScale;
  float fImageFactHeight = fImageHeight * fVScale;

  float fPlateWidth, fPlateHeight;
  CFX_FloatRect rcPlate = GetClientRect();
  fPlateWidth = rcPlate.right - rcPlate.left;
  fPlateHeight = rcPlate.top - rcPlate.bottom;

  x = (fPlateWidth - fImageFactWidth) * fLeft;
  y = (fPlateHeight - fImageFactHeight) * fBottom;
}
