// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssvaluelistparser.h"

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_system.h"

CFX_CSSValueListParser::CFX_CSSValueListParser(WideStringView list,
                                               wchar_t separator)
    : m_Separator(separator),
      m_pCur(list.unterminated_c_str()),
      m_pEnd(list.unterminated_c_str() + list.GetLength()) {
  DCHECK_NE(m_pCur, m_pEnd);
}

std::optional<CFX_CSSValueListParser::Result>
CFX_CSSValueListParser::NextValue() {
  while (m_pCur < m_pEnd && (*m_pCur <= ' ' || *m_pCur == m_Separator)) {
    ++m_pCur;
  }
  if (m_pCur >= m_pEnd) {
    return std::nullopt;
  }
  auto eType = CFX_CSSValue::PrimitiveType::kUnknown;
  const wchar_t* pStart = m_pCur;
  size_t nLength = 0;
  wchar_t wch = *m_pCur;
  if (wch == '#') {
    nLength = SkipToChar(' ');
    if (nLength == 4 || nLength == 7) {
      eType = CFX_CSSValue::PrimitiveType::kRGB;
    }
  } else if (FXSYS_IsDecimalDigit(wch) || wch == '.' || wch == '-' ||
             wch == '+') {
    while (m_pCur < m_pEnd && (*m_pCur > ' ' && *m_pCur != m_Separator)) {
      ++m_pCur;
    }
    nLength = m_pCur - pStart;
    eType = CFX_CSSValue::PrimitiveType::kNumber;
  } else if (wch == '\"' || wch == '\'') {
    ++pStart;
    ++m_pCur;
    nLength = SkipToChar(wch);
    ++m_pCur;
    eType = CFX_CSSValue::PrimitiveType::kString;
  } else if (m_pEnd - m_pCur > 5 && m_pCur[3] == '(') {
    if (FXSYS_wcsnicmp(L"rgb", m_pCur, 3) == 0) {
      nLength = SkipToChar(')') + 1;
      ++m_pCur;
      eType = CFX_CSSValue::PrimitiveType::kRGB;
    }
  } else {
    nLength = SkipToCharMatchingParens(m_Separator);
    eType = CFX_CSSValue::PrimitiveType::kString;
  }
  if (m_pCur <= m_pEnd && nLength > 0) {
    return Result{eType, WideStringView(pStart, nLength)};
  }
  return std::nullopt;
}

size_t CFX_CSSValueListParser::SkipToChar(wchar_t wch) {
  const wchar_t* pStart = m_pCur;
  while (m_pCur < m_pEnd && *m_pCur != wch) {
    m_pCur++;
  }
  return m_pCur - pStart;
}

size_t CFX_CSSValueListParser::SkipToCharMatchingParens(wchar_t wch) {
  const wchar_t* pStart = m_pCur;
  int64_t bracketCount = 0;
  while (m_pCur < m_pEnd && *m_pCur != wch) {
    if (*m_pCur <= ' ')
      break;
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
