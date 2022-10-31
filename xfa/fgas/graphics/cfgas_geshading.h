// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_

#include <stddef.h>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/fx_dib.h"

class CFGAS_GEShading final {
 public:
  enum class Type { kAxial = 1, kRadial };

  static constexpr size_t kSteps = 256;

  // Axial shading.
  CFGAS_GEShading(const CFX_PointF& beginPoint,
                  const CFX_PointF& endPoint,
                  bool isExtendedBegin,
                  bool isExtendedEnd,
                  FX_ARGB beginArgb,
                  FX_ARGB endArgb);

  // Radial shading.
  CFGAS_GEShading(const CFX_PointF& beginPoint,
                  const CFX_PointF& endPoint,
                  float beginRadius,
                  float endRadius,
                  bool isExtendedBegin,
                  bool isExtendedEnd,
                  FX_ARGB beginArgb,
                  FX_ARGB endArgb);

  ~CFGAS_GEShading();

  Type GetType() const { return m_type; }
  CFX_PointF GetBeginPoint() const { return m_beginPoint; }
  CFX_PointF GetEndPoint() const { return m_endPoint; }
  float GetBeginRadius() const { return m_beginRadius; }
  float GetEndRadius() const { return m_endRadius; }
  bool IsExtendedBegin() const { return m_isExtendedBegin; }
  bool IsExtendedEnd() const { return m_isExtendedEnd; }
  FX_ARGB GetArgb(size_t index) const { return m_argbArray[index]; }

 private:
  void InitArgbArray(FX_ARGB beginArgb, FX_ARGB endArgb);

  const Type m_type;
  const CFX_PointF m_beginPoint;
  const CFX_PointF m_endPoint;
  const float m_beginRadius;
  const float m_endRadius;
  const bool m_isExtendedBegin;
  const bool m_isExtendedEnd;
  FX_ARGB m_argbArray[kSteps];
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
