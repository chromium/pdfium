// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_themepart.h"

CFWL_ThemePart::CFWL_ThemePart(CFWL_Widget* pWidget) : m_pWidget(pWidget) {}

CFWL_ThemePart::~CFWL_ThemePart() = default;

FWLTHEME_STATE CFWL_ThemePart::GetThemeState() const {
  if (m_dwStates & CFWL_PartState_Disabled)
    return FWLTHEME_STATE::kDisable;
  if (m_dwStates & CFWL_PartState_Pressed)
    return FWLTHEME_STATE::kPressed;
  if (m_dwStates & CFWL_PartState_Hovered)
    return FWLTHEME_STATE::kHover;
  return FWLTHEME_STATE::kNormal;
}
