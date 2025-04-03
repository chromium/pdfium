// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_

#include <stddef.h>

#include <array>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/dib/fx_dib.h"

class CFGAS_GEShading final {
 public:
  enum class Type { kAxial = 1, kRadial };

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

  Type GetType() const { return type_; }
  CFX_PointF GetBeginPoint() const { return begin_point_; }
  CFX_PointF GetEndPoint() const { return end_point_; }
  float GetBeginRadius() const { return begin_radius_; }
  float GetEndRadius() const { return end_radius_; }
  bool IsExtendedBegin() const { return is_extended_begin_; }
  bool IsExtendedEnd() const { return is_extended_end_; }
  FX_ARGB GetArgb(float value) const {
    return argb_array_[static_cast<size_t>(value * (kSteps - 1))];
  }

 private:
  static constexpr size_t kSteps = 256;

  void InitArgbArray(FX_ARGB begin_argb, FX_ARGB end_argb);

  const Type type_;
  const CFX_PointF begin_point_;
  const CFX_PointF end_point_;
  const float begin_radius_;
  const float end_radius_;
  const bool is_extended_begin_;
  const bool is_extended_end_;
  std::array<FX_ARGB, kSteps> argb_array_;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
