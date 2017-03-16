// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_CHAR_H_
#define CORE_FXCRT_CFX_CHAR_H_

#include <stdint.h>

#include "core/fxcrt/fx_ucd.h"

enum class CFX_BreakType { None = 0, Piece, Line, Paragraph, Page };

class CFX_Char {
 public:
  CFX_Char();
  CFX_Char(uint16_t wCharCode, uint32_t dwCharProps);
  CFX_Char(const CFX_Char& other);
  ~CFX_Char();

  FX_CHARTYPE GetCharType() const;

  CFX_BreakType m_dwStatus;
  uint8_t m_nBreakType;
  uint32_t m_dwCharStyles;
  uint32_t m_dwCharProps;
  int32_t m_iCharWidth;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
  int16_t m_iBidiClass;
  int16_t m_iBidiLevel;
  int16_t m_iBidiPos;
  int16_t m_iBidiOrder;
  uint16_t m_wCharCode;
  int32_t m_iFontSize;
  uint32_t m_dwIdentity;
  CFX_RetainPtr<CFX_Retainable> m_pUserData;
};

#endif  // CORE_FXCRT_CFX_CHAR_H_
