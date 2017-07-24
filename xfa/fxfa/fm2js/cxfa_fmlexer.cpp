// Copright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"

#include <algorithm>

#include "core/fxcrt/fx_extension.h"
#include "third_party/base/ptr_util.h"
#include "third_party/icu/source/common/unicode/uchar.h"

namespace {

bool IsValidFormCalcCharacter(wchar_t c) {
  return c == 0 || (c >= 0x09 && c <= 0x0D) || (c >= 0x20 && c <= 0xd7FF) ||
         (c >= 0xE000 && c <= 0xFFFD);
}

bool IsValidIdentifierCharacter(wchar_t c) {
  return u_isalnum(c) || c == 0x005F ||  // '_'
         c == 0x0024;                    // '$'
}

bool IsValidInitialIdentifierCharacter(wchar_t c) {
  return u_isalpha(c) || c == 0x005F ||  // '_'
         c == 0x0024 ||                  // '$'
         c == 0x0021;                    // '!'
}

const XFA_FMKeyword keyWords[] = {
    {TOKand, 0x00000026, L"&"},
    {TOKlparen, 0x00000028, L"("},
    {TOKrparen, 0x00000029, L")"},
    {TOKmul, 0x0000002a, L"*"},
    {TOKplus, 0x0000002b, L"+"},
    {TOKcomma, 0x0000002c, L","},
    {TOKminus, 0x0000002d, L"-"},
    {TOKdot, 0x0000002e, L"."},
    {TOKdiv, 0x0000002f, L"/"},
    {TOKlt, 0x0000003c, L"<"},
    {TOKassign, 0x0000003d, L"="},
    {TOKgt, 0x0000003e, L">"},
    {TOKlbracket, 0x0000005b, L"["},
    {TOKrbracket, 0x0000005d, L"]"},
    {TOKor, 0x0000007c, L"|"},
    {TOKdotscream, 0x0000ec11, L".#"},
    {TOKdotstar, 0x0000ec18, L".*"},
    {TOKdotdot, 0x0000ec1c, L".."},
    {TOKle, 0x000133f9, L"<="},
    {TOKne, 0x000133fa, L"<>"},
    {TOKeq, 0x0001391a, L"=="},
    {TOKge, 0x00013e3b, L">="},
    {TOKdo, 0x00020153, L"do"},
    {TOKkseq, 0x00020676, L"eq"},
    {TOKksge, 0x000210ac, L"ge"},
    {TOKksgt, 0x000210bb, L"gt"},
    {TOKif, 0x00021aef, L"if"},
    {TOKin, 0x00021af7, L"in"},
    {TOKksle, 0x00022a51, L"le"},
    {TOKkslt, 0x00022a60, L"lt"},
    {TOKksne, 0x00023493, L"ne"},
    {TOKksor, 0x000239c1, L"or"},
    {TOKnull, 0x052931bb, L"null"},
    {TOKbreak, 0x05518c25, L"break"},
    {TOKksand, 0x09f9db33, L"and"},
    {TOKend, 0x0a631437, L"end"},
    {TOKeof, 0x0a63195a, L"eof"},
    {TOKfor, 0x0a7d67a7, L"for"},
    {TOKnan, 0x0b4f91dd, L"nan"},
    {TOKksnot, 0x0b4fd9b1, L"not"},
    {TOKvar, 0x0c2203e9, L"var"},
    {TOKthen, 0x2d5738cf, L"then"},
    {TOKelse, 0x45f65ee9, L"else"},
    {TOKexit, 0x4731d6ba, L"exit"},
    {TOKdownto, 0x4caadc3b, L"downto"},
    {TOKreturn, 0x4db8bd60, L"return"},
    {TOKinfinity, 0x5c0a010a, L"infinity"},
    {TOKendwhile, 0x5c64bff0, L"endwhile"},
    {TOKforeach, 0x67e31f38, L"foreach"},
    {TOKendfunc, 0x68f984a3, L"endfunc"},
    {TOKelseif, 0x78253218, L"elseif"},
    {TOKwhile, 0x84229259, L"while"},
    {TOKendfor, 0x8ab49d7e, L"endfor"},
    {TOKthrow, 0x8db05c94, L"throw"},
    {TOKstep, 0xa7a7887c, L"step"},
    {TOKupto, 0xb5155328, L"upto"},
    {TOKcontinue, 0xc0340685, L"continue"},
    {TOKfunc, 0xcdce60ec, L"func"},
    {TOKendif, 0xe0e8fee6, L"endif"},
};

const XFA_FM_TOKEN KEYWORD_START = TOKdo;
const XFA_FM_TOKEN KEYWORD_END = TOKendif;

}  // namespace

const wchar_t* XFA_FM_KeywordToString(XFA_FM_TOKEN op) {
  if (op < KEYWORD_START || op > KEYWORD_END)
    return L"";
  return keyWords[op].m_keyword;
}

CXFA_FMToken::CXFA_FMToken() : m_type(TOKreserver), m_uLinenum(1) {}

CXFA_FMToken::CXFA_FMToken(uint32_t uLineNum)
    : m_type(TOKreserver), m_uLinenum(uLineNum) {}

CXFA_FMToken::~CXFA_FMToken() {}

CXFA_FMLexer::CXFA_FMLexer(const CFX_WideStringC& wsFormCalc)
    : m_ptr(wsFormCalc.unterminated_c_str()),
      m_end(m_ptr + wsFormCalc.GetLength() - 1),
      m_uCurrentLine(1),
      m_LexerError(false) {}

CXFA_FMLexer::~CXFA_FMLexer() {}

CXFA_FMToken* CXFA_FMLexer::NextToken() {
  m_pToken = pdfium::MakeUnique<CXFA_FMToken>(m_uCurrentLine);
  while (m_ptr <= m_end && *m_ptr) {
    if (!IsValidFormCalcCharacter(*m_ptr)) {
      m_LexerError = true;
      return m_pToken.get();
    }

    switch (*m_ptr) {
      case 0x0A:
        ++m_uCurrentLine;
        m_pToken->m_uLinenum = m_uCurrentLine;
        ++m_ptr;
        break;
      case 0x0D:
        ++m_ptr;
        break;
      case ';': {
        m_ptr = Comment(m_ptr);
        break;
      }
      case '"': {
        m_pToken->m_type = TOKstring;
        m_ptr = String(m_pToken.get(), m_ptr);
        return m_pToken.get();
      }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        m_pToken->m_type = TOKnumber;
        m_ptr = Number(m_pToken.get(), m_ptr);
        return m_pToken.get();
      }
      case '=':
        ++m_ptr;
        if (m_ptr > m_end) {
          m_pToken->m_type = TOKassign;
          return m_pToken.get();
        }

        if (IsValidFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_pToken->m_type = TOKeq;
            ++m_ptr;
          } else {
            m_pToken->m_type = TOKassign;
          }
        } else {
          m_LexerError = true;
        }
        return m_pToken.get();
      case '<':
        ++m_ptr;
        if (m_ptr > m_end) {
          m_pToken->m_type = TOKlt;
          return m_pToken.get();
        }

        if (IsValidFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_pToken->m_type = TOKle;
            ++m_ptr;
          } else if (*m_ptr == '>') {
            m_pToken->m_type = TOKne;
            ++m_ptr;
          } else {
            m_pToken->m_type = TOKlt;
          }
        } else {
          m_LexerError = true;
        }
        return m_pToken.get();
      case '>':
        ++m_ptr;
        if (m_ptr > m_end) {
          m_pToken->m_type = TOKgt;
          return m_pToken.get();
        }

        if (IsValidFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_pToken->m_type = TOKge;
            ++m_ptr;
          } else {
            m_pToken->m_type = TOKgt;
          }
        } else {
          m_LexerError = true;
        }
        return m_pToken.get();
      case ',':
        m_pToken->m_type = TOKcomma;
        ++m_ptr;
        return m_pToken.get();
      case '(':
        m_pToken->m_type = TOKlparen;
        ++m_ptr;
        return m_pToken.get();
      case ')':
        m_pToken->m_type = TOKrparen;
        ++m_ptr;
        return m_pToken.get();
      case '[':
        m_pToken->m_type = TOKlbracket;
        ++m_ptr;
        return m_pToken.get();
      case ']':
        m_pToken->m_type = TOKrbracket;
        ++m_ptr;
        return m_pToken.get();
      case '&':
        ++m_ptr;
        m_pToken->m_type = TOKand;
        return m_pToken.get();
      case '|':
        ++m_ptr;
        m_pToken->m_type = TOKor;
        return m_pToken.get();
      case '+':
        ++m_ptr;
        m_pToken->m_type = TOKplus;
        return m_pToken.get();
      case '-':
        ++m_ptr;
        m_pToken->m_type = TOKminus;
        return m_pToken.get();
      case '*':
        ++m_ptr;
        m_pToken->m_type = TOKmul;
        return m_pToken.get();
      case '/': {
        ++m_ptr;
        if (m_ptr > m_end) {
          m_pToken->m_type = TOKdiv;
          return m_pToken.get();
        }

        if (!IsValidFormCalcCharacter(*m_ptr)) {
          m_LexerError = true;
          return m_pToken.get();
        }
        if (*m_ptr != '/') {
          m_pToken->m_type = TOKdiv;
          return m_pToken.get();
        }
        m_ptr = Comment(m_ptr);
        break;
      }
      case '.':
        ++m_ptr;
        if (m_ptr > m_end) {
          m_pToken->m_type = TOKdot;
          return m_pToken.get();
        }

        if (IsValidFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '.') {
            m_pToken->m_type = TOKdotdot;
            ++m_ptr;
          } else if (*m_ptr == '*') {
            m_pToken->m_type = TOKdotstar;
            ++m_ptr;
          } else if (*m_ptr == '#') {
            m_pToken->m_type = TOKdotscream;
            ++m_ptr;
          } else if (*m_ptr <= '9' && *m_ptr >= '0') {
            m_pToken->m_type = TOKnumber;
            --m_ptr;
            m_ptr = Number(m_pToken.get(), m_ptr);
          } else {
            m_pToken->m_type = TOKdot;
          }
        } else {
          m_LexerError = true;
        }
        return m_pToken.get();
      case 0x09:
      case 0x0B:
      case 0x0C:
      case 0x20:
        ++m_ptr;
        break;
      default: {
        if (!IsValidInitialIdentifierCharacter(*m_ptr)) {
          m_LexerError = true;
          return m_pToken.get();
        }
        m_ptr = Identifiers(m_pToken.get(), m_ptr);
        return m_pToken.get();
      }
    }
  }

  // If there isn't currently a token type then mark it EOF.
  if (m_pToken->m_type == TOKreserver)
    m_pToken->m_type = TOKeof;
  return m_pToken.get();
}

const wchar_t* CXFA_FMLexer::Number(CXFA_FMToken* t, const wchar_t* p) {
  // This will set pEnd to the character after the end of the number.
  wchar_t* pEnd = nullptr;
  if (p)
    wcstod(const_cast<wchar_t*>(p), &pEnd);
  if (pEnd && FXSYS_iswalpha(*pEnd)) {
    m_LexerError = true;
    return pEnd;
  }

  t->m_wstring = CFX_WideStringC(p, (pEnd - p));
  return pEnd;
}

const wchar_t* CXFA_FMLexer::String(CXFA_FMToken* t, const wchar_t* p) {
  const wchar_t* start = p;
  ++p;
  while (p <= m_end && *p) {
    if (!IsValidFormCalcCharacter(*p))
      break;

    if (*p == '"') {
      // Check for escaped "s, i.e. "".
      ++p;
      // If the end of the input has been reached it was not escaped.
      if (p > m_end) {
        t->m_wstring = CFX_WideStringC(start, (p - start));
        return p;
      }
      // If the next character is not a " then the end of the string has been
      // found.
      if (*p != '"') {
        if (!IsValidFormCalcCharacter(*p)) {
          break;
        }
        t->m_wstring = CFX_WideStringC(start, (p - start));
        return p;
      }
    }
    ++p;
  }

  // Didn't find the end of the string.
  t->m_wstring = CFX_WideStringC(start, (p - start));
  m_LexerError = true;
  return p;
}

const wchar_t* CXFA_FMLexer::Identifiers(CXFA_FMToken* t, const wchar_t* p) {
  const wchar_t* pStart = p;
  ++p;
  while (p <= m_end && *p) {
    if (!IsValidFormCalcCharacter(*p)) {
      t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
      m_LexerError = true;
      return p;
    }

    if (!IsValidIdentifierCharacter(*p)) {
      break;
    }
    ++p;
  }
  t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
  t->m_type = IsKeyword(t->m_wstring);
  return p;
}

const wchar_t* CXFA_FMLexer::Comment(const wchar_t* p) {
  p++;
  while (p <= m_end && *p) {
    if (*p == L'\r')
      return ++p;
    if (*p == L'\n') {
      ++m_uCurrentLine;
      return ++p;
    }
    ++p;
  }
  return p;
}

XFA_FM_TOKEN CXFA_FMLexer::IsKeyword(const CFX_WideStringC& str) {
  uint32_t key = FX_HashCode_GetW(str, true);
  auto cmpFunc = [](const XFA_FMKeyword& iter, const uint32_t& val) {
    return iter.m_uHash < val;
  };

  const XFA_FMKeyword* result = std::lower_bound(
      std::begin(keyWords) + KEYWORD_START, std::end(keyWords), key, cmpFunc);
  if (result <= keyWords + KEYWORD_END && result->m_uHash == key) {
    return result->m_type;
  }
  return TOKidentifier;
}
