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
  bool HasArabicChar() const { return m_iArabicChars > 0; }

  std::vector<CFGAS_Char> m_LineChars;
  std::vector<CFGAS_BreakPiece> m_LinePieces;
  int32_t m_iStart = 0;
  int32_t m_iWidth = 0;

 private:
  int32_t m_iArabicChars = 0;
};

#endif  // XFA_FGAS_LAYOUT_CFGAS_BREAKLINE_H_
