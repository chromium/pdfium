// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_DATA_H_
#define XFA_FDE_CFDE_DATA_H_

#include "core/fxcrt/fx_coordinates.h"

enum class FDE_TextAlignment : uint8_t {
  kTopLeft = 0,
  kCenterLeft,
  kCenter,
  kCenterRight
};

struct FDE_TextStyle {
  FDE_TextStyle()
      : single_line_(false), line_wrap_(false), last_line_height_(false) {}
  ~FDE_TextStyle() {}

  void Reset() {
    single_line_ = false;
    line_wrap_ = false;
    last_line_height_ = false;
  }

  bool single_line_;
  bool line_wrap_;
  bool last_line_height_;
};

struct FDE_TTOPIECE {
  FDE_TTOPIECE();
  FDE_TTOPIECE(const FDE_TTOPIECE& that);
  ~FDE_TTOPIECE();

  int32_t iStartChar;
  int32_t iChars;
  uint32_t dwCharStyles;
  CFX_RectF rtPiece;
};

#endif  // XFA_FDE_CFDE_DATA_H_
