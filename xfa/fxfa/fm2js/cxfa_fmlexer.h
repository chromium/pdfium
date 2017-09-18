// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMLEXER_H_
#define XFA_FXFA_FM2JS_CXFA_FMLEXER_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_string.h"

enum XFA_FM_TOKEN {
  TOKand,
  TOKlparen,
  TOKrparen,
  TOKmul,
  TOKplus,
  TOKcomma,
  TOKminus,
  TOKdot,
  TOKdiv,
  TOKlt,
  TOKassign,
  TOKgt,
  TOKlbracket,
  TOKrbracket,
  TOKor,
  TOKdotscream,
  TOKdotstar,
  TOKdotdot,
  TOKle,
  TOKne,
  TOKeq,
  TOKge,
  TOKdo,
  TOKkseq,
  TOKksge,
  TOKksgt,
  TOKif,
  TOKin,
  TOKksle,
  TOKkslt,
  TOKksne,
  TOKksor,
  TOKnull,
  TOKbreak,
  TOKksand,
  TOKend,
  TOKeof,
  TOKfor,
  TOKnan,
  TOKksnot,
  TOKvar,
  TOKthen,
  TOKelse,
  TOKexit,
  TOKdownto,
  TOKreturn,
  TOKinfinity,
  TOKendwhile,
  TOKforeach,
  TOKendfunc,
  TOKelseif,
  TOKwhile,
  TOKendfor,
  TOKthrow,
  TOKstep,
  TOKupto,
  TOKcontinue,
  TOKfunc,
  TOKendif,
  TOKstar,
  TOKidentifier,
  TOKunderscore,
  TOKdollar,
  TOKexclamation,
  TOKcall,
  TOKstring,
  TOKnumber,
  TOKreserver
};

struct XFA_FMKeyword {
  XFA_FM_TOKEN m_type;
  uint32_t m_hash;
  const wchar_t* m_keyword;
};

class CXFA_FMToken {
 public:
  CXFA_FMToken();
  explicit CXFA_FMToken(uint32_t line_num);
  ~CXFA_FMToken();

  WideString ToDebugString() const;

  WideStringView m_string;
  XFA_FM_TOKEN m_type;
  uint32_t m_line_num;
};

class CXFA_FMLexer {
 public:
  explicit CXFA_FMLexer(const WideStringView& wsFormcalc);
  ~CXFA_FMLexer();

  std::unique_ptr<CXFA_FMToken> NextToken();

  void SetCurrentLine(uint32_t line) { m_current_line = line; }
  const wchar_t* GetPos() { return m_cursor; }
  void SetPos(const wchar_t* pos) { m_cursor = pos; }

 private:
  void AdvanceForNumber();
  void AdvanceForString();
  void AdvanceForIdentifier();
  void AdvanceForComment();

  void RaiseError() {
    m_token.reset();
    m_lexer_error = true;
  }

  const wchar_t* m_cursor;
  const wchar_t* const m_end;
  uint32_t m_current_line;
  std::unique_ptr<CXFA_FMToken> m_token;
  bool m_lexer_error;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMLEXER_H_
