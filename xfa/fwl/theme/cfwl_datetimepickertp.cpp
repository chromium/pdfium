// Copyright 2014 PDFium Authors. All rights reserved.
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
  switch (pParams.m_iPart) {
    case CFWL_ThemePart::Part::kBorder:
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    case CFWL_ThemePart::Part::kDropDownButton:
      DrawDropDownButton(pParams, pParams.m_matrix);
      break;
    default:
      break;
  }
}

void CFWL_DateTimePickerTP::DrawDropDownButton(
    const CFWL_ThemeBackground& pParams,
    const CFX_Matrix& matrix) {
  uint32_t dwStates = pParams.m_dwStates;
  dwStates &= 0x03;
  FWLTHEME_STATE eState = FWLTHEME_STATE::kNormal;

  // TODO(tsepez): enum mismatch, &ing with 1 makes no sense here.
  switch (static_cast<uint32_t>(eState) & dwStates) {
    case CFWL_PartState_Normal:
      eState = FWLTHEME_STATE::kNormal;
      break;
    case CFWL_PartState_Hovered:
      eState = FWLTHEME_STATE::kHover;
      break;
    case CFWL_PartState_Pressed:
      eState = FWLTHEME_STATE::kPressed;
      break;
    case CFWL_PartState_Disabled:
      eState = FWLTHEME_STATE::kDisable;
      break;
    default:
      break;
  }
  DrawArrowBtn(pParams.GetGraphics(), pParams.m_PartRect,
               FWLTHEME_DIRECTION::kDown, eState, matrix);
}
