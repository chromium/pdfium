// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfx_breakline.h"

#include "third_party/base/stl_util.h"

CFX_BreakLine::CFX_BreakLine() : m_iStart(0), m_iWidth(0), m_iArabicChars(0) {}

CFX_BreakLine::~CFX_BreakLine() {}

int32_t CFX_BreakLine::CountChars() const {
  return pdfium::CollectionSize<int32_t>(m_LineChars);
}

CFX_Char* CFX_BreakLine::GetChar(int32_t index) {
  ASSERT(pdfium::IndexInBounds(m_LineChars, index));
  return &m_LineChars[index];
}

const CFX_Char* CFX_BreakLine::GetChar(int32_t index) const {
  ASSERT(pdfium::IndexInBounds(m_LineChars, index));
  return &m_LineChars[index];
}

int32_t CFX_BreakLine::CountPieces() const {
  return pdfium::CollectionSize<int32_t>(m_LinePieces);
}

const CFX_BreakPiece* CFX_BreakLine::GetPiece(int32_t index) const {
  ASSERT(index >= 0 && index < CountPieces());
  return &m_LinePieces[index];
}

int32_t CFX_BreakLine::GetLineEnd() const {
  return m_iStart + m_iWidth;
}

void CFX_BreakLine::Clear() {
  m_LineChars.clear();
  m_LinePieces.clear();
  m_iWidth = 0;
  m_iArabicChars = 0;
}
