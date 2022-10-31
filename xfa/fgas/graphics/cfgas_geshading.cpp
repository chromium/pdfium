// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/graphics/cfgas_geshading.h"

CFGAS_GEShading::CFGAS_GEShading(const CFX_PointF& beginPoint,
                                 const CFX_PointF& endPoint,
                                 bool isExtendedBegin,
                                 bool isExtendedEnd,
                                 FX_ARGB beginArgb,
                                 FX_ARGB endArgb)
    : m_type(Type::kAxial),
      m_beginPoint(beginPoint),
      m_endPoint(endPoint),
      m_beginRadius(0),
      m_endRadius(0),
      m_isExtendedBegin(isExtendedBegin),
      m_isExtendedEnd(isExtendedEnd) {
  InitArgbArray(beginArgb, endArgb);
}

CFGAS_GEShading::CFGAS_GEShading(const CFX_PointF& beginPoint,
                                 const CFX_PointF& endPoint,
                                 float beginRadius,
                                 float endRadius,
                                 bool isExtendedBegin,
                                 bool isExtendedEnd,
                                 FX_ARGB beginArgb,
                                 FX_ARGB endArgb)
    : m_type(Type::kRadial),
      m_beginPoint(beginPoint),
      m_endPoint(endPoint),
      m_beginRadius(beginRadius),
      m_endRadius(endRadius),
      m_isExtendedBegin(isExtendedBegin),
      m_isExtendedEnd(isExtendedEnd) {
  InitArgbArray(beginArgb, endArgb);
}

CFGAS_GEShading::~CFGAS_GEShading() = default;

void CFGAS_GEShading::InitArgbArray(FX_ARGB beginArgb, FX_ARGB endArgb) {
  int32_t a1;
  int32_t r1;
  int32_t g1;
  int32_t b1;
  std::tie(a1, r1, g1, b1) = ArgbDecode(beginArgb);

  int32_t a2;
  int32_t r2;
  int32_t g2;
  int32_t b2;
  std::tie(a2, r2, g2, b2) = ArgbDecode(endArgb);

  float f = static_cast<float>(kSteps - 1);
  float aScale = 1.0 * (a2 - a1) / f;
  float rScale = 1.0 * (r2 - r1) / f;
  float gScale = 1.0 * (g2 - g1) / f;
  float bScale = 1.0 * (b2 - b1) / f;

  for (size_t i = 0; i < kSteps; i++) {
    int32_t a3 = static_cast<int32_t>(i * aScale);
    int32_t r3 = static_cast<int32_t>(i * rScale);
    int32_t g3 = static_cast<int32_t>(i * gScale);
    int32_t b3 = static_cast<int32_t>(i * bScale);

    // TODO(dsinclair): Add overloads for FX_ARGB. pdfium:437
    m_argbArray[i] = ArgbEncode(a1 + a3, r1 + r3, g1 + g3, b1 + b3);
  }
}
