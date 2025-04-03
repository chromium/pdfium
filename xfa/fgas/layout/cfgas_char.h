// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_CHAR_H_
#define XFA_FGAS_LAYOUT_CFGAS_CHAR_H_

#include <stddef.h>
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
  uint16_t char_code() const { return char_code_; }
  int16_t horizonal_scale() const { return horizontal_scale_; }
  int16_t vertical_scale() const { return vertical_scale_; }

  BreakType status_ = BreakType::kNone;
  FX_BIDICLASS bidi_class_ = FX_BIDICLASS::kON;
  FX_LINEBREAKTYPE line_break_type_ = FX_LINEBREAKTYPE::kUNKNOWN;
  uint32_t char_styles_ = 0;
  int32_t char_width_ = 0;
  uint16_t bidi_level_ = 0;
  uint16_t bidi_pos_ = 0;
  uint16_t bidi_order_ = 0;
  int32_t font_size_ = 0;
  uint32_t identity_ = 0;
  RetainPtr<CFGAS_TextUserData> user_data_;

 private:
  uint16_t char_code_;
  int32_t horizontal_scale_;
  int32_t vertical_scale_;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_CHAR_H_
