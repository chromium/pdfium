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

  HatchStyle GetHatchStyle() const { return hatch_style_; }
  FX_ARGB GetForeArgb() const { return fore_argb_; }
  FX_ARGB GetBackArgb() const { return back_argb_; }

 private:
  const HatchStyle hatch_style_;
  const FX_ARGB fore_argb_;
  const FX_ARGB back_argb_;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_
