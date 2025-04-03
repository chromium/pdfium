// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_breakpiece.h"

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "xfa/fgas/layout/cfgas_textuserdata.h"

CFGAS_BreakPiece::CFGAS_BreakPiece() = default;

CFGAS_BreakPiece::CFGAS_BreakPiece(const CFGAS_BreakPiece& other) = default;

CFGAS_BreakPiece::~CFGAS_BreakPiece() = default;

int32_t CFGAS_BreakPiece::GetEndPos() const {
  return width_ < 0 ? start_pos_ : start_pos_ + width_;
}

size_t CFGAS_BreakPiece::GetLength() const {
  return pdfium::checked_cast<size_t>(char_count_);
}

CFGAS_Char* CFGAS_BreakPiece::GetChar(int32_t index) const {
  return GetChar(pdfium::checked_cast<size_t>(index));
}

CFGAS_Char* CFGAS_BreakPiece::GetChar(size_t index) const {
  DCHECK_LT(index, GetLength());
  DCHECK(chars_);
  return &(*chars_)[start_char_ + index];
}

WideString CFGAS_BreakPiece::GetString() const {
  WideString ret;
  ret.Reserve(char_count_);
  for (int32_t i = start_char_; i < start_char_ + char_count_; i++) {
    ret += static_cast<wchar_t>((*chars_)[i].char_code());
  }
  return ret;
}

std::vector<int32_t> CFGAS_BreakPiece::GetWidths() const {
  std::vector<int32_t> ret;
  ret.reserve(char_count_);
  for (int32_t i = start_char_; i < start_char_ + char_count_; i++) {
    ret.push_back((*chars_)[i].char_width_);
  }
  return ret;
}
