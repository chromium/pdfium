// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_CFGAS_BREAKLINE_H_
#define XFA_FGAS_LAYOUT_CFGAS_BREAKLINE_H_

#include <vector>

#include "xfa/fgas/layout/cfgas_breakpiece.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_BreakLine {
 public:
  CFGAS_BreakLine();
  ~CFGAS_BreakLine();

  CFGAS_Char* LastChar();
  int32_t GetLineEnd() const;

  void Clear();

  void IncrementArabicCharCount();
  void DecrementArabicCharCount();
  bool HasArabicChar() const { return arabic_chars_ > 0; }

  std::vector<CFGAS_Char> line_chars_;
  std::vector<CFGAS_BreakPiece> line_pieces_;
  int32_t start_ = 0;
  int32_t width_ = 0;

 private:
  int32_t arabic_chars_ = 0;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAKLINE_H_
