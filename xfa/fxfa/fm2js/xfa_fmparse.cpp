// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_fmparse.h"

#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"

CXFA_FMParse::CXFA_FMParse() : m_pToken(nullptr), m_pErrorInfo(0) {}

CXFA_FMParse::~CXFA_FMParse() {}

int32_t CXFA_FMParse::Init(const CFX_WideStringC& wsFormcalc,
                           CXFA_FMErrorInfo* pErrorInfo) {
  m_pErrorInfo = pErrorInfo;
  m_lexer = pdfium::MakeUnique<CXFA_FMLexer>(wsFormcalc, m_pErrorInfo);
  return 0;
}

void CXFA_FMParse::NextToken() {
  m_pToken = m_lexer->NextToken();
  while (m_pToken->m_type == TOKreserver) {
    if (m_lexer->HasError()) {
      break;
    }
    m_pToken = m_lexer->NextToken();
  }
}

void CXFA_FMParse::Check(XFA_FM_TOKEN op) {
  if (m_pToken->m_type != op) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken, XFA_FM_KeywordToString(op),
          ws_TempString.c_str());
  }
  NextToken();
}

void CXFA_FMParse::Error(uint32_t lineNum, const FX_WCHAR* msg, ...) {
  m_pErrorInfo->linenum = lineNum;
  va_list ap;
  va_start(ap, msg);
  m_pErrorInfo->message.FormatV(msg, ap);
  va_end(ap);
}

std::vector<std::unique_ptr<CXFA_FMExpression>>
CXFA_FMParse::ParseTopExpression() {
  std::unique_ptr<CXFA_FMExpression> expr;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  while (1) {
    if (m_pToken->m_type == TOKeof || m_pToken->m_type == TOKendfunc ||
        m_pToken->m_type == TOKendif || m_pToken->m_type == TOKelseif ||
        m_pToken->m_type == TOKelse || m_pToken->m_type == TOKreserver) {
      return expressions;
    }

    if (m_pToken->m_type == TOKfunc) {
      expr = ParseFunction();
      if (expr) {
        expressions.push_back(std::move(expr));
      } else {
        break;
      }
    } else {
      expr = ParseExpression();
      if (expr) {
        expressions.push_back(std::move(expr));
      } else {
        break;
      }
    }
  }
  return expressions;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseFunction() {
  std::unique_ptr<CXFA_FMExpression> expr;
  CFX_WideStringC ident;
  std::unique_ptr<CFX_WideStringCArray> pArguments;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
          ws_TempString.c_str());
  } else {
    ident = m_pToken->m_wstring;
    NextToken();
  }
  Check(TOKlparen);
  if (m_pToken->m_type == TOKrparen) {
    NextToken();
  } else {
    pArguments = pdfium::MakeUnique<CFX_WideStringCArray>();
    CFX_WideStringC p;
    while (1) {
      if (m_pToken->m_type == TOKidentifier) {
        p = m_pToken->m_wstring;
        pArguments->Add(p);
        NextToken();
        if (m_pToken->m_type == TOKcomma) {
          NextToken();
          continue;
        } else if (m_pToken->m_type == TOKrparen) {
          NextToken();
          break;
        } else {
          Check(TOKrparen);
          break;
        }
      } else {
        CFX_WideString ws_TempString(m_pToken->m_wstring);
        Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
              ws_TempString.c_str());
        NextToken();
        break;
      }
    }
  }
  Check(TOKdo);
  if (m_pToken->m_type == TOKendfunc) {
    NextToken();
  } else {
    expressions = ParseTopExpression();
    Check(TOKendfunc);
  }
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
        line, false, ident, std::move(pArguments), std::move(expressions));
  } else if (pArguments) {
    pArguments->RemoveAll();
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseExpression() {
  std::unique_ptr<CXFA_FMExpression> expr;
  uint32_t line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKvar:
      expr = ParseVarExpression();
      break;
    case TOKnull:
    case TOKnumber:
    case TOKstring:
    case TOKplus:
    case TOKminus:
    case TOKksnot:
    case TOKidentifier:
    case TOKlparen:
      expr = ParseExpExpression();
      break;
    case TOKif:
      expr = ParseIfExpression();
      break;
    case TOKwhile:
      expr = ParseWhileExpression();
      break;
    case TOKfor:
      expr = ParseForExpression();
      break;
    case TOKforeach:
      expr = ParseForeachExpression();
      break;
    case TOKdo:
      expr = ParseDoExpression();
      break;
    case TOKbreak:
      expr = pdfium::MakeUnique<CXFA_FMBreakExpression>(line);
      NextToken();
      break;
    case TOKcontinue:
      expr = pdfium::MakeUnique<CXFA_FMContinueExpression>(line);
      NextToken();
      break;
    default:
      CFX_WideString ws_TempString(m_pToken->m_wstring);
      Error(m_pToken->m_uLinenum, kFMErrUnexpectedExpression,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseVarExpression() {
  std::unique_ptr<CXFA_FMExpression> expr;
  CFX_WideStringC ident;
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
          ws_TempString.c_str());
  } else {
    ident = m_pToken->m_wstring;
    NextToken();
  }
  if (m_pToken->m_type == TOKassign) {
    NextToken();
    expr = ParseExpExpression();
  }
  if (m_pErrorInfo->message.IsEmpty()) {
    expr =
        pdfium::MakeUnique<CXFA_FMVarExpression>(line, ident, expr.release());
  } else {
    expr.reset();
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseSimpleExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();
  while (m_pToken->m_type == TOKassign) {
    NextToken();
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (m_pErrorInfo->message.IsEmpty()) {
      pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
          line, TOKassign, pExp1.release(), pExp2.release());
    } else {
      pExp1.reset();
    }
  }
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseExpExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseSimpleExpression();
  std::unique_ptr<CXFA_FMExpression> expr;
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMExpExpression>(line, pExp1.release());
  } else {
    expr.reset();
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseLogicalOrExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseLogicalAndExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKor:
      case TOKksor: {
        NextToken();
        std::unique_ptr<CXFA_FMSimpleExpression> e2(
            ParseLogicalAndExpression());
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMLogicalOrExpression>(
              line, TOKor, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      }
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseLogicalAndExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseEqualityExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKand:
      case TOKksand: {
        NextToken();
        std::unique_ptr<CXFA_FMSimpleExpression> e2 = ParseEqualityExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMLogicalAndExpression>(
              line, TOKand, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      }
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseEqualityExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseRelationalExpression();
  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_pToken->m_type) {
      case TOKeq:
      case TOKkseq:
        NextToken();
        e2 = ParseRelationalExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
              line, TOKeq, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKne:
      case TOKksne:
        NextToken();
        e2 = ParseRelationalExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
              line, TOKne, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseRelationalExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseAddtiveExpression();
  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_pToken->m_type) {
      case TOKlt:
      case TOKkslt:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
              line, TOKlt, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKgt:
      case TOKksgt:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
              line, TOKgt, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKle:
      case TOKksle:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
              line, TOKle, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKge:
      case TOKksge:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
              line, TOKge, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseAddtiveExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseMultiplicativeExpression();
  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_pToken->m_type) {
      case TOKplus:
        NextToken();
        e2 = ParseMultiplicativeExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
              line, TOKplus, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKminus:
        NextToken();
        e2 = ParseMultiplicativeExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
              line, TOKminus, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseMultiplicativeExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseUnaryExpression();
  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_pToken->m_type) {
      case TOKmul:
        NextToken();
        e2 = ParseUnaryExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
              line, TOKmul, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      case TOKdiv:
        NextToken();
        e2 = ParseUnaryExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
              line, TOKdiv, e1.release(), e2.release());
        } else {
          e1.reset();
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseUnaryExpression() {
  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKplus:
      NextToken();
      expr = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        expr = pdfium::MakeUnique<CXFA_FMPosExpression>(line, expr.release());
      } else {
        expr.reset();
      }
      break;
    case TOKminus:
      NextToken();
      expr = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        expr = pdfium::MakeUnique<CXFA_FMNegExpression>(line, expr.release());
      } else {
        expr.reset();
      }
      break;
    case TOKksnot:
      NextToken();
      expr = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        expr = pdfium::MakeUnique<CXFA_FMNotExpression>(line, expr.release());
      } else {
        expr.reset();
      }
      break;
    default:
      expr = ParsePrimaryExpression();
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParsePrimaryExpression() {
  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKnumber:
      expr = pdfium::MakeUnique<CXFA_FMNumberExpression>(line,
                                                         m_pToken->m_wstring);
      NextToken();
      break;
    case TOKstring:
      expr = pdfium::MakeUnique<CXFA_FMStringExpression>(line,
                                                         m_pToken->m_wstring);
      NextToken();
      break;
    case TOKidentifier: {
      CFX_WideStringC wsIdentifier(m_pToken->m_wstring);
      NextToken();
      if (m_pToken->m_type == TOKlbracket) {
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (s) {
          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, nullptr, TOKdot, wsIdentifier, s.release());
        }
        NextToken();
      } else {
        expr =
            pdfium::MakeUnique<CXFA_FMIdentifierExpression>(line, wsIdentifier);
      }
    } break;
    case TOKif:
      expr = pdfium::MakeUnique<CXFA_FMIdentifierExpression>(
          line, m_pToken->m_wstring);
      NextToken();
      break;
    case TOKnull:
      expr = pdfium::MakeUnique<CXFA_FMNullExpression>(line);
      NextToken();
      break;
    case TOKlparen:
      expr = ParseParenExpression();
      break;
    default:
      CFX_WideString ws_TempString(m_pToken->m_wstring);
      Error(m_pToken->m_uLinenum, kFMErrUnexpectedExpression,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  expr = ParsePostExpression(std::move(expr));
  if (!(m_pErrorInfo->message.IsEmpty()))
    expr.reset();
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParsePostExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> expr) {
  uint32_t line = m_pToken->m_uLinenum;
  while (1) {
    switch (m_pToken->m_type) {
      case TOKlparen: {
        NextToken();
        std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
        if (m_pToken->m_type != TOKrparen) {
          while (m_pToken->m_type != TOKrparen) {
            if (std::unique_ptr<CXFA_FMSimpleExpression> expr =
                    ParseSimpleExpression())
              expressions.push_back(std::move(expr));
            if (m_pToken->m_type == TOKcomma) {
              NextToken();
            } else if (m_pToken->m_type == TOKeof ||
                       m_pToken->m_type == TOKreserver) {
              break;
            }
          }
          if (m_pToken->m_type != TOKrparen) {
            CFX_WideString ws_TempString(m_pToken->m_wstring);
            Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
                  XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
          }
        }
        if (m_pErrorInfo->message.IsEmpty()) {
          expr = pdfium::MakeUnique<CXFA_FMCallExpression>(
              line, expr.release(), std::move(expressions), false);
          NextToken();
          if (m_pToken->m_type != TOKlbracket)
            continue;

          std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
          if (s) {
            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                line, expr.release(), TOKcall, FX_WSTRC(L""), s.release());
          } else {
            expr.reset();
          }
        } else {
          expr.reset();
        }
      } break;
      case TOKdot:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          uint32_t tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlparen) {
            std::unique_ptr<CXFA_FMSimpleExpression> pExpCall;
            NextToken();
            std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
            if (m_pToken->m_type != TOKrparen) {
              while (m_pToken->m_type != TOKrparen) {
                std::unique_ptr<CXFA_FMSimpleExpression> exp =
                    ParseSimpleExpression();
                expressions.push_back(std::move(exp));
                if (m_pToken->m_type == TOKcomma) {
                  NextToken();
                } else if (m_pToken->m_type == TOKeof ||
                           m_pToken->m_type == TOKreserver) {
                  break;
                }
              }
              if (m_pToken->m_type != TOKrparen) {
                CFX_WideString ws_TempString(m_pToken->m_wstring);
                Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
                      XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
              }
            }
            if (m_pErrorInfo->message.IsEmpty()) {
              std::unique_ptr<CXFA_FMSimpleExpression> pIdentifier =
                  pdfium::MakeUnique<CXFA_FMIdentifierExpression>(tempLine,
                                                                  tempStr);
              pExpCall = pdfium::MakeUnique<CXFA_FMCallExpression>(
                  line, pIdentifier.release(), std::move(expressions), true);
              expr = pdfium::MakeUnique<CXFA_FMMethodCallExpression>(
                  line, expr.release(), pExpCall.release());
              NextToken();
              if (m_pToken->m_type != TOKlbracket)
                continue;

              std::unique_ptr<CXFA_FMSimpleExpression> s =
                  ParseIndexExpression();
              if (s) {
                expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                    line, expr.release(), TOKcall, FX_WSTRC(L""), s.release());
              } else {
                expr.reset();
              }
            } else {
              expr.reset();
            }
          } else if (m_pToken->m_type == TOKlbracket) {
            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty()))
              return nullptr;

            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                tempLine, expr.release(), TOKdot, tempStr, s.release());
          } else {
            std::unique_ptr<CXFA_FMSimpleExpression> s =
                pdfium::MakeUnique<CXFA_FMIndexExpression>(
                    tempLine, ACCESSOR_NO_INDEX, nullptr, false);
            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                line, expr.release(), TOKdot, tempStr, s.release());
            continue;
          }
        } else {
          CFX_WideString ws_TempString(m_pToken->m_wstring);
          Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
                ws_TempString.c_str());
          return expr;
        }
        break;
      case TOKdotdot:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          uint32_t tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlbracket) {
            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty())) {
              return nullptr;
            }
            expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
                tempLine, expr.release(), TOKdotdot, tempStr, s.release());
          } else {
            std::unique_ptr<CXFA_FMSimpleExpression> s =
                pdfium::MakeUnique<CXFA_FMIndexExpression>(
                    tempLine, ACCESSOR_NO_INDEX, nullptr, false);
            expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
                line, expr.release(), TOKdotdot, tempStr, s.release());
            continue;
          }
        } else {
          CFX_WideString ws_TempString(m_pToken->m_wstring);
          Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
                ws_TempString.c_str());
          return expr;
        }
        break;
      case TOKdotscream:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          uint32_t tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlbracket) {
            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty()))
              return nullptr;

            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                tempLine, expr.release(), TOKdotscream, tempStr, s.release());
          } else {
            std::unique_ptr<CXFA_FMSimpleExpression> s =
                pdfium::MakeUnique<CXFA_FMIndexExpression>(
                    tempLine, ACCESSOR_NO_INDEX, nullptr, false);
            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                line, expr.release(), TOKdotscream, tempStr, s.release());
            continue;
          }
        } else {
          CFX_WideString ws_TempString(m_pToken->m_wstring);
          Error(m_pToken->m_uLinenum, kFMErrExpectedIdentifier,
                ws_TempString.c_str());
          return expr;
        }
        break;
      case TOKdotstar: {
        std::unique_ptr<CXFA_FMSimpleExpression> s =
            pdfium::MakeUnique<CXFA_FMIndexExpression>(line, ACCESSOR_NO_INDEX,
                                                       nullptr, false);
        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, expr.release(), TOKdotstar, FX_WSTRC(L"*"), s.release());
      } break;
      default:
        return expr;
    }
    NextToken();
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseIndexExpression() {
  std::unique_ptr<CXFA_FMSimpleExpression> pExp;
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  std::unique_ptr<CXFA_FMSimpleExpression> s;
  XFA_FM_AccessorIndex accessorIndex = ACCESSOR_NO_RELATIVEINDEX;
  if (m_pToken->m_type == TOKmul) {
    pExp = pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                      s.release(), true);
    NextToken();
    if (m_pToken->m_type != TOKrbracket) {
      CFX_WideString ws_TempString(m_pToken->m_wstring);
      Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
            XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
      pExp.reset();
    }
    return pExp;
  }
  if (m_pToken->m_type == TOKplus) {
    accessorIndex = ACCESSOR_POSITIVE_INDEX;
    NextToken();
  } else if (m_pToken->m_type == TOKminus) {
    accessorIndex = ACCESSOR_NEGATIVE_INDEX;
    NextToken();
  }
  s = ParseSimpleExpression();
  if (m_pToken->m_type != TOKrbracket) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
          XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
  } else {
    pExp = pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                      s.release(), false);
  }
  return pExp;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseParenExpression() {
  Check(TOKlparen);

  if (m_pToken->m_type == TOKrparen) {
    Error(m_pToken->m_uLinenum, kFMErrExpectedNonEmptyExpression);
    NextToken();
    return nullptr;
  }

  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();

  while (m_pToken->m_type == TOKassign) {
    NextToken();
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (m_pErrorInfo->message.IsEmpty()) {
      pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
          line, TOKassign, pExp1.release(), pExp2.release());
    } else {
      pExp1.reset();
    }
  }
  Check(TOKrparen);
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseBlockExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  std::unique_ptr<CXFA_FMExpression> expr;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;

  while (1) {
    switch (m_pToken->m_type) {
      case TOKeof:
      case TOKendif:
      case TOKelseif:
      case TOKelse:
      case TOKendwhile:
      case TOKendfor:
      case TOKend:
      case TOKendfunc:
      case TOKreserver:
        break;
      case TOKfunc:
        expr = ParseFunction();
        if (expr) {
          expressions.push_back(std::move(expr));
        }
        continue;
      default:
        expr = ParseExpression();
        if (expr) {
          expressions.push_back(std::move(expr));
        }
        continue;
    }
    break;
  }
  std::unique_ptr<CXFA_FMBlockExpression> pExp;
  if (m_pErrorInfo->message.IsEmpty()) {
    pExp = pdfium::MakeUnique<CXFA_FMBlockExpression>(line,
                                                      std::move(expressions));
  }
  return pExp;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseIfExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  const FX_WCHAR* pStartPos = m_lexer->SavePos();
  NextToken();
  Check(TOKlparen);
  std::unique_ptr<CXFA_FMSimpleExpression> pExpression;
  while (m_pToken->m_type != TOKrparen) {
    pExpression = ParseSimpleExpression();
    if (m_pToken->m_type != TOKcomma)
      break;
    NextToken();
  }
  Check(TOKrparen);
  if (m_pToken->m_type != TOKthen) {
    m_lexer->SetCurrentLine(line);
    m_pToken = new CXFA_FMToken(line);
    m_pToken->m_type = TOKidentifier;
    m_pToken->m_wstring = FX_WSTRC(L"if");
    m_lexer->SetToken(m_pToken);
    m_lexer->RestorePos(pStartPos);
    return ParseExpExpression();
  }
  Check(TOKthen);
  std::unique_ptr<CXFA_FMExpression> pIfExpression = ParseBlockExpression();
  std::unique_ptr<CXFA_FMExpression> pElseExpression;
  switch (m_pToken->m_type) {
    case TOKeof:
    case TOKendif:
      Check(TOKendif);
      break;
    case TOKif:
      pElseExpression = ParseIfExpression();
      Check(TOKendif);
      break;
    case TOKelseif:
      pElseExpression = ParseIfExpression();
      break;
    case TOKelse:
      NextToken();
      pElseExpression = ParseBlockExpression();
      Check(TOKendif);
      break;
    default:
      CFX_WideString ws_TempString(m_pToken->m_wstring);
      Error(m_pToken->m_uLinenum, kFMErrExpectedEndIf, ws_TempString.c_str());
      NextToken();
      break;
  }
  std::unique_ptr<CXFA_FMIfExpression> pExp;
  if (m_pErrorInfo->message.IsEmpty()) {
    pExp = pdfium::MakeUnique<CXFA_FMIfExpression>(line, pExpression.release(),
                                                   pIfExpression.release(),
                                                   pElseExpression.release());
  }
  return pExp;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseWhileExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  std::unique_ptr<CXFA_FMSimpleExpression> pCondition = ParseParenExpression();
  Check(TOKdo);
  std::unique_ptr<CXFA_FMExpression> pExpression = ParseBlockExpression();
  Check(TOKendwhile);
  std::unique_ptr<CXFA_FMExpression> expr;
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMWhileExpression>(
        line, pCondition.release(), pExpression.release());
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseSubassignmentInForExpression() {
  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  switch (m_pToken->m_type) {
    case TOKidentifier:
      expr = ParseSimpleExpression();
      break;
    default:
      CFX_WideString ws_TempString(m_pToken->m_wstring);
      Error(m_pToken->m_uLinenum, kFMErrUnexpectedExpression,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseForExpression() {
  CFX_WideStringC wsVariant;
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  wsVariant = m_pToken->m_wstring;
  NextToken();
  std::unique_ptr<CXFA_FMSimpleExpression> pAssignment;
  if (m_pToken->m_type == TOKassign) {
    NextToken();
    pAssignment = ParseSimpleExpression();
  } else {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  int32_t iDirection = 0;
  if (m_pToken->m_type == TOKupto) {
    iDirection = 1;
  } else if (m_pToken->m_type == TOKdownto) {
    iDirection = -1;
  } else {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken, L"upto or downto",
          ws_TempString.c_str());
  }
  NextToken();
  std::unique_ptr<CXFA_FMSimpleExpression> pAccessor = ParseSimpleExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> pStep;
  if (m_pToken->m_type == TOKstep) {
    NextToken();
    pStep = ParseSimpleExpression();
  }
  Check(TOKdo);
  std::unique_ptr<CXFA_FMExpression> pList = ParseBlockExpression();
  Check(TOKendfor);
  std::unique_ptr<CXFA_FMExpression> expr;
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMForExpression>(
        line, wsVariant, pAssignment.release(), pAccessor.release(), iDirection,
        pStep.release(), pList.release());
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseForeachExpression() {
  std::unique_ptr<CXFA_FMExpression> expr;
  CFX_WideStringC wsIdentifier;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> pAccessors;
  std::unique_ptr<CXFA_FMExpression> pList;
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrExpectedToken,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  wsIdentifier = m_pToken->m_wstring;
  NextToken();
  Check(TOKin);
  Check(TOKlparen);
  if (m_pToken->m_type == TOKrparen) {
    CFX_WideString ws_TempString(m_pToken->m_wstring);
    Error(m_pToken->m_uLinenum, kFMErrUnexpectedExpression,
          ws_TempString.c_str());
    NextToken();
  } else {
    while (m_pToken->m_type != TOKrparen) {
      std::unique_ptr<CXFA_FMSimpleExpression> s = ParseSimpleExpression();
      if (s)
        pAccessors.push_back(std::move(s));
      if (m_pToken->m_type != TOKcomma)
        break;
      NextToken();
    }
    Check(TOKrparen);
  }
  Check(TOKdo);
  pList = ParseBlockExpression();
  Check(TOKendfor);
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMForeachExpression>(
        line, wsIdentifier, std::move(pAccessors), pList.release());
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseDoExpression() {
  uint32_t line = m_pToken->m_uLinenum;
  NextToken();
  std::unique_ptr<CXFA_FMExpression> expr = ParseBlockExpression();
  Check(TOKend);
  if (m_pErrorInfo->message.IsEmpty()) {
    expr = pdfium::MakeUnique<CXFA_FMDoExpression>(line, expr.release());
  } else {
    expr.reset();
  }
  return expr;
}
