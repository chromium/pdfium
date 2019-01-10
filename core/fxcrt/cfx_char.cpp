// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_char.h"

CFX_Char::CFX_Char(uint16_t wCharCode) : CFX_Char(wCharCode, 100, 100) {}

CFX_Char::CFX_Char(uint16_t wCharCode,
                   int32_t iHorizontalScale,
                   int32_t iVerticalScale)
    : m_wCharCode(wCharCode),
      m_iHorizontalScale(iHorizontalScale),
      m_iVerticalScale(iVerticalScale) {}

CFX_Char::CFX_Char(const CFX_Char& other) = default;

CFX_Char::~CFX_Char() = default;

FX_CHARTYPE CFX_Char::GetCharType() const {
  return FX_GetCharType(m_wCharCode);
}
