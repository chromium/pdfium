// Copyright 2020 The PDFium Authors
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

  constexpr explicit CFX_FillRenderOptions(FillType fill_type)
      : fill_type(fill_type) {}

  friend constexpr bool operator==(const CFX_FillRenderOptions&,
                                   const CFX_FillRenderOptions&) = default;

  // Fill type.
  FillType fill_type;

  // Adjusted stroke rendering is enabled.
  bool adjust_stroke : 1 = false;

  // Whether anti aliasing is enabled for path rendering.
  bool aliased_path : 1 = false;

  // Fills with the sum of colors from both cover and source.
  bool full_cover : 1 = false;

  // Rect paths use anti-aliasing.
  bool rect_aa : 1 = false;

  // Path is stroke.
  bool stroke : 1 = false;

  // Renders text by filling strokes.
  bool stroke_text_mode : 1 = false;

  // Path is text.
  bool text_mode : 1 = false;

  // Path encloses zero area.
  bool zero_area : 1 = false;
};

#endif  // CORE_FXGE_CFX_FILLRENDEROPTIONS_H_
