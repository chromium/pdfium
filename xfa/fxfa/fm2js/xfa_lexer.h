// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_XFA_LEXER_H_
#define XFA_FXFA_FM2JS_XFA_LEXER_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/fm2js/xfa_error.h"

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
  uint32_t m_uHash;
  const wchar_t* m_keyword;
};

const wchar_t* XFA_FM_KeywordToString(XFA_FM_TOKEN op);

class CXFA_FMToken {
 public:
  CXFA_FMToken();
  explicit CXFA_FMToken(uint32_t uLineNum);

  CFX_WideStringC m_wstring;
  XFA_FM_TOKEN m_type;
  uint32_t m_uLinenum;
};

class CXFA_FMLexer {
 public:
  CXFA_FMLexer(const CFX_WideStringC& wsFormcalc, CXFA_FMErrorInfo* pErrorInfo);
  ~CXFA_FMLexer();

  CXFA_FMToken* NextToken();
  uint32_t Number(CXFA_FMToken* t, const wchar_t* p, const wchar_t*& pEnd);
  uint32_t String(CXFA_FMToken* t, const wchar_t* p, const wchar_t*& pEnd);
  uint32_t Identifiers(CXFA_FMToken* t, const wchar_t* p, const wchar_t*& pEnd);
  void Comment(const wchar_t* p, const wchar_t*& pEnd);
  XFA_FM_TOKEN IsKeyword(const CFX_WideStringC& p);
  void SetCurrentLine(uint32_t line) { m_uCurrentLine = line; }
  void SetToken(CXFA_FMToken* pToken) {
    if (m_pToken.get() != pToken)
      m_pToken.reset(pToken);
  }
  const wchar_t* SavePos() { return m_ptr; }
  void RestorePos(const wchar_t* pPos) { m_ptr = pPos; }
  void Error(const wchar_t* msg, ...);
  bool HasError() const;

 private:
  CXFA_FMToken* Scan();

  const wchar_t* m_ptr;
  uint32_t m_uCurrentLine;
  std::unique_ptr<CXFA_FMToken> m_pToken;
  CXFA_FMErrorInfo* m_pErrorInfo;
};

#endif  // XFA_FXFA_FM2JS_XFA_LEXER_H_
