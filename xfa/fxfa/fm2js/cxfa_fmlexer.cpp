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

bool IsFormCalcCharacter(wchar_t c) {
  return (c >= 0x09 && c <= 0x0D) || (c >= 0x20 && c <= 0xd7FF) ||
         (c >= 0xE000 && c <= 0xFFFD);
}

bool IsIdentifierCharacter(wchar_t c) {
  return u_isalnum(c) || c == 0x005F ||  // '_'
         c == 0x0024;                    // '$'
}

bool IsInitialIdentifierCharacter(wchar_t c) {
  return u_isalpha(c) || c == 0x005F ||  // '_'
         c == 0x0024 ||                  // '$'
         c == 0x0021;                    // '!'
}

bool IsWhitespaceCharacter(wchar_t c) {
  return c == 0x0009 ||  // Horizontal tab
         c == 0x000B ||  // Vertical tab
         c == 0x000C ||  // Form feed
         c == 0x0020;    // Space
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

const wchar_t* tokenStrings[] = {
    L"TOKand",        L"TOKlparen",     L"TOKrparen",   L"TOKmul",
    L"TOKplus",       L"TOKcomma",      L"TOKminus",    L"TOKdot",
    L"TOKdiv",        L"TOKlt",         L"TOKassign",   L"TOKgt",
    L"TOKlbracket",   L"TOKrbracket",   L"TOKor",       L"TOKdotscream",
    L"TOKdotstar",    L"TOKdotdot",     L"TOKle",       L"TOKne",
    L"TOKeq",         L"TOKge",         L"TOKdo",       L"TOKkseq",
    L"TOKksge",       L"TOKksgt",       L"TOKif",       L"TOKin",
    L"TOKksle",       L"TOKkslt",       L"TOKksne",     L"TOKksor",
    L"TOKnull",       L"TOKbreak",      L"TOKksand",    L"TOKend",
    L"TOKeof",        L"TOKfor",        L"TOKnan",      L"TOKksnot",
    L"TOKvar",        L"TOKthen",       L"TOKelse",     L"TOKexit",
    L"TOKdownto",     L"TOKreturn",     L"TOKinfinity", L"TOKendwhile",
    L"TOKforeach",    L"TOKendfunc",    L"TOKelseif",   L"TOKwhile",
    L"TOKendfor",     L"TOKthrow",      L"TOKstep",     L"TOKupto",
    L"TOKcontinue",   L"TOKfunc",       L"TOKendif",    L"TOKstar",
    L"TOKidentifier", L"TOKunderscore", L"TOKdollar",   L"TOKexclamation",
    L"TOKcall",       L"TOKstring",     L"TOKnumber",   L"TOKreserver",
};

XFA_FM_TOKEN TokenizeIdentifier(const WideStringView& str) {
  uint32_t key = FX_HashCode_GetW(str, true);

  const XFA_FMKeyword* end = std::begin(keyWords) + KEYWORD_END + 1;
  const XFA_FMKeyword* result =
      std::lower_bound(std::begin(keyWords) + KEYWORD_START, end, key,
                       [](const XFA_FMKeyword& iter, const uint32_t& val) {
                         return iter.m_hash < val;
                       });
  if (result != end && result->m_hash == key)
    return result->m_type;
  return TOKidentifier;
}

}  // namespace

CXFA_FMToken::CXFA_FMToken() : m_type(TOKreserver), m_line_num(1) {}

CXFA_FMToken::CXFA_FMToken(uint32_t line_num)
    : m_type(TOKreserver), m_line_num(line_num) {}

CXFA_FMToken::~CXFA_FMToken() {}

WideString CXFA_FMToken::ToDebugString() const {
  WideString str(L"type = ");
  str += tokenStrings[m_type];
  str += L", string = ";
  str += m_string;
  str += L", line_num = ";
  str += std::to_wstring(m_line_num).c_str();
  return str;
}

CXFA_FMLexer::CXFA_FMLexer(const WideStringView& wsFormCalc)
    : m_cursor(wsFormCalc.unterminated_c_str()),
      m_end(m_cursor + wsFormCalc.GetLength() - 1),
      m_current_line(1),
      m_lexer_error(false) {}

CXFA_FMLexer::~CXFA_FMLexer() {}

std::unique_ptr<CXFA_FMToken> CXFA_FMLexer::NextToken() {
  if (m_lexer_error)
    return nullptr;

  m_token = pdfium::MakeUnique<CXFA_FMToken>(m_current_line);
  while (m_cursor <= m_end && *m_cursor) {
    if (!IsFormCalcCharacter(*m_cursor)) {
      RaiseError();
      return nullptr;
    }

    switch (*m_cursor) {
      case '\n':
        ++m_current_line;
        m_token->m_line_num = m_current_line;
        ++m_cursor;
        break;
      case '\r':
        ++m_cursor;
        break;
      case ';':
        AdvanceForComment();
        break;
      case '"':
        m_token->m_type = TOKstring;
        AdvanceForString();
        return std::move(m_token);
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        m_token->m_type = TOKnumber;
        AdvanceForNumber();
        return std::move(m_token);
      case '=':
        ++m_cursor;
        if (m_cursor > m_end) {
          m_token->m_type = TOKassign;
          return std::move(m_token);
        }

        if (!IsFormCalcCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }
        if (*m_cursor == '=') {
          m_token->m_type = TOKeq;
          ++m_cursor;
        } else {
          m_token->m_type = TOKassign;
        }
        return std::move(m_token);
      case '<':
        ++m_cursor;
        if (m_cursor > m_end) {
          m_token->m_type = TOKlt;
          return std::move(m_token);
        }

        if (!IsFormCalcCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }
        if (*m_cursor == '=') {
          m_token->m_type = TOKle;
          ++m_cursor;
        } else if (*m_cursor == '>') {
          m_token->m_type = TOKne;
          ++m_cursor;
        } else {
          m_token->m_type = TOKlt;
        }
        return std::move(m_token);
      case '>':
        ++m_cursor;
        if (m_cursor > m_end) {
          m_token->m_type = TOKgt;
          return std::move(m_token);
        }

        if (!IsFormCalcCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }
        if (*m_cursor == '=') {
          m_token->m_type = TOKge;
          ++m_cursor;
        } else {
          m_token->m_type = TOKgt;
        }
        return std::move(m_token);
      case ',':
        m_token->m_type = TOKcomma;
        ++m_cursor;
        return std::move(m_token);
      case '(':
        m_token->m_type = TOKlparen;
        ++m_cursor;
        return std::move(m_token);
      case ')':
        m_token->m_type = TOKrparen;
        ++m_cursor;
        return std::move(m_token);
      case '[':
        m_token->m_type = TOKlbracket;
        ++m_cursor;
        return std::move(m_token);
      case ']':
        m_token->m_type = TOKrbracket;
        ++m_cursor;
        return std::move(m_token);
      case '&':
        ++m_cursor;
        m_token->m_type = TOKand;
        return std::move(m_token);
      case '|':
        ++m_cursor;
        m_token->m_type = TOKor;
        return std::move(m_token);
      case '+':
        ++m_cursor;
        m_token->m_type = TOKplus;
        return std::move(m_token);
      case '-':
        ++m_cursor;
        m_token->m_type = TOKminus;
        return std::move(m_token);
      case '*':
        ++m_cursor;
        m_token->m_type = TOKmul;
        return std::move(m_token);
      case '/': {
        ++m_cursor;
        if (m_cursor > m_end) {
          m_token->m_type = TOKdiv;
          return std::move(m_token);
        }

        if (!IsFormCalcCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }
        if (*m_cursor != '/') {
          m_token->m_type = TOKdiv;
          return std::move(m_token);
        }
        AdvanceForComment();
        break;
      }
      case '.':
        ++m_cursor;
        if (m_cursor > m_end) {
          m_token->m_type = TOKdot;
          return std::move(m_token);
        }

        if (!IsFormCalcCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }

        if (*m_cursor == '.') {
          m_token->m_type = TOKdotdot;
          ++m_cursor;
        } else if (*m_cursor == '*') {
          m_token->m_type = TOKdotstar;
          ++m_cursor;
        } else if (*m_cursor == '#') {
          m_token->m_type = TOKdotscream;
          ++m_cursor;
        } else if (*m_cursor <= '9' && *m_cursor >= '0') {
          m_token->m_type = TOKnumber;
          --m_cursor;
          AdvanceForNumber();
        } else {
          m_token->m_type = TOKdot;
        }
        return std::move(m_token);
      default:
        if (IsWhitespaceCharacter(*m_cursor)) {
          ++m_cursor;
          break;
        }
        if (!IsInitialIdentifierCharacter(*m_cursor)) {
          RaiseError();
          return nullptr;
        }
        AdvanceForIdentifier();
        return std::move(m_token);
    }
  }

  // If there isn't currently a token type then mark it EOF.
  if (m_token->m_type == TOKreserver)
    m_token->m_type = TOKeof;
  return std::move(m_token);
}

void CXFA_FMLexer::AdvanceForNumber() {
  // This will set end to the character after the end of the number.
  wchar_t* end = nullptr;
  if (m_cursor)
    wcstod(const_cast<wchar_t*>(m_cursor), &end);
  if (!end || FXSYS_iswalpha(*end)) {
    RaiseError();
    return;
  }

  m_token->m_string =
      WideStringView(m_cursor, static_cast<size_t>(end - m_cursor));
  m_cursor = end;
}

void CXFA_FMLexer::AdvanceForString() {
  const wchar_t* start = m_cursor;
  ++m_cursor;
  while (m_cursor <= m_end && *m_cursor) {
    if (!IsFormCalcCharacter(*m_cursor))
      break;

    if (*m_cursor == '"') {
      // Check for escaped "s, i.e. "".
      ++m_cursor;
      // If the end of the input has been reached it was not escaped.
      if (m_cursor > m_end) {
        m_token->m_string =
            WideStringView(start, static_cast<size_t>(m_cursor - start));
        return;
      }
      // If the next character is not a " then the end of the string has been
      // found.
      if (*m_cursor != '"') {
        if (!IsFormCalcCharacter(*m_cursor)) {
          break;
        }
        m_token->m_string = WideStringView(start, (m_cursor - start));
        return;
      }
    }
    ++m_cursor;
  }

  // Didn't find the end of the string.
  RaiseError();
}

void CXFA_FMLexer::AdvanceForIdentifier() {
  const wchar_t* start = m_cursor;
  ++m_cursor;
  while (m_cursor <= m_end && *m_cursor) {
    if (!IsFormCalcCharacter(*m_cursor)) {
      RaiseError();
      return;
    }

    if (!IsIdentifierCharacter(*m_cursor)) {
      break;
    }
    ++m_cursor;
  }
  m_token->m_string =
      WideStringView(start, static_cast<size_t>(m_cursor - start));
  m_token->m_type = TokenizeIdentifier(m_token->m_string);
}

void CXFA_FMLexer::AdvanceForComment() {
  m_cursor++;
  while (m_cursor <= m_end && *m_cursor) {
    if (!IsFormCalcCharacter(*m_cursor)) {
      RaiseError();
      return;
    }

    if (*m_cursor == L'\r') {
      ++m_cursor;
      return;
    }
    if (*m_cursor == L'\n') {
      ++m_current_line;
      ++m_cursor;
      return;
    }
    ++m_cursor;
  }
}
