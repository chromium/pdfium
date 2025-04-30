// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/formcalc/cxfa_fmlexer.h"

#include <algorithm>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_extension.h"

namespace {

bool IsFormCalcCharacter(wchar_t c) {
  return (c >= 0x09 && c <= 0x0D) || (c >= 0x20 && c <= 0xd7FF) ||
         (c >= 0xE000 && c <= 0xFFFD);
}

bool IsIdentifierCharacter(wchar_t c) {
  return FXSYS_iswalnum(c) || c == 0x005F ||  // '_'
         c == 0x0024;                         // '$'
}

bool IsInitialIdentifierCharacter(wchar_t c) {
  return FXSYS_iswalpha(c) || c == 0x005F ||  // '_'
         c == 0x0024 ||                       // '$'
         c == 0x0021;                         // '!'
}

bool IsWhitespaceCharacter(wchar_t c) {
  return c == 0x0009 ||  // Horizontal tab
         c == 0x000B ||  // Vertical tab
         c == 0x000C ||  // Form feed
         c == 0x0020;    // Space
}

struct XFA_FMKeyword {
  XFA_FM_TOKEN type_;
  const char* keyword_;  // Raw, POD struct.
};

const XFA_FMKeyword kKeyWords[] = {
    {TOKdo, "do"},
    {TOKkseq, "eq"},
    {TOKksge, "ge"},
    {TOKksgt, "gt"},
    {TOKif, "if"},
    {TOKin, "in"},
    {TOKksle, "le"},
    {TOKkslt, "lt"},
    {TOKksne, "ne"},
    {TOKksor, "or"},
    {TOKnull, "null"},
    {TOKbreak, "break"},
    {TOKksand, "and"},
    {TOKend, "end"},
    {TOKeof, "eof"},
    {TOKfor, "for"},
    {TOKnan, "nan"},
    {TOKksnot, "not"},
    {TOKvar, "var"},
    {TOKthen, "then"},
    {TOKelse, "else"},
    {TOKexit, "exit"},
    {TOKdownto, "downto"},
    {TOKreturn, "return"},
    {TOKinfinity, "infinity"},
    {TOKendwhile, "endwhile"},
    {TOKforeach, "foreach"},
    {TOKendfunc, "endfunc"},
    {TOKelseif, "elseif"},
    {TOKwhile, "while"},
    {TOKendfor, "endfor"},
    {TOKthrow, "throw"},
    {TOKstep, "step"},
    {TOKupto, "upto"},
    {TOKcontinue, "continue"},
    {TOKfunc, "func"},
    {TOKendif, "endif"},
};

XFA_FM_TOKEN TokenizeIdentifier(WideStringView str) {
  const XFA_FMKeyword* result =
      std::ranges::find_if(kKeyWords, [str](const XFA_FMKeyword& iter) {
        return str.EqualsASCII(iter.keyword_);
      });
  if (result != std::end(kKeyWords) && str.EqualsASCII(result->keyword_)) {
    return result->type_;
  }
  return TOKidentifier;
}

}  // namespace

CXFA_FMLexer::Token::Token() = default;

CXFA_FMLexer::Token::Token(XFA_FM_TOKEN token) : type_(token) {}

CXFA_FMLexer::Token::Token(XFA_FM_TOKEN token, WideStringView str)
    : type_(token), string_(str) {}

CXFA_FMLexer::Token::Token(const Token& that) = default;

CXFA_FMLexer::Token::~Token() = default;

CXFA_FMLexer::CXFA_FMLexer(WideStringView wsFormCalc)
    : input_(wsFormCalc.span()) {}

CXFA_FMLexer::~CXFA_FMLexer() = default;

CXFA_FMLexer::Token CXFA_FMLexer::NextToken() {
  if (lexer_error_) {
    return Token();
  }

  while (!IsComplete() && input_[cursor_]) {
    if (!IsFormCalcCharacter(input_[cursor_])) {
      RaiseError();
      return Token();
    }

    switch (input_[cursor_]) {
      case '\n':
        ++cursor_;
        break;
      case '\r':
        ++cursor_;
        break;
      case ';':
        AdvanceForComment();
        break;
      case '"':
        return AdvanceForString();
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
        return AdvanceForNumber();
      case '=':
        ++cursor_;
        if (cursor_ >= input_.size()) {
          return Token(TOKassign);
        }

        if (!IsFormCalcCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }
        if (input_[cursor_] == '=') {
          ++cursor_;
          return Token(TOKeq);
        }
        return Token(TOKassign);
      case '<':
        ++cursor_;
        if (cursor_ >= input_.size()) {
          return Token(TOKlt);
        }

        if (!IsFormCalcCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }
        if (input_[cursor_] == '=') {
          ++cursor_;
          return Token(TOKle);
        }
        if (input_[cursor_] == '>') {
          ++cursor_;
          return Token(TOKne);
        }
        return Token(TOKlt);
      case '>':
        ++cursor_;
        if (cursor_ >= input_.size()) {
          return Token(TOKgt);
        }

        if (!IsFormCalcCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }
        if (input_[cursor_] == '=') {
          ++cursor_;
          return Token(TOKge);
        }
        return Token(TOKgt);
      case ',':
        ++cursor_;
        return Token(TOKcomma);
      case '(':
        ++cursor_;
        return Token(TOKlparen);
      case ')':
        ++cursor_;
        return Token(TOKrparen);
      case '[':
        ++cursor_;
        return Token(TOKlbracket);
      case ']':
        ++cursor_;
        return Token(TOKrbracket);
      case '&':
        ++cursor_;
        return Token(TOKand);
      case '|':
        ++cursor_;
        return Token(TOKor);
      case '+':
        ++cursor_;
        return Token(TOKplus);
      case '-':
        ++cursor_;
        return Token(TOKminus);
      case '*':
        ++cursor_;
        return Token(TOKmul);
      case '/': {
        ++cursor_;
        if (cursor_ >= input_.size()) {
          return Token(TOKdiv);
        }

        if (!IsFormCalcCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }
        if (input_[cursor_] != '/') {
          return Token(TOKdiv);
        }

        AdvanceForComment();
        break;
      }
      case '.':
        ++cursor_;
        if (cursor_ >= input_.size()) {
          return Token(TOKdot);
        }

        if (!IsFormCalcCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }

        if (input_[cursor_] == '.') {
          ++cursor_;
          return Token(TOKdotdot);
        }
        if (input_[cursor_] == '*') {
          ++cursor_;
          return Token(TOKdotstar);
        }
        if (input_[cursor_] == '#') {
          ++cursor_;
          return Token(TOKdotscream);
        }
        if (FXSYS_IsDecimalDigit(input_[cursor_])) {
          --cursor_;
          return AdvanceForNumber();
        }
        return Token(TOKdot);
      default:
        if (IsWhitespaceCharacter(input_[cursor_])) {
          ++cursor_;
          break;
        }
        if (!IsInitialIdentifierCharacter(input_[cursor_])) {
          RaiseError();
          return Token();
        }
        return AdvanceForIdentifier();
    }
  }
  return Token(TOKeof);
}

CXFA_FMLexer::Token CXFA_FMLexer::AdvanceForNumber() {
  // This will set end to the character after the end of the number.
  size_t used_length = 0;
  if (cursor_ < input_.size()) {
    FXSYS_wcstof(WideStringView(input_.subspan(cursor_)), &used_length);
  }
  size_t end = cursor_ + used_length;
  if (used_length == 0 ||
      (end < input_.size() && FXSYS_iswalpha(input_[end]))) {
    RaiseError();
    return Token();
  }
  WideStringView str(input_.subspan(cursor_, end - cursor_));
  cursor_ = end;
  return Token(TOKnumber, str);
}

CXFA_FMLexer::Token CXFA_FMLexer::AdvanceForString() {
  size_t start = cursor_;
  ++cursor_;
  while (!IsComplete() && input_[cursor_]) {
    if (!IsFormCalcCharacter(input_[cursor_])) {
      break;
    }

    if (input_[cursor_] == '"') {
      // Check for escaped "s, i.e. "".
      ++cursor_;
      // If the end of the input has been reached it was not escaped.
      if (cursor_ >= input_.size()) {
        return Token(TOKstring,
                     WideStringView(input_.subspan(start, cursor_ - start)));
      }
      // If the next character is not a " then the end of the string has been
      // found.
      if (input_[cursor_] != '"') {
        if (!IsFormCalcCharacter(input_[cursor_])) {
          break;
        }

        return Token(TOKstring,
                     WideStringView(input_.subspan(start, cursor_ - start)));
      }
    }
    ++cursor_;
  }

  // Didn't find the end of the string.
  RaiseError();
  return Token();
}

CXFA_FMLexer::Token CXFA_FMLexer::AdvanceForIdentifier() {
  size_t start = cursor_;
  ++cursor_;
  while (!IsComplete() && input_[cursor_]) {
    if (!IsFormCalcCharacter(input_[cursor_])) {
      RaiseError();
      return Token();
    }
    if (!IsIdentifierCharacter(input_[cursor_])) {
      break;
    }

    ++cursor_;
  }

  WideStringView str(input_.subspan(start, cursor_ - start));
  return Token(TokenizeIdentifier(str), str);
}

void CXFA_FMLexer::AdvanceForComment() {
  ++cursor_;
  while (!IsComplete() && input_[cursor_]) {
    if (!IsFormCalcCharacter(input_[cursor_])) {
      RaiseError();
      return;
    }
    if (input_[cursor_] == L'\r') {
      ++cursor_;
      return;
    }
    if (input_[cursor_] == L'\n') {
      ++cursor_;
      return;
    }
    ++cursor_;
  }
}
