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

bool IsValidFormCalcCharacter(const wchar_t* p) {
  return *p == 0 || (*p >= 0x09 && *p <= 0x0D) ||
         (*p >= 0x20 && *p <= 0xd7FF) || (*p >= 0xE000 && *p <= 0xFFFD);
}

bool IsValidIdentifierCharacter(const wchar_t* p) {
  return u_isalnum(*p) || *p == 0x005F ||  // '_'
         *p == 0x0024;                     // '$'
}

bool IsValidInitialIdentifierCharacter(const wchar_t* p) {
  return u_isalpha(*p) || *p == 0x005F ||  // '_'
         *p == 0x0024 ||                   // '$'
         *p == 0x0021;                     // '!'
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
  // Make sure we don't walk off the end of the string.
  if (m_ptr > m_end) {
    m_pToken = pdfium::MakeUnique<CXFA_FMToken>(m_uCurrentLine);
    m_pToken->m_type = TOKeof;
  } else {
    m_pToken = Scan();
  }
  return m_pToken.get();
}

std::unique_ptr<CXFA_FMToken> CXFA_FMLexer::Scan() {
  wchar_t ch = 0;
  auto p = pdfium::MakeUnique<CXFA_FMToken>(m_uCurrentLine);
  if (!IsValidFormCalcCharacter(m_ptr)) {
    ch = *m_ptr;
    m_LexerError = true;
    return p;
  }

  while (1) {
    // Make sure we don't walk off the end of the string. If we don't currently
    // have a token type then mark it EOF.
    if (m_ptr > m_end) {
      if (p->m_type == TOKreserver)
        p->m_type = TOKeof;
      return p;
    }

    ch = *m_ptr;
    if (!IsValidFormCalcCharacter(m_ptr)) {
      m_LexerError = true;
      return p;
    }

    switch (ch) {
      case 0:
        p->m_type = TOKeof;
        return p;
      case 0x0A:
        ++m_uCurrentLine;
        p->m_uLinenum = m_uCurrentLine;
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
        p->m_type = TOKstring;
        m_ptr = String(p.get(), m_ptr);
        return p;
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
        p->m_type = TOKnumber;
        m_ptr = Number(p.get(), m_ptr);
        return p;
      }
      case '=':
        ++m_ptr;
        if (m_ptr > m_end) {
          p->m_type = TOKassign;
          return p;
        }

        if (IsValidFormCalcCharacter(m_ptr)) {
          ch = *m_ptr;
          if (ch == '=') {
            p->m_type = TOKeq;
            ++m_ptr;
          } else {
            p->m_type = TOKassign;
          }
        } else {
          ch = *m_ptr;
          m_LexerError = true;
        }
        return p;
      case '<':
        ++m_ptr;
        if (m_ptr > m_end) {
          p->m_type = TOKlt;
          return p;
        }

        if (IsValidFormCalcCharacter(m_ptr)) {
          ch = *m_ptr;
          if (ch == '=') {
            p->m_type = TOKle;
            ++m_ptr;
          } else if (ch == '>') {
            p->m_type = TOKne;
            ++m_ptr;
          } else {
            p->m_type = TOKlt;
          }
        } else {
          ch = *m_ptr;
          m_LexerError = true;
        }
        return p;
      case '>':
        ++m_ptr;
        if (m_ptr > m_end) {
          p->m_type = TOKgt;
          return p;
        }

        if (IsValidFormCalcCharacter(m_ptr)) {
          ch = *m_ptr;
          if (ch == '=') {
            p->m_type = TOKge;
            ++m_ptr;
          } else {
            p->m_type = TOKgt;
          }
        } else {
          ch = *m_ptr;
          m_LexerError = true;
        }
        return p;
      case ',':
        p->m_type = TOKcomma;
        ++m_ptr;
        return p;
      case '(':
        p->m_type = TOKlparen;
        ++m_ptr;
        return p;
      case ')':
        p->m_type = TOKrparen;
        ++m_ptr;
        return p;
      case '[':
        p->m_type = TOKlbracket;
        ++m_ptr;
        return p;
      case ']':
        p->m_type = TOKrbracket;
        ++m_ptr;
        return p;
      case '&':
        ++m_ptr;
        p->m_type = TOKand;
        return p;
      case '|':
        ++m_ptr;
        p->m_type = TOKor;
        return p;
      case '+':
        ++m_ptr;
        p->m_type = TOKplus;
        return p;
      case '-':
        ++m_ptr;
        p->m_type = TOKminus;
        return p;
      case '*':
        ++m_ptr;
        p->m_type = TOKmul;
        return p;
      case '/': {
        ++m_ptr;
        if (m_ptr > m_end) {
          p->m_type = TOKdiv;
          return p;
        }

        if (!IsValidFormCalcCharacter(m_ptr)) {
          ch = *m_ptr;
          m_LexerError = true;
          return p;
        }
        ch = *m_ptr;
        if (ch != '/') {
          p->m_type = TOKdiv;
          return p;
        }
        m_ptr = Comment(m_ptr);
        break;
      }
      case '.':
        ++m_ptr;
        if (m_ptr > m_end) {
          p->m_type = TOKdot;
          return p;
        }

        if (IsValidFormCalcCharacter(m_ptr)) {
          ch = *m_ptr;
          if (ch == '.') {
            p->m_type = TOKdotdot;
            ++m_ptr;
          } else if (ch == '*') {
            p->m_type = TOKdotstar;
            ++m_ptr;
          } else if (ch == '#') {
            p->m_type = TOKdotscream;
            ++m_ptr;
          } else if (ch <= '9' && ch >= '0') {
            p->m_type = TOKnumber;
            --m_ptr;
            m_ptr = Number(p.get(), m_ptr);
          } else {
            p->m_type = TOKdot;
          }
        } else {
          ch = *m_ptr;
          m_LexerError = true;
        }
        return p;
      case 0x09:
      case 0x0B:
      case 0x0C:
      case 0x20:
        ++m_ptr;
        break;
      default: {
        if (!IsValidInitialIdentifierCharacter(m_ptr)) {
          m_LexerError = true;
          return p;
        }
        m_ptr = Identifiers(p.get(), m_ptr);
        return p;
      }
    }
  }
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
  const wchar_t* pStart = p;

  ++p;
  if (p > m_end) {
    m_LexerError = true;
    return p;
  }

  uint16_t ch = *p;
  while (ch) {
    if (!IsValidFormCalcCharacter(p)) {
      ch = *p;
      t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
      m_LexerError = true;
      return p;
    }

    ++p;
    if (ch != '"') {
      // We've hit the end of the input, return the string.
      if (p > m_end) {
        m_LexerError = true;
        return p;
      }
      ch = *p;
      continue;
    }
    // We've hit the end of the input, return the string.
    if (p > m_end)
      break;

    if (!IsValidFormCalcCharacter(p)) {
      ch = *p;
      t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
      m_LexerError = true;
      return p;
    }
    ch = *p;
    if (ch != '"')
      break;

    ++p;
    if (p > m_end) {
      m_LexerError = true;
      return p;
    }
    ch = *p;
  }
  t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
  return p;
}

const wchar_t* CXFA_FMLexer::Identifiers(CXFA_FMToken* t, const wchar_t* p) {
  const wchar_t* pStart = p;
  ++p;
  while (p <= m_end && *p) {
    if (!IsValidFormCalcCharacter(p)) {
      t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
      m_LexerError = true;
      return p;
    }

    if (!IsValidIdentifierCharacter(p)) {
      break;
    }
    ++p;
    if (p > m_end)
      break;
  }
  t->m_wstring = CFX_WideStringC(pStart, (p - pStart));
  t->m_type = IsKeyword(t->m_wstring);
  return p;
}

const wchar_t* CXFA_FMLexer::Comment(const wchar_t* p) {
  ++p;

  if (p > m_end)
    return p;

  unsigned ch = *p;
  while (ch) {
    ++p;
    if (ch == L'\r')
      return p;
    if (ch == L'\n') {
      ++m_uCurrentLine;
      return p;
    }
    if (p > m_end)
      return p;
    ch = *p;
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
