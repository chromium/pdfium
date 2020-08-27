// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_CFX_FILLRENDEROPTIONS_H_
#define CORE_FXGE_CFX_FILLRENDEROPTIONS_H_

#include <stdint.h>

// Represents the options for filling paths.
struct CFX_FillRenderOptions {
  // FillType defines how path is filled.
  enum class FillType : uint8_t {
    // No filling needed.
    kNoFill = 0,

    // Use even-odd or inverse even-odd algorithms to decide if the area needs
    // to be filled.
    kEvenOdd = 1,

    // Use winding or inverse winding algorithms to decide whether the area
    // needs to be filled.
    kWinding = 2,
  };

  static constexpr CFX_FillRenderOptions EvenOddOptions() {
    return CFX_FillRenderOptions(FillType::kEvenOdd);
  }
  static constexpr CFX_FillRenderOptions WindingOptions() {
    return CFX_FillRenderOptions(FillType::kWinding);
  }

  constexpr CFX_FillRenderOptions()
      : CFX_FillRenderOptions(FillType::kNoFill) {}

  // TODO(thestig): Switch to default member initializer for bit-fields when
  // C++20 is available.
  constexpr explicit CFX_FillRenderOptions(FillType fill_type)
      : fill_type(fill_type),
        adjust_stroke(false),
        aliased_path(false),
        full_cover(false),
        rect_aa(false),
        stroke(false),
        stroke_text_mode(false),
        text_mode(false),
        zero_area(false) {}

  // Fill type.
  FillType fill_type;

  // Adjusted stroke rendering is enabled.
  bool adjust_stroke : 1;

  // Whether anti aliasing is enabled for path rendering.
  bool aliased_path : 1;

  // Fills with the sum of colors from both cover and source.
  bool full_cover : 1;

  // Rect paths use anti-aliasing.
  bool rect_aa : 1;

  // Path is stroke.
  bool stroke : 1;

  // Renders text by filling strokes.
  bool stroke_text_mode : 1;

  // Path is text.
  bool text_mode : 1;

  // Path encloses zero area.
  bool zero_area : 1;
};

#endif  // CORE_FXGE_CFX_FILLRENDEROPTIONS_H_
