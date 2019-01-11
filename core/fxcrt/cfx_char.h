// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_CHAR_H_
#define CORE_FXCRT_CFX_CHAR_H_

#include <stdint.h>

#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/retain_ptr.h"

enum class CFX_BreakType : uint8_t { None = 0, Piece, Line, Paragraph, Page };

class CFX_Char {
 public:
  explicit CFX_Char(uint16_t wCharCode);
  CFX_Char(uint16_t wCharCode,
           int32_t iHorizontalScale,
           int32_t iVerticalScale);
  CFX_Char(const CFX_Char& other);
  ~CFX_Char();

  FX_CHARTYPE GetCharType() const;

  uint16_t char_code() const { return m_wCharCode; }
  int16_t horizonal_scale() const { return m_iHorizontalScale; }
  int16_t vertical_scale() const { return m_iVerticalScale; }

  CFX_BreakType m_dwStatus = CFX_BreakType::None;
  FX_BIDICLASS m_iBidiClass = FX_BIDICLASS::kON;
  uint8_t m_nBreakType = 0;
  uint32_t m_dwCharStyles = 0;
  int32_t m_iCharWidth = 0;
  uint16_t m_iBidiLevel = 0;
  uint16_t m_iBidiPos = 0;
  uint16_t m_iBidiOrder = 0;
  int32_t m_iFontSize = 0;
  uint32_t m_dwIdentity = 0;
  RetainPtr<Retainable> m_pUserData;

 private:
  uint16_t m_wCharCode;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
};

#endif  // CORE_FXCRT_CFX_CHAR_H_
