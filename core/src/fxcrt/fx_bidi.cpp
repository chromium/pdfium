// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcrt/fx_bidi.h"
#include "core/include/fxcrt/fx_ucd.h"

CFX_BidiChar::CFX_BidiChar()
    : m_iCurStart(0),
      m_iCurCount(0),
      m_CurBidi(NEUTRAL),
      m_iLastStart(0),
      m_iLastCount(0),
      m_LastBidi(NEUTRAL) {
}

CFX_BidiChar::~CFX_BidiChar() {
}

bool CFX_BidiChar::AppendChar(FX_WCHAR wch) {
  FX_DWORD dwProps = FX_GetUnicodeProperties(wch);
  int32_t iBidiCls = (dwProps & FX_BIDICLASSBITSMASK) >> FX_BIDICLASSBITS;
  Direction bidi = NEUTRAL;
  switch (iBidiCls) {
    case FX_BIDICLASS_L:
    case FX_BIDICLASS_AN:
    case FX_BIDICLASS_EN:
      bidi = LEFT;
      break;
    case FX_BIDICLASS_R:
    case FX_BIDICLASS_AL:
      bidi = RIGHT;
      break;
  }

  bool bRet = (bidi != m_CurBidi);
  if (bRet) {
    SaveCurrentStateToLastState();
    m_CurBidi = bidi;
  }
  m_iCurCount++;
  return bRet;
}

bool CFX_BidiChar::EndChar() {
  SaveCurrentStateToLastState();
  return m_iLastCount > 0;
}

CFX_BidiChar::Direction CFX_BidiChar::GetBidiInfo(int32_t* iStart,
                                                  int32_t* iCount) const {
  if (iStart)
    *iStart = m_iLastStart;
  if (iCount)
    *iCount = m_iLastCount;
  return m_LastBidi;
}

void CFX_BidiChar::SaveCurrentStateToLastState() {
  m_LastBidi = m_CurBidi;
  m_iLastStart = m_iCurStart;
  m_iCurStart = m_iCurCount;
  m_iLastCount = m_iCurCount - m_iLastStart;
}
