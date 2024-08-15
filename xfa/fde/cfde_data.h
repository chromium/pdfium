// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_DATA_H_
#define XFA_FDE_CFDE_DATA_H_

#include "core/fxcrt/fx_coordinates.h"

namespace pdfium {

enum class FDE_TextAlignment : uint8_t {
  kTopLeft = 0,
  kCenterLeft,
  kCenter,
  kCenterRight
};

struct FDE_TextStyle {
  FDE_TextStyle() = default;
  ~FDE_TextStyle() = default;

  void Reset() {
    single_line_ = false;
    line_wrap_ = false;
    last_line_height_ = false;
  }

  bool single_line_ = false;
  bool line_wrap_ = false;
  bool last_line_height_ = false;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::FDE_TextAlignment;
using pdfium::FDE_TextStyle;

#endif  // XFA_FDE_CFDE_DATA_H_
