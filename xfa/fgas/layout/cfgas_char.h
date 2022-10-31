// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_CHAR_H_
#define XFA_FGAS_LAYOUT_CFGAS_CHAR_H_

#include <stdint.h>

#include <vector>

#include "core/fxcrt/fx_unicode.h"
#include "core/fxcrt/retain_ptr.h"
#include "xfa/fgas/layout/cfgas_textuserdata.h"
#include "xfa/fgas/layout/fgas_linebreak.h"

class CFGAS_Char {
 public:
  enum class BreakType : uint8_t {
    kNone = 0,
    kPiece,
    kLine,
    kParagraph,
    kPage
  };

  static void BidiLine(std::vector<CFGAS_Char>* chars, size_t iCount);

  explicit CFGAS_Char(uint16_t wCharCode);
  CFGAS_Char(uint16_t wCharCode,
             int32_t iHorizontalScale,
             int32_t iVerticalScale);
  CFGAS_Char(const CFGAS_Char& other);
  ~CFGAS_Char();

  FX_CHARTYPE GetCharType() const;
  uint16_t char_code() const { return m_wCharCode; }
  int16_t horizonal_scale() const { return m_iHorizontalScale; }
  int16_t vertical_scale() const { return m_iVerticalScale; }

  BreakType m_dwStatus = BreakType::kNone;
  FX_BIDICLASS m_iBidiClass = FX_BIDICLASS::kON;
  FX_LINEBREAKTYPE m_eLineBreakType = FX_LINEBREAKTYPE::kUNKNOWN;
  uint32_t m_dwCharStyles = 0;
  int32_t m_iCharWidth = 0;
  uint16_t m_iBidiLevel = 0;
  uint16_t m_iBidiPos = 0;
  uint16_t m_iBidiOrder = 0;
  int32_t m_iFontSize = 0;
  uint32_t m_dwIdentity = 0;
  RetainPtr<CFGAS_TextUserData> m_pUserData;

 private:
  uint16_t m_wCharCode;
  int32_t m_iHorizontalScale;
  int32_t m_iVerticalScale;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_CHAR_H_
