// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_breakline.h"

#include "core/fxcrt/stl_util.h"
#include "third_party/base/check.h"

CFGAS_BreakLine::CFGAS_BreakLine() = default;

CFGAS_BreakLine::~CFGAS_BreakLine() = default;

CFGAS_Char* CFGAS_BreakLine::GetChar(int32_t index) {
  DCHECK(fxcrt::IndexInBounds(m_LineChars, index));
  return &m_LineChars[index];
}

int32_t CFGAS_BreakLine::GetLineEnd() const {
  return m_iStart + m_iWidth;
}

void CFGAS_BreakLine::Clear() {
  m_LineChars.clear();
  m_LinePieces.clear();
  m_iWidth = 0;
  m_iArabicChars = 0;
}

void CFGAS_BreakLine::IncrementArabicCharCount() {
  ++m_iArabicChars;
}

void CFGAS_BreakLine::DecrementArabicCharCount() {
  DCHECK(m_iArabicChars > 0);
  --m_iArabicChars;
}
