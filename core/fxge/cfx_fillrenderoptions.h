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

  bool operator==(const CFX_FillRenderOptions& other) const {
    return fill_type == other.fill_type &&
           adjust_stroke == other.adjust_stroke &&
           aliased_path == other.aliased_path &&
           full_cover == other.full_cover && rect_aa == other.rect_aa &&
           stroke == other.stroke &&
           stroke_text_mode == other.stroke_text_mode &&
           text_mode == other.text_mode && zero_area == other.zero_area;
  }

  bool operator!=(const CFX_FillRenderOptions& other) const {
    return !(*this == other);
  }

  // Fill type.
  FillType fill_type = FillType::kNoFill;

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
