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

void CFGAS_GEShading::InitArgbArray(FX_ARGB begin_argb, FX_ARGB end_argb) {
  FX_BGRA_STRUCT<uint8_t> bgra0 = ArgbToBGRAStruct(begin_argb);
  FX_BGRA_STRUCT<uint8_t> bgra1 = ArgbToBGRAStruct(end_argb);

  constexpr float f = static_cast<float>(kSteps - 1);
  const float a_scale = 1.0 * (bgra1.alpha - bgra0.alpha) / f;
  const float r_scale = 1.0 * (bgra1.red - bgra0.red) / f;
  const float g_scale = 1.0 * (bgra1.green - bgra0.green) / f;
  const float b_scale = 1.0 * (bgra1.blue - bgra0.blue) / f;

  for (size_t i = 0; i < kSteps; i++) {
    m_argbArray[i] = ArgbEncode(static_cast<int32_t>(i * a_scale) + bgra0.alpha,
                                static_cast<int32_t>(i * r_scale) + bgra0.red,
                                static_cast<int32_t>(i * g_scale) + bgra0.green,
                                static_cast<int32_t>(i * b_scale) + bgra0.blue);
  }
}
