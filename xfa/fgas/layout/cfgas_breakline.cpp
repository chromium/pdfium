// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_breakline.h"

#include "core/fxcrt/check.h"

CFGAS_BreakLine::CFGAS_BreakLine() = default;

CFGAS_BreakLine::~CFGAS_BreakLine() = default;

CFGAS_Char* CFGAS_BreakLine::LastChar() {
  if (line_chars_.empty()) {
    return nullptr;
  }

  return &line_chars_.back();
}

int32_t CFGAS_BreakLine::GetLineEnd() const {
  return start_ + width_;
}

void CFGAS_BreakLine::Clear() {
  line_chars_.clear();
  line_pieces_.clear();
  width_ = 0;
  arabic_chars_ = 0;
}

void CFGAS_BreakLine::IncrementArabicCharCount() {
  ++arabic_chars_;
}

void CFGAS_BreakLine::DecrementArabicCharCount() {
  DCHECK(arabic_chars_ > 0);
  --arabic_chars_;
}
