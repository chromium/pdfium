// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfx_breakpiece.h"

#include "xfa/fgas/layout/cfx_textuserdata.h"

CFX_BreakPiece::CFX_BreakPiece() = default;

CFX_BreakPiece::CFX_BreakPiece(const CFX_BreakPiece& other) = default;

CFX_BreakPiece::~CFX_BreakPiece() = default;

int32_t CFX_BreakPiece::GetEndPos() const {
  return m_iWidth < 0 ? m_iStartPos : m_iStartPos + m_iWidth;
}

CFX_Char* CFX_BreakPiece::GetChar(int32_t index) const {
  ASSERT(index >= 0);
  ASSERT(index < m_iChars);
  ASSERT(m_pChars);
  return &(*m_pChars)[m_iStartChar + index];
}

WideString CFX_BreakPiece::GetString() const {
  WideString ret;
  ret.Reserve(m_iChars);
  for (int32_t i = m_iStartChar; i < m_iStartChar + m_iChars; i++)
    ret += static_cast<wchar_t>((*m_pChars)[i].char_code());
  return ret;
}

std::vector<int32_t> CFX_BreakPiece::GetWidths() const {
  std::vector<int32_t> ret;
  ret.reserve(m_iChars);
  for (int32_t i = m_iStartChar; i < m_iStartChar + m_iChars; i++)
    ret.push_back((*m_pChars)[i].m_iCharWidth);
  return ret;
}
