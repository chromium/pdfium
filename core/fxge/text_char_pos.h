// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_TEXT_CHAR_POS_H_
#define CORE_FXGE_TEXT_CHAR_POS_H_

#include <array>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"

class TextCharPos {
 public:
  TextCharPos();
  TextCharPos(const TextCharPos&);
  ~TextCharPos();

  CFX_Matrix GetEffectiveMatrix(const CFX_Matrix& matrix) const;

  CFX_PointF origin_;
  uint32_t unicode_ = 0;
  uint32_t glyph_index_ = 0;
  int font_char_width_ = 0;
#if BUILDFLAG(IS_APPLE)
  uint32_t ext_gid_ = 0;
#endif
  int32_t fallback_font_position_ = 0;
  bool glyph_adjust_ = false;
  bool font_style_ = false;
  std::array<float, 4> adjust_matrix_ = {};
};

#endif  // CORE_FXGE_TEXT_CHAR_POS_H_
