// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_

#include "core/fxge/dib/fx_dib.h"

class CFGAS_GEPattern final {
 public:
  enum class HatchStyle {
    Horizontal = 0,
    Vertical = 1,
    ForwardDiagonal = 2,
    BackwardDiagonal = 3,
    Cross = 4,
    DiagonalCross = 5
  };

  CFGAS_GEPattern(HatchStyle hatchStyle, FX_ARGB foreArgb, FX_ARGB backArgb);
  ~CFGAS_GEPattern();

  HatchStyle GetHatchStyle() const { return m_hatchStyle; }
  FX_ARGB GetForeArgb() const { return m_foreArgb; }
  FX_ARGB GetBackArgb() const { return m_backArgb; }

 private:
  const HatchStyle m_hatchStyle;
  const FX_ARGB m_foreArgb;
  const FX_ARGB m_backArgb;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_
