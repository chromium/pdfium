// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_datetimepickertp.h"

#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_themebackground.h"

CFWL_DateTimePickerTP::CFWL_DateTimePickerTP() = default;

CFWL_DateTimePickerTP::~CFWL_DateTimePickerTP() = default;

void CFWL_DateTimePickerTP::DrawBackground(
    const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder:
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    case CFWL_ThemePart::Part::kDropDownButton:
      DrawArrowBtn(pParams.GetGraphics(), pParams.m_PartRect,
                   FWLTHEME_DIRECTION::kDown, pParams.GetThemeState(),
                   pParams.m_matrix);
      break;
    default:
      break;
  }
}
