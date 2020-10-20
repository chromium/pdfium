// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/dib/fx_dib.h"

#define FX_SHADING_Steps 256

enum CFGAS_GEShading_Type { FX_SHADING_Axial = 1, FX_SHADING_Radial };

class CFGAS_GEShading final {
 public:
  // Axial shading.
  CFGAS_GEShading(const CFX_PointF& beginPoint,
                  const CFX_PointF& endPoint,
                  bool isExtendedBegin,
                  bool isExtendedEnd,
                  const FX_ARGB beginArgb,
                  const FX_ARGB endArgb);

  // Radial shading.
  CFGAS_GEShading(const CFX_PointF& beginPoint,
                  const CFX_PointF& endPoint,
                  const float beginRadius,
                  const float endRadius,
                  bool isExtendedBegin,
                  bool isExtendedEnd,
                  const FX_ARGB beginArgb,
                  const FX_ARGB endArgb);

  ~CFGAS_GEShading();

 private:
  friend class CFGAS_GEGraphics;

  void InitArgbArray();

  const CFGAS_GEShading_Type m_type;
  const CFX_PointF m_beginPoint;
  const CFX_PointF m_endPoint;
  const float m_beginRadius;
  const float m_endRadius;
  const bool m_isExtendedBegin;
  const bool m_isExtendedEnd;
  const FX_ARGB m_beginArgb;
  const FX_ARGB m_endArgb;
  FX_ARGB m_argbArray[FX_SHADING_Steps];
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GESHADING_H_
