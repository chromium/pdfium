// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmparser.h"

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/autorestorer.h"
#include "third_party/base/ptr_util.h"

namespace {

const unsigned int kMaxAssignmentChainLength = 12;
const unsigned int kMaxParseDepth = 1250;

}  // namespace

CXFA_FMParser::CXFA_FMParser(const WideStringView& wsFormcalc)
    : m_error(false), m_parse_depth(0), m_max_parse_depth(kMaxParseDepth) {
  m_lexer = pdfium::MakeUnique<CXFA_FMLexer>(wsFormcalc);
  m_token = m_lexer->NextToken();
}

CXFA_FMParser::~CXFA_FMParser() {}

std::unique_ptr<CXFA_FMFunctionDefinition> CXFA_FMParser::Parse() {
  auto expressions = ParseTopExpression();
  if (HasError())
    return nullptr;

  std::vector<WideStringView> arguments;
  return pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      1, true, L"", std::move(arguments), std::move(expressions));
}

bool CXFA_FMParser::NextToken() {
  if (HasError())
    return false;
  m_token = m_lexer->NextToken();
  while (!HasError() && m_token->m_type == TOKreserver)
    m_token = m_lexer->NextToken();
  return !HasError();
}

bool CXFA_FMParser::CheckThenNext(XFA_FM_TOKEN op) {
  if (HasError())
    return false;

  if (m_token->m_type != op) {
    m_error = true;
    return false;
  }
  return NextToken();
}

bool CXFA_FMParser::IncrementParseDepthAndCheck() {
  return ++m_parse_depth < m_max_parse_depth;
}

std::vector<std::unique_ptr<CXFA_FMExpression>>
CXFA_FMParser::ParseTopExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return std::vector<std::unique_ptr<CXFA_FMExpression>>();

  std::unique_ptr<CXFA_FMExpression> expr;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  while (!HasError()) {
    if (m_token->m_type == TOKeof || m_token->m_type == TOKendfunc ||
        m_token->m_type == TOKendif || m_token->m_type == TOKelseif ||
        m_token->m_type == TOKelse || m_token->m_type == TOKreserver) {
      return expressions;
    }

    expr = m_token->m_type == TOKfunc ? ParseFunction() : ParseExpression();
    if (!expr) {
      m_error = true;
      break;
    }
    expressions.push_back(std::move(expr));
  }
  return std::vector<std::unique_ptr<CXFA_FMExpression>>();
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseFunction() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  WideStringView ident;
  std::vector<WideStringView> arguments;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;
  if (m_token->m_type != TOKidentifier) {
    m_error = true;
    return nullptr;
  } else {
    ident = m_token->m_string;
    if (!NextToken())
      return nullptr;
  }
  if (!CheckThenNext(TOKlparen))
    return nullptr;
  if (m_token->m_type == TOKrparen) {
    if (!NextToken())
      return nullptr;
  } else {
    while (1) {
      if (m_token->m_type != TOKidentifier) {
        m_error = true;
        return nullptr;
      }
      arguments.push_back(m_token->m_string);
      if (!NextToken())
        return nullptr;
      if (m_token->m_type == TOKcomma) {
        if (!NextToken())
          return nullptr;
        continue;
      }
      if (m_token->m_type == TOKrparen) {
        if (!NextToken())
          return nullptr;
      } else {
        if (!CheckThenNext(TOKrparen))
          return nullptr;
      }
      break;
    }
  }
  if (!CheckThenNext(TOKdo))
    return nullptr;
  if (m_token->m_type == TOKendfunc) {
    if (!NextToken())
      return nullptr;
  } else {
    expressions = ParseTopExpression();
    if (!expressions.size() || !CheckThenNext(TOKendfunc))
      return nullptr;
  }

  return pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      line, false, ident, std::move(arguments), std::move(expressions));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  uint32_t line = m_token->m_line_num;
  switch (m_token->m_type) {
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
      if (!NextToken())
        return nullptr;
      break;
    case TOKcontinue:
      expr = pdfium::MakeUnique<CXFA_FMContinueExpression>(line);
      if (!NextToken())
        return nullptr;
      break;
    default:
      m_error = true;
      return nullptr;
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseVarExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  WideStringView ident;
  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;
  if (m_token->m_type != TOKidentifier) {
    m_error = true;
    return nullptr;
  }

  ident = m_token->m_string;
  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  if (m_token->m_type == TOKassign) {
    if (!NextToken())
      return nullptr;

    expr = ParseExpExpression();
    if (!expr)
      return nullptr;
  }

  return pdfium::MakeUnique<CXFA_FMVarExpression>(line, ident, std::move(expr));
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseSimpleExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();
  if (!pExp1)
    return nullptr;
  int level = 1;
  while (m_token->m_type == TOKassign) {
    if (!NextToken())
      return nullptr;
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (!pExp2)
      return nullptr;
    if (level++ == kMaxAssignmentChainLength) {
      m_error = true;
      return nullptr;
    }
    pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
        line, TOKassign, std::move(pExp1), std::move(pExp2));
  }
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseExpExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseSimpleExpression();
  if (!pExp1)
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMExpExpression>(line, std::move(pExp1));
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseLogicalOrExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseLogicalAndExpression();
  if (!e1)
    return nullptr;

  for (;;) {
    switch (m_token->m_type) {
      case TOKor:
      case TOKksor: {
        if (!NextToken())
          return nullptr;

        std::unique_ptr<CXFA_FMSimpleExpression> e2(
            ParseLogicalAndExpression());
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMLogicalOrExpression>(
            line, TOKor, std::move(e1), std::move(e2));
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
CXFA_FMParser::ParseLogicalAndExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseEqualityExpression();
  if (!e1)
    return nullptr;

  for (;;) {
    switch (m_token->m_type) {
      case TOKand:
      case TOKksand: {
        if (!NextToken())
          return nullptr;

        std::unique_ptr<CXFA_FMSimpleExpression> e2 = ParseEqualityExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMLogicalAndExpression>(
            line, TOKand, std::move(e1), std::move(e2));
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
CXFA_FMParser::ParseEqualityExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseRelationalExpression();
  if (!e1)
    return nullptr;
  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_token->m_type) {
      case TOKeq:
      case TOKkseq:
        if (!NextToken())
          return nullptr;

        e2 = ParseRelationalExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
            line, TOKeq, std::move(e1), std::move(e2));
        continue;
      case TOKne:
      case TOKksne:
        if (!NextToken())
          return nullptr;

        e2 = ParseRelationalExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
            line, TOKne, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseRelationalExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseAddtiveExpression();
  if (!e1)
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_token->m_type) {
      case TOKlt:
      case TOKkslt:
        if (!NextToken())
          return nullptr;

        e2 = ParseAddtiveExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKlt, std::move(e1), std::move(e2));
        continue;
      case TOKgt:
      case TOKksgt:
        if (!NextToken())
          return nullptr;

        e2 = ParseAddtiveExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKgt, std::move(e1), std::move(e2));
        continue;
      case TOKle:
      case TOKksle:
        if (!NextToken())
          return nullptr;

        e2 = ParseAddtiveExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKle, std::move(e1), std::move(e2));
        continue;
      case TOKge:
      case TOKksge:
        if (!NextToken())
          return nullptr;

        e2 = ParseAddtiveExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKge, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseAddtiveExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseMultiplicativeExpression();
  if (!e1)
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_token->m_type) {
      case TOKplus:
        if (!NextToken())
          return nullptr;

        e2 = ParseMultiplicativeExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
            line, TOKplus, std::move(e1), std::move(e2));
        continue;
      case TOKminus:
        if (!NextToken())
          return nullptr;

        e2 = ParseMultiplicativeExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
            line, TOKminus, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseMultiplicativeExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseUnaryExpression();
  if (!e1)
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_token->m_type) {
      case TOKmul:
        if (!NextToken())
          return nullptr;

        e2 = ParseUnaryExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
            line, TOKmul, std::move(e1), std::move(e2));
        continue;
      case TOKdiv:
        if (!NextToken())
          return nullptr;

        e2 = ParseUnaryExpression();
        if (!e2)
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
            line, TOKdiv, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParser::ParseUnaryExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_token->m_line_num;
  switch (m_token->m_type) {
    case TOKplus:
      if (!NextToken())
        return nullptr;

      expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;

      expr = pdfium::MakeUnique<CXFA_FMPosExpression>(line, std::move(expr));
      break;
    case TOKminus:
      if (!NextToken())
        return nullptr;

      expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;

      expr = pdfium::MakeUnique<CXFA_FMNegExpression>(line, std::move(expr));
      break;
    case TOKksnot:
      if (!NextToken())
        return nullptr;

      expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;

      expr = pdfium::MakeUnique<CXFA_FMNotExpression>(line, std::move(expr));
      break;
    default:
      expr = ParsePrimaryExpression();
      if (!expr)
        return nullptr;
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParsePrimaryExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_token->m_line_num;
  switch (m_token->m_type) {
    case TOKnumber:
      expr =
          pdfium::MakeUnique<CXFA_FMNumberExpression>(line, m_token->m_string);
      if (!NextToken())
        return nullptr;
      break;
    case TOKstring:
      expr =
          pdfium::MakeUnique<CXFA_FMStringExpression>(line, m_token->m_string);
      if (!NextToken())
        return nullptr;
      break;
    case TOKidentifier: {
      WideStringView wsIdentifier(m_token->m_string);
      if (!NextToken())
        return nullptr;
      if (m_token->m_type == TOKlbracket) {
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (!s)
          return nullptr;

        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, nullptr, TOKdot, wsIdentifier, std::move(s));
        if (!expr)
          return nullptr;
        if (!NextToken())
          return nullptr;
      } else {
        expr =
            pdfium::MakeUnique<CXFA_FMIdentifierExpression>(line, wsIdentifier);
      }
      break;
    }
    case TOKif:
      expr = pdfium::MakeUnique<CXFA_FMIdentifierExpression>(line,
                                                             m_token->m_string);
      if (!expr || !NextToken())
        return nullptr;
      break;
    case TOKnull:
      expr = pdfium::MakeUnique<CXFA_FMNullExpression>(line);
      if (!expr || !NextToken())
        return nullptr;
      break;
    case TOKlparen:
      expr = ParseParenExpression();
      if (!expr)
        return nullptr;
      break;
    default:
      m_error = true;
      return nullptr;
  }
  expr = ParsePostExpression(std::move(expr));
  if (!expr)
    return nullptr;
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParser::ParsePostExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> expr) {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (HasError())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  while (1) {
    switch (m_token->m_type) {
      case TOKlparen: {
        if (!NextToken())
          return nullptr;
        std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
        if (m_token->m_type != TOKrparen) {
          while (m_token->m_type != TOKrparen) {
            std::unique_ptr<CXFA_FMSimpleExpression> simple_expr =
                ParseSimpleExpression();
            if (!simple_expr)
              return nullptr;

            expressions.push_back(std::move(simple_expr));
            if (m_token->m_type == TOKcomma) {
              if (!NextToken())
                return nullptr;
            } else if (m_token->m_type == TOKeof ||
                       m_token->m_type == TOKreserver) {
              break;
            }
          }
          if (m_token->m_type != TOKrparen) {
            m_error = true;
            return nullptr;
          }
        }
        expr = pdfium::MakeUnique<CXFA_FMCallExpression>(
            line, std::move(expr), std::move(expressions), false);
        if (!NextToken())
          return nullptr;
        if (m_token->m_type != TOKlbracket)
          continue;

        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (!s)
          return nullptr;

        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, std::move(expr), TOKcall, L"", std::move(s));
        break;
      }
      case TOKdot: {
        if (!NextToken())
          return nullptr;
        if (m_token->m_type != TOKidentifier) {
          m_error = true;
          return nullptr;
        }
        WideStringView tempStr = m_token->m_string;
        uint32_t tempLine = m_token->m_line_num;
        if (!NextToken())
          return nullptr;
        if (m_token->m_type == TOKlparen) {
          std::unique_ptr<CXFA_FMSimpleExpression> pExpCall;
          if (!NextToken())
            return nullptr;

          std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
          if (m_token->m_type != TOKrparen) {
            while (m_token->m_type != TOKrparen) {
              std::unique_ptr<CXFA_FMSimpleExpression> exp =
                  ParseSimpleExpression();
              if (!exp)
                return nullptr;

              expressions.push_back(std::move(exp));
              if (m_token->m_type == TOKcomma) {
                if (!NextToken())
                  return nullptr;
              } else if (m_token->m_type == TOKeof ||
                         m_token->m_type == TOKreserver) {
                break;
              }
            }
            if (m_token->m_type != TOKrparen) {
              m_error = true;
              return nullptr;
            }
          }
          std::unique_ptr<CXFA_FMSimpleExpression> pIdentifier =
              pdfium::MakeUnique<CXFA_FMIdentifierExpression>(tempLine,
                                                              tempStr);
          pExpCall = pdfium::MakeUnique<CXFA_FMCallExpression>(
              line, std::move(pIdentifier), std::move(expressions), true);
          expr = pdfium::MakeUnique<CXFA_FMMethodCallExpression>(
              line, std::move(expr), std::move(pExpCall));
          if (!NextToken())
            return nullptr;
          if (m_token->m_type != TOKlbracket)
            continue;

          std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, std::move(expr), TOKcall, L"", std::move(s));
        } else if (m_token->m_type == TOKlbracket) {
          std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              tempLine, std::move(expr), TOKdot, tempStr, std::move(s));
        } else {
          std::unique_ptr<CXFA_FMSimpleExpression> s =
              pdfium::MakeUnique<CXFA_FMIndexExpression>(
                  tempLine, ACCESSOR_NO_INDEX, nullptr, false);
          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, std::move(expr), TOKdot, tempStr, std::move(s));
          continue;
        }
      } break;
      case TOKdotdot: {
        if (!NextToken())
          return nullptr;
        if (m_token->m_type != TOKidentifier) {
          m_error = true;
          return nullptr;
        }
        WideStringView tempStr = m_token->m_string;
        uint32_t tempLine = m_token->m_line_num;
        if (!NextToken())
          return nullptr;
        if (m_token->m_type == TOKlbracket) {
          std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
              tempLine, std::move(expr), TOKdotdot, tempStr, std::move(s));
        } else {
          std::unique_ptr<CXFA_FMSimpleExpression> s =
              pdfium::MakeUnique<CXFA_FMIndexExpression>(
                  tempLine, ACCESSOR_NO_INDEX, nullptr, false);
          expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
              line, std::move(expr), TOKdotdot, tempStr, std::move(s));
          continue;
        }
      } break;
      case TOKdotscream: {
        if (!NextToken())
          return nullptr;
        if (m_token->m_type != TOKidentifier) {
          m_error = true;
          return nullptr;
        }
        WideStringView tempStr = m_token->m_string;
        uint32_t tempLine = m_token->m_line_num;
        if (!NextToken())
          return nullptr;
        if (m_token->m_type != TOKlbracket) {
          std::unique_ptr<CXFA_FMSimpleExpression> s =
              pdfium::MakeUnique<CXFA_FMIndexExpression>(
                  tempLine, ACCESSOR_NO_INDEX, nullptr, false);
          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, std::move(expr), TOKdotscream, tempStr, std::move(s));
          continue;
        }
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (!s)
          return nullptr;

        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            tempLine, std::move(expr), TOKdotscream, tempStr, std::move(s));
        break;
      }
      case TOKdotstar: {
        std::unique_ptr<CXFA_FMSimpleExpression> s =
            pdfium::MakeUnique<CXFA_FMIndexExpression>(line, ACCESSOR_NO_INDEX,
                                                       nullptr, false);
        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, std::move(expr), TOKdotstar, L"*", std::move(s));
        break;
      }
      default:
        return expr;
    }
    if (!NextToken())
      return nullptr;
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParser::ParseIndexExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> s;
  XFA_FM_AccessorIndex accessorIndex = ACCESSOR_NO_RELATIVEINDEX;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp;
  if (m_token->m_type == TOKmul) {
    pExp = pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                      std::move(s), true);
    if (!pExp || !NextToken())
      return nullptr;
    if (m_token->m_type != TOKrbracket) {
      m_error = true;
      return nullptr;
    }
    return pExp;
  }
  if (m_token->m_type == TOKplus) {
    accessorIndex = ACCESSOR_POSITIVE_INDEX;
    if (!NextToken())
      return nullptr;
  } else if (m_token->m_type == TOKminus) {
    accessorIndex = ACCESSOR_NEGATIVE_INDEX;
    if (!NextToken())
      return nullptr;
  }
  s = ParseSimpleExpression();
  if (!s)
    return nullptr;
  if (m_token->m_type != TOKrbracket) {
    m_error = true;
    return nullptr;
  }
  return pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                    std::move(s), false);
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParser::ParseParenExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (!CheckThenNext(TOKlparen))
    return nullptr;

  if (m_token->m_type == TOKrparen) {
    m_error = true;
    return nullptr;
  }

  uint32_t line = m_token->m_line_num;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();
  if (!pExp1)
    return nullptr;

  int level = 1;
  while (m_token->m_type == TOKassign) {
    if (!NextToken())
      return nullptr;

    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (!pExp2)
      return nullptr;
    if (level++ == kMaxAssignmentChainLength) {
      m_error = true;
      return nullptr;
    }

    pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
        line, TOKassign, std::move(pExp1), std::move(pExp2));
  }
  if (!CheckThenNext(TOKrparen))
    return nullptr;
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseBlockExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (HasError())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  while (1) {
    std::unique_ptr<CXFA_FMExpression> expr;
    switch (m_token->m_type) {
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
        if (!expr)
          return nullptr;

        expressions.push_back(std::move(expr));
        continue;
      default:
        expr = ParseExpression();
        if (!expr)
          return nullptr;

        expressions.push_back(std::move(expr));
        continue;
    }
    break;
  }
  return pdfium::MakeUnique<CXFA_FMBlockExpression>(line,
                                                    std::move(expressions));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseIfExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  const wchar_t* pStartPos = m_lexer->GetPos();
  if (!NextToken() || !CheckThenNext(TOKlparen))
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pExpression;
  while (m_token->m_type != TOKrparen) {
    pExpression = ParseSimpleExpression();
    if (!pExpression)
      return nullptr;
    if (m_token->m_type != TOKcomma)
      break;
    if (!NextToken())
      return nullptr;
  }
  if (!CheckThenNext(TOKrparen))
    return nullptr;
  if (m_token->m_type != TOKthen) {
    m_lexer->SetCurrentLine(line);
    auto pNewToken = pdfium::MakeUnique<CXFA_FMToken>(line);
    m_token = std::move(pNewToken);
    m_token->m_type = TOKidentifier;
    m_token->m_string = L"if";
    m_lexer->SetPos(pStartPos);
    return ParseExpExpression();
  }
  if (!CheckThenNext(TOKthen))
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> pIfExpression = ParseBlockExpression();
  if (!pIfExpression)
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> pElseExpression;
  switch (m_token->m_type) {
    case TOKeof:
    case TOKendif:
      if (!CheckThenNext(TOKendif))
        return nullptr;
      break;
    case TOKif:
      pElseExpression = ParseIfExpression();
      if (!pElseExpression || !CheckThenNext(TOKendif))
        return nullptr;
      break;
    case TOKelseif:
      pElseExpression = ParseIfExpression();
      if (!pElseExpression)
        return nullptr;
      break;
    case TOKelse:
      if (!NextToken())
        return nullptr;
      pElseExpression = ParseBlockExpression();
      if (!pElseExpression || !CheckThenNext(TOKendif))
        return nullptr;
      break;
    default:
      m_error = true;
      return nullptr;
  }
  return pdfium::MakeUnique<CXFA_FMIfExpression>(line, std::move(pExpression),
                                                 std::move(pIfExpression),
                                                 std::move(pElseExpression));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseWhileExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pCondition = ParseParenExpression();
  if (!pCondition || !CheckThenNext(TOKdo))
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> pExpression = ParseBlockExpression();
  if (!pExpression || !CheckThenNext(TOKendwhile))
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMWhileExpression>(line, std::move(pCondition),
                                                    std::move(pExpression));
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParser::ParseSubassignmentInForExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (HasError())
    return nullptr;

  if (m_token->m_type != TOKidentifier) {
    m_error = true;
    return nullptr;
  }
  std::unique_ptr<CXFA_FMSimpleExpression> expr = ParseSimpleExpression();
  if (!expr)
    return nullptr;
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseForExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  WideStringView wsVariant;
  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;
  if (m_token->m_type != TOKidentifier) {
    m_error = true;
    return nullptr;
  }

  wsVariant = m_token->m_string;
  if (!NextToken())
    return nullptr;
  if (m_token->m_type != TOKassign) {
    m_error = true;
    return nullptr;
  }
  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pAssignment =
      ParseSimpleExpression();
  if (!pAssignment)
    return nullptr;

  int32_t iDirection = 0;
  if (m_token->m_type == TOKupto) {
    iDirection = 1;
  } else if (m_token->m_type == TOKdownto) {
    iDirection = -1;
  } else {
    m_error = true;
    return nullptr;
  }

  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pAccessor = ParseSimpleExpression();
  if (!pAccessor)
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pStep;
  if (m_token->m_type == TOKstep) {
    if (!NextToken())
      return nullptr;
    pStep = ParseSimpleExpression();
    if (!pStep)
      return nullptr;
  }
  if (!CheckThenNext(TOKdo))
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> pList = ParseBlockExpression();
  if (!pList || !CheckThenNext(TOKendfor))
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  if (!expr)
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMForExpression>(
      line, wsVariant, std::move(pAssignment), std::move(pAccessor), iDirection,
      std::move(pStep), std::move(pList));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseForeachExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  WideStringView wsIdentifier;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> pAccessors;
  std::unique_ptr<CXFA_FMExpression> pList;
  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;
  if (m_token->m_type != TOKidentifier) {
    m_error = true;
    return nullptr;
  }

  wsIdentifier = m_token->m_string;
  if (!NextToken() || !CheckThenNext(TOKin) || !CheckThenNext(TOKlparen))
    return nullptr;
  if (m_token->m_type == TOKrparen) {
    m_error = true;
    return nullptr;
  }

  while (m_token->m_type != TOKrparen) {
    std::unique_ptr<CXFA_FMSimpleExpression> s = ParseSimpleExpression();
    if (!s)
      return nullptr;

    pAccessors.push_back(std::move(s));
    if (m_token->m_type != TOKcomma)
      break;
    if (!NextToken())
      return nullptr;
  }
  if (!CheckThenNext(TOKrparen) || !CheckThenNext(TOKdo))
    return nullptr;

  pList = ParseBlockExpression();
  if (!pList || !CheckThenNext(TOKendfor))
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMForeachExpression>(
      line, wsIdentifier, std::move(pAccessors), std::move(pList));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParser::ParseDoExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (HasError())
    return nullptr;

  uint32_t line = m_token->m_line_num;
  if (!NextToken())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr = ParseBlockExpression();
  if (!expr || !CheckThenNext(TOKend))
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMDoExpression>(line, std::move(expr));
}

bool CXFA_FMParser::HasError() const {
  return m_error || m_token == nullptr;
}
