// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FORMCALC_CXFA_FMLEXER_H_
#define XFA_FXFA_FORMCALC_CXFA_FMLEXER_H_

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/widestring.h"
#include "v8/include/cppgc/macros.h"

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

class CXFA_FMLexer {
  CPPGC_STACK_ALLOCATED();  // Raw pointers allowed.

 public:
  class Token {
   public:
    Token();
    explicit Token(XFA_FM_TOKEN token);
    Token(XFA_FM_TOKEN token, WideStringView str);
    Token(const Token& that);
    ~Token();

    XFA_FM_TOKEN GetType() const { return m_type; }
    WideStringView GetString() const { return m_string; }
#ifndef NDEBUG
    WideString ToDebugString() const;
#endif  // NDEBUG

   private:
    XFA_FM_TOKEN m_type = TOKreserver;
    WideStringView m_string;
  };

  explicit CXFA_FMLexer(WideStringView wsFormcalc);
  ~CXFA_FMLexer();

  Token NextToken();
  bool IsComplete() const { return m_nCursor >= m_spInput.size(); }

 private:
  Token AdvanceForNumber();
  Token AdvanceForString();
  Token AdvanceForIdentifier();
  void AdvanceForComment();

  void RaiseError() { m_bLexerError = true; }

  pdfium::raw_span<const wchar_t> m_spInput;
  size_t m_nCursor = 0;
  bool m_bLexerError = false;
};

#endif  // XFA_FXFA_FORMCALC_CXFA_FMLEXER_H_
