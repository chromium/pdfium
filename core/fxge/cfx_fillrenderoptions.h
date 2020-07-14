// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXGE_CFX_FILLRENDEROPTIONS_H_
#define CORE_FXGE_CFX_FILLRENDEROPTIONS_H_

// Represents the options for filling paths.
struct CFX_FillRenderOptions {
  // FillType defines how path is filled.
  enum class FillType {
    // No filling needed.
    kNoFill = 0,

    // Use even-odd or inverse even-odd algorithms to decide if the area needs
    // to be filled.
    kEvenOdd = 1,

    // Use winding or inverse winding algorithms to decide whether the area
    // needs to be filled.
    kWinding = 2,
  };

  static const CFX_FillRenderOptions& EvenOddOptions();
  static const CFX_FillRenderOptions& WindingOptions();

  CFX_FillRenderOptions();
  explicit CFX_FillRenderOptions(FillType fill_type);

  // Fill type.
  FillType fill_type = FillType::kNoFill;

  // Adjusted stroke rendering is enabled.
  bool adjust_stroke = false;

  // Whether anti aliasing is enabled for path rendering.
  bool aliased_path = false;

  // Fills with the sum of colors from both cover and source.
  bool full_cover = false;

  // Rect paths use anti-aliasing.
  bool rect_aa = false;

  // Path is stroke.
  bool stroke = false;

  // Renders text by filling strokes.
  bool stroke_text_mode = false;

  // Path is text.
  bool text_mode = false;

  // Path encloses zero area.
  bool zero_area = false;
};

#endif  // CORE_FXGE_CFX_FILLRENDEROPTIONS_H_
