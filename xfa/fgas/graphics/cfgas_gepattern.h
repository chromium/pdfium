// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"

class CFX_DIBitmap;
class CFX_Matrix;

class CFGAS_GEPattern final {
 public:
  CFGAS_GEPattern(FX_HatchStyle hatchStyle,
                  const FX_ARGB foreArgb,
                  const FX_ARGB backArgb);

  ~CFGAS_GEPattern();

 private:
  friend class CFGAS_GEGraphics;

  const FX_HatchStyle m_hatchStyle;
  const FX_ARGB m_foreArgb;
  const FX_ARGB m_backArgb;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GEPATTERN_H_
