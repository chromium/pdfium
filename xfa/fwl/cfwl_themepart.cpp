// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_themepart.h"

namespace pdfium {

CFWL_ThemePart::CFWL_ThemePart(Part iPart, CFWL_Widget* pWidget)
    : part_(iPart), widget_(pWidget) {}

CFWL_ThemePart::~CFWL_ThemePart() = default;

FWLTHEME_STATE CFWL_ThemePart::GetThemeState() const {
  if (states_ & CFWL_PartState::kDisabled) {
    return FWLTHEME_STATE::kDisable;
  }
  if (states_ & CFWL_PartState::kPressed) {
    return FWLTHEME_STATE::kPressed;
  }
  if (states_ & CFWL_PartState::kHovered) {
    return FWLTHEME_STATE::kHover;
  }
  return FWLTHEME_STATE::kNormal;
}

}  // namespace pdfium
