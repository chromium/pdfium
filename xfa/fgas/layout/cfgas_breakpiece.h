// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_
#define XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_

#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_TextUserData;

class CFGAS_BreakPiece {
 public:
  CFGAS_BreakPiece();
  CFGAS_BreakPiece(const CFGAS_BreakPiece& other);
  ~CFGAS_BreakPiece();

  int32_t GetEndPos() const;

  // TODO(thestig): When GetCharCount() returns size_t, remove this.
  size_t GetLength() const;

  CFGAS_Char* GetChar(int32_t index) const;
  CFGAS_Char* GetChar(size_t index) const;
  WideString GetString() const;
  std::vector<int32_t> GetWidths() const;

  CFGAS_Char::BreakType GetStatus() const { return status_; }
  void SetStatus(CFGAS_Char::BreakType status) { status_ = status; }

  int32_t GetStartPos() const { return start_pos_; }
  void SetStartPos(int32_t pos) { start_pos_ = pos; }
  void IncrementStartPos(int32_t count) { start_pos_ += count; }

  int32_t GetWidth() const { return width_; }
  void SetWidth(int32_t width) { width_ = width; }
  void IncrementWidth(int32_t width) { width_ += width; }

  int32_t GetStartChar() const { return start_char_; }
  void SetStartChar(int32_t pos) { start_char_ = pos; }

  int32_t GetCharCount() const { return char_count_; }
  void SetCharCount(int32_t count) { char_count_ = count; }

  int32_t GetBidiLevel() const { return bidi_level_; }
  void SetBidiLevel(int32_t level) { bidi_level_ = level; }

  int32_t GetBidiPos() const { return bidi_pos_; }
  void SetBidiPos(int32_t pos) { bidi_pos_ = pos; }

  int32_t GetFontSize() const { return font_size_; }
  void SetFontSize(int32_t font_size) { font_size_ = font_size; }

  int32_t GetHorizontalScale() const { return horizontal_scale_; }
  void SetHorizontalScale(int32_t scale) { horizontal_scale_ = scale; }

  int32_t GetVerticalScale() const { return vertical_scale_; }
  void SetVerticalScale(int32_t scale) { vertical_scale_ = scale; }

  uint32_t GetCharStyles() const { return char_styles_; }
  void SetCharStyles(uint32_t styles) { char_styles_ = styles; }

  void SetChars(std::vector<CFGAS_Char>* chars) { chars_ = chars; }

  const CFGAS_TextUserData* GetUserData() const { return user_data_.Get(); }
  void SetUserData(const RetainPtr<CFGAS_TextUserData>& user_data) {
    user_data_ = user_data;
  }

 private:
  CFGAS_Char::BreakType status_ = CFGAS_Char::BreakType::kPiece;
  int32_t start_pos_ = 0;
  int32_t width_ = -1;
  int32_t start_char_ = 0;
  int32_t char_count_ = 0;
  int32_t bidi_level_ = 0;
  int32_t bidi_pos_ = 0;
  int32_t font_size_ = 0;
  int32_t horizontal_scale_ = 100;
  int32_t vertical_scale_ = 100;
  uint32_t char_styles_ = 0;
  UnownedPtr<std::vector<CFGAS_Char>> chars_;
  RetainPtr<CFGAS_TextUserData> user_data_;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAKPIECE_H_
