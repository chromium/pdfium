// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssvaluelistparser.h"

CFDE_CSSValueListParser::CFDE_CSSValueListParser(const FX_WCHAR* psz,
                                                 int32_t iLen,
                                                 FX_WCHAR separator)
    : m_Separator(separator), m_pCur(psz), m_pEnd(psz + iLen) {
  ASSERT(psz && iLen > 0);
}

bool CFDE_CSSValueListParser::NextValue(FDE_CSSPrimitiveType& eType,
                                        const FX_WCHAR*& pStart,
                                        int32_t& iLength) {
  while (m_pCur < m_pEnd && (*m_pCur <= ' ' || *m_pCur == m_Separator))
    ++m_pCur;

  if (m_pCur >= m_pEnd)
    return false;

  eType = FDE_CSSPrimitiveType::Unknown;
  pStart = m_pCur;
  iLength = 0;
  FX_WCHAR wch = *m_pCur;
  if (wch == '#') {
    iLength = SkipTo(' ', false, false);
    if (iLength == 4 || iLength == 7)
      eType = FDE_CSSPrimitiveType::RGB;
  } else if ((wch >= '0' && wch <= '9') || wch == '.' || wch == '-' ||
             wch == '+') {
    while (m_pCur < m_pEnd && (*m_pCur > ' ' && *m_pCur != m_Separator))
      ++m_pCur;

    iLength = m_pCur - pStart;
    eType = FDE_CSSPrimitiveType::Number;
  } else if (wch == '\"' || wch == '\'') {
    pStart++;
    m_pCur++;
    iLength = SkipTo(wch, false, false);
    m_pCur++;
    eType = FDE_CSSPrimitiveType::String;
  } else if (m_pEnd - m_pCur > 5 && m_pCur[3] == '(') {
    if (FXSYS_wcsnicmp(L"rgb", m_pCur, 3) == 0) {
      iLength = SkipTo(')', false, false) + 1;
      m_pCur++;
      eType = FDE_CSSPrimitiveType::RGB;
    }
  } else {
    iLength = SkipTo(m_Separator, true, true);
    eType = FDE_CSSPrimitiveType::String;
  }
  return m_pCur <= m_pEnd && iLength > 0;
}

int32_t CFDE_CSSValueListParser::SkipTo(FX_WCHAR wch,
                                        bool breakOnSpace,
                                        bool matchBrackets) {
  const FX_WCHAR* pStart = m_pCur;
  int32_t bracketCount = 0;
  while (m_pCur < m_pEnd && *m_pCur != wch) {
    if (breakOnSpace && *m_pCur <= ' ')
      break;
    if (!matchBrackets) {
      m_pCur++;
      continue;
    }

    if (*m_pCur == '(')
      bracketCount++;
    else if (*m_pCur == ')')
      bracketCount--;

    m_pCur++;
  }

  while (bracketCount > 0 && m_pCur < m_pEnd) {
    if (*m_pCur == ')')
      bracketCount--;
    m_pCur++;
  }
  return m_pCur - pStart;
}
