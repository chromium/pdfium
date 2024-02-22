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
  return m_iWidth < 0 ? m_iStartPos : m_iStartPos + m_iWidth;
}

size_t CFGAS_BreakPiece::GetLength() const {
  return pdfium::checked_cast<size_t>(m_iCharCount);
}

CFGAS_Char* CFGAS_BreakPiece::GetChar(int32_t index) const {
  return GetChar(pdfium::checked_cast<size_t>(index));
}

CFGAS_Char* CFGAS_BreakPiece::GetChar(size_t index) const {
  DCHECK_LT(index, GetLength());
  DCHECK(m_pChars);
  return &(*m_pChars)[m_iStartChar + index];
}

WideString CFGAS_BreakPiece::GetString() const {
  WideString ret;
  ret.Reserve(m_iCharCount);
  for (int32_t i = m_iStartChar; i < m_iStartChar + m_iCharCount; i++)
    ret += static_cast<wchar_t>((*m_pChars)[i].char_code());
  return ret;
}

std::vector<int32_t> CFGAS_BreakPiece::GetWidths() const {
  std::vector<int32_t> ret;
  ret.reserve(m_iCharCount);
  for (int32_t i = m_iStartChar; i < m_iStartChar + m_iCharCount; i++)
    ret.push_back((*m_pChars)[i].m_iCharWidth);
  return ret;
}
