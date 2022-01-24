// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmparser.h"

#include <utility>
#include <vector>

#include "core/fxcrt/autorestorer.h"
#include "v8/include/cppgc/heap.h"

namespace {

constexpr unsigned int kMaxParseDepth = 1250;
constexpr unsigned int kMaxPostExpressions = 256;
constexpr unsigned int kMaxExpressionListSize = 10000;

}  // namespace

CXFA_FMParser::CXFA_FMParser(cppgc::Heap* pHeap, CXFA_FMLexer* pLexer)
    : m_heap(pHeap), m_lexer(pLexer), m_max_parse_depth(kMaxParseDepth) {}

CXFA_FMParser::~CXFA_FMParser() = default;

CXFA_FMAST* CXFA_FMParser::Parse() {
  m_token = m_lexer->NextToken();
  if (HasError())
    return nullptr;

  auto expressions = ParseExpressionList();
  if (HasError())
    return nullptr;

  // We failed to parse all of the input so something has gone wrong.
  if (!m_lexer->IsComplete())
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMAST>(m_heap->GetAllocationHandle(),
                                                 std::move(expressions));
}

bool CXFA_FMParser::NextToken() {
  if (HasError())
    return false;

  m_token = m_lexer->NextToken();
  while (!HasError() && m_token.m_type == TOKreserver)
    m_token = m_lexer->NextToken();
  return !HasError();
}

bool CXFA_FMParser::CheckThenNext(XFA_FM_TOKEN op) {
  if (HasError())
    return false;

  if (m_token.m_type != op) {
    m_error = true;
    return false;
  }
  return NextToken();
}

bool CXFA_FMParser::IncrementParseDepthAndCheck() {
  return ++m_parse_depth < m_max_parse_depth;
}

std::vector<cppgc::Member<CXFA_FMExpression>>
CXFA_FMParser::ParseExpressionList() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return std::vector<cppgc::Member<CXFA_FMExpression>>();

  std::vector<cppgc::Member<CXFA_FMExpression>> expressions;
  while (!HasError()) {
    if (m_token.m_type == TOKeof || m_token.m_type == TOKendfunc ||
        m_token.m_type == TOKendif || m_token.m_type == TOKelseif ||
        m_token.m_type == TOKelse || m_token.m_type == TOKendwhile ||
        m_token.m_type == TOKendfor || m_token.m_type == TOKend ||
        m_token.m_type == TOKendfunc || m_token.m_type == TOKreserver) {
      break;
    }

    CXFA_FMExpression* expr =
        m_token.m_type == TOKfunc ? ParseFunction() : ParseExpression();
    if (!expr) {
      m_error = true;
      return std::vector<cppgc::Member<CXFA_FMExpression>>();
    }
    if (expressions.size() >= kMaxExpressionListSize) {
      m_error = true;
      return std::vector<cppgc::Member<CXFA_FMExpression>>();
    }
    expressions.push_back(expr);
  }
  return expressions;
}

// Func := 'func' Identifier '(' ParameterList ')' do ExpressionList 'endfunc'
// ParamterList := (Not actually defined in the grammar) .....
//                 (Identifier (',' Identifier)*)?
CXFA_FMExpression* CXFA_FMParser::ParseFunction() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKfunc))
    return nullptr;
  if (m_token.m_type != TOKidentifier)
    return nullptr;

  WideString ident(m_token.m_string);
  if (!NextToken())
    return nullptr;
  if (!CheckThenNext(TOKlparen))
    return nullptr;

  std::vector<WideString> arguments;
  bool last_was_comma = false;
  while (true) {
    if (m_token.m_type == TOKrparen)
      break;
    if (m_token.m_type != TOKidentifier)
      return nullptr;

    last_was_comma = false;

    arguments.emplace_back(m_token.m_string);
    if (!NextToken())
      return nullptr;
    if (m_token.m_type != TOKcomma)
      continue;

    last_was_comma = true;
    if (!NextToken())
      return nullptr;
  }
  if (last_was_comma || !CheckThenNext(TOKrparen))
    return nullptr;
  if (!CheckThenNext(TOKdo))
    return nullptr;

  std::vector<cppgc::Member<CXFA_FMExpression>> expressions;
  if (m_token.m_type != TOKendfunc)
    expressions = ParseExpressionList();

  if (!CheckThenNext(TOKendfunc))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMFunctionDefinition>(
      m_heap->GetAllocationHandle(), std::move(ident), std::move(arguments),
      std::move(expressions));
}

// Expression := IfExpression | WhileExpression | ForExpression |
//               ForEachExpression | AssignmentExpression |
//               DeclarationExpression | SimpleExpression
CXFA_FMExpression* CXFA_FMParser::ParseExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMExpression* expr = nullptr;
  switch (m_token.m_type) {
    case TOKvar:
      expr = ParseDeclarationExpression();
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
      expr = cppgc::MakeGarbageCollected<CXFA_FMBreakExpression>(
          m_heap->GetAllocationHandle());
      if (!NextToken())
        return nullptr;
      break;
    case TOKcontinue:
      expr = cppgc::MakeGarbageCollected<CXFA_FMContinueExpression>(
          m_heap->GetAllocationHandle());
      if (!NextToken())
        return nullptr;
      break;
    default:
      return nullptr;
  }
  return expr;
}

// Declaration := 'var' Variable | 'var' Variable '=' SimpleExpression |
//           'Func' Identifier '(' ParameterList ')' do ExpressionList 'EndFunc'
// TODO(dsinclair): We appear to be handling the 'func' case elsewhere.
CXFA_FMExpression* CXFA_FMParser::ParseDeclarationExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (!NextToken() || m_token.m_type != TOKidentifier)
    return nullptr;

  WideString ident(m_token.m_string);
  if (!NextToken())
    return nullptr;

  CXFA_FMSimpleExpression* expr = nullptr;
  if (m_token.m_type == TOKassign) {
    if (!NextToken())
      return nullptr;

    expr = ParseSimpleExpression();
    if (!expr)
      return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMVarExpression>(
      m_heap->GetAllocationHandle(), std::move(ident), expr);
}

// SimpleExpression := LogicalOrExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseSimpleExpression() {
  if (HasError())
    return nullptr;

  return ParseLogicalOrExpression();
}

// Exp := SimpleExpression ( '=' SimpleExpression )?
CXFA_FMExpression* CXFA_FMParser::ParseExpExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* pExp1 = ParseSimpleExpression();
  if (!pExp1)
    return nullptr;

  if (m_token.m_type == TOKassign) {
    if (!NextToken())
      return nullptr;

    CXFA_FMSimpleExpression* pExp2 = ParseSimpleExpression();
    if (!pExp2)
      return nullptr;

    pExp1 = cppgc::MakeGarbageCollected<CXFA_FMAssignExpression>(
        m_heap->GetAllocationHandle(), TOKassign, pExp1, pExp2);
  }
  return cppgc::MakeGarbageCollected<CXFA_FMExpExpression>(
      m_heap->GetAllocationHandle(), pExp1);
}

// LogicalOr := LogicalAndExpression |
//              LogicalOrExpression LogicalOrOperator LogicalAndExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseLogicalOrExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseLogicalAndExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKor:
      case TOKksor: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseLogicalAndExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLogicalOrExpression>(
            m_heap->GetAllocationHandle(), TOKor, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// LogicalAnd := EqualityExpression |
//               LogicalAndExpression LogicalAndOperator EqualityExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseLogicalAndExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseEqualityExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKand:
      case TOKksand: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseEqualityExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLogicalAndExpression>(
            m_heap->GetAllocationHandle(), TOKand, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Equality := RelationExpression |
//             EqualityExpression EqulaityOperator RelationalExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseEqualityExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseRelationalExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKeq:
      case TOKkseq: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseRelationalExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMEqualExpression>(
            m_heap->GetAllocationHandle(), TOKeq, e1, e2);
        break;
      }
      case TOKne:
      case TOKksne: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseRelationalExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMNotEqualExpression>(
            m_heap->GetAllocationHandle(), TOKne, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Relational := AdditiveExpression |
//               RelationalExpression RelationalOperator AdditiveExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseRelationalExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseAdditiveExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKlt:
      case TOKkslt: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLtExpression>(
            m_heap->GetAllocationHandle(), TOKlt, e1, e2);
        break;
      }
      case TOKgt:
      case TOKksgt: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMGtExpression>(
            m_heap->GetAllocationHandle(), TOKgt, e1, e2);
        break;
      }
      case TOKle:
      case TOKksle: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLeExpression>(
            m_heap->GetAllocationHandle(), TOKle, e1, e2);
        break;
      }
      case TOKge:
      case TOKksge: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMGeExpression>(
            m_heap->GetAllocationHandle(), TOKge, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Additive := MultiplicativeExpression |
//             AdditiveExpression AdditiveOperator MultiplicativeExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseAdditiveExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseMultiplicativeExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKplus: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseMultiplicativeExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMPlusExpression>(
            m_heap->GetAllocationHandle(), TOKplus, e1, e2);
        break;
      }
      case TOKminus: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseMultiplicativeExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMMinusExpression>(
            m_heap->GetAllocationHandle(), TOKminus, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Multiplicative := UnaryExpression |
//                 MultiplicateExpression MultiplicativeOperator UnaryExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseMultiplicativeExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* e1 = ParseUnaryExpression();
  if (!e1)
    return nullptr;

  while (true) {
    if (!IncrementParseDepthAndCheck())
      return nullptr;

    switch (m_token.m_type) {
      case TOKmul: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseUnaryExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMMulExpression>(
            m_heap->GetAllocationHandle(), TOKmul, e1, e2);
        break;
      }
      case TOKdiv: {
        if (!NextToken())
          return nullptr;
        CXFA_FMSimpleExpression* e2 = ParseUnaryExpression();
        if (!e2)
          return nullptr;
        e1 = cppgc::MakeGarbageCollected<CXFA_FMDivExpression>(
            m_heap->GetAllocationHandle(), TOKdiv, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Unary := PrimaryExpression | UnaryOperator UnaryExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseUnaryExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  switch (m_token.m_type) {
    case TOKplus: {
      if (!NextToken())
        return nullptr;
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;
      return cppgc::MakeGarbageCollected<CXFA_FMPosExpression>(
          m_heap->GetAllocationHandle(), expr);
    }
    case TOKminus: {
      if (!NextToken())
        return nullptr;
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;
      return cppgc::MakeGarbageCollected<CXFA_FMNegExpression>(
          m_heap->GetAllocationHandle(), expr);
    }
    case TOKksnot: {
      if (!NextToken())
        return nullptr;
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr)
        return nullptr;
      return cppgc::MakeGarbageCollected<CXFA_FMNotExpression>(
          m_heap->GetAllocationHandle(), expr);
    }
    default:
      return ParsePrimaryExpression();
  }
}

// Primary := Literal | FunctionCall | Accessor ('.*' )? |
//           '(' SimpleExpression ')'
CXFA_FMSimpleExpression* CXFA_FMParser::ParsePrimaryExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  CXFA_FMSimpleExpression* expr = ParseLiteral();
  if (expr)
    return NextToken() ? expr : nullptr;

  switch (m_token.m_type) {
    case TOKidentifier: {
      WideString wsIdentifier(m_token.m_string);
      if (!NextToken())
        return nullptr;
      if (m_token.m_type == TOKlbracket) {
        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s)
          return nullptr;
        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            m_heap->GetAllocationHandle(), nullptr, TOKdot,
            std::move(wsIdentifier), s);
        if (!expr)
          return nullptr;
        if (!NextToken())
          return nullptr;
      } else {
        expr = cppgc::MakeGarbageCollected<CXFA_FMIdentifierExpression>(
            m_heap->GetAllocationHandle(), wsIdentifier);
      }
      break;
    }
    case TOKlparen: {
      expr = ParseParenExpression();
      if (!expr)
        return nullptr;
      break;
    }
    default:
      return nullptr;
  }
  return ParsePostExpression(expr);
}

// Literal := String | Number | Null
CXFA_FMSimpleExpression* CXFA_FMParser::ParseLiteral() {
  switch (m_token.m_type) {
    case TOKnumber:
      return cppgc::MakeGarbageCollected<CXFA_FMNumberExpression>(
          m_heap->GetAllocationHandle(), WideString(m_token.m_string));
    case TOKstring:
      return cppgc::MakeGarbageCollected<CXFA_FMStringExpression>(
          m_heap->GetAllocationHandle(), WideString(m_token.m_string));
    case TOKnull:
      return cppgc::MakeGarbageCollected<CXFA_FMNullExpression>(
          m_heap->GetAllocationHandle());
    default:
      return nullptr;
  }
}

// TODO(dsinclair): Make this match up to the grammar
// I believe this is parsing the accessor ( '.' | '..' | '.#' )
CXFA_FMSimpleExpression* CXFA_FMParser::ParsePostExpression(
    CXFA_FMSimpleExpression* expr) {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  size_t expr_count = 0;
  while (true) {
    ++expr_count;
    // Limit the number of expressions allowed in the post expression statement.
    // If we don't do this then its possible to generate a stack overflow
    // by having a very large number of things like .. expressions.
    if (expr_count > kMaxPostExpressions)
      return nullptr;

    switch (m_token.m_type) {
      case TOKlparen: {
        absl::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
            expressions = ParseArgumentList();
        if (!expressions.has_value())
          return nullptr;

        expr = cppgc::MakeGarbageCollected<CXFA_FMCallExpression>(
            m_heap->GetAllocationHandle(), expr, std::move(expressions.value()),
            false);
        if (!NextToken())
          return nullptr;
        if (m_token.m_type != TOKlbracket)
          continue;

        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s)
          return nullptr;

        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            m_heap->GetAllocationHandle(), expr, TOKcall, WideString(), s);
        break;
      }
      case TOKdot: {
        if (!NextToken())
          return nullptr;
        if (m_token.m_type != TOKidentifier)
          return nullptr;

        WideString tempStr(m_token.m_string);
        if (!NextToken())
          return nullptr;
        if (m_token.m_type == TOKlparen) {
          absl::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
              expressions = ParseArgumentList();
          if (!expressions.has_value())
            return nullptr;

          auto* pIdentifier =
              cppgc::MakeGarbageCollected<CXFA_FMIdentifierExpression>(
                  m_heap->GetAllocationHandle(), std::move(tempStr));
          auto* pExpCall = cppgc::MakeGarbageCollected<CXFA_FMCallExpression>(
              m_heap->GetAllocationHandle(), pIdentifier,
              std::move(expressions.value()), true);
          expr = cppgc::MakeGarbageCollected<CXFA_FMMethodCallExpression>(
              m_heap->GetAllocationHandle(), expr, pExpCall);
          if (!NextToken())
            return nullptr;
          if (m_token.m_type != TOKlbracket)
            continue;

          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKcall, WideString(), s);
        } else if (m_token.m_type == TOKlbracket) {
          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKdot, std::move(tempStr),
              s);
        } else {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              m_heap->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKdot, std::move(tempStr),
              subexpr);
          continue;
        }
        break;
      }
      case TOKdotdot: {
        if (!NextToken())
          return nullptr;
        if (m_token.m_type != TOKidentifier)
          return nullptr;

        WideString tempStr(m_token.m_string);
        if (!NextToken())
          return nullptr;
        if (m_token.m_type == TOKlbracket) {
          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s)
            return nullptr;

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKdotdot,
              std::move(tempStr), s);
        } else {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              m_heap->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKdotdot,
              std::move(tempStr), subexpr);
          continue;
        }
        break;
      }
      case TOKdotscream: {
        if (!NextToken())
          return nullptr;
        if (m_token.m_type != TOKidentifier)
          return nullptr;

        WideString tempStr(m_token.m_string);
        if (!NextToken())
          return nullptr;

        if (m_token.m_type != TOKlbracket) {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              m_heap->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              m_heap->GetAllocationHandle(), expr, TOKdotscream,
              std::move(tempStr), subexpr);
          continue;
        }

        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s)
          return nullptr;

        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            m_heap->GetAllocationHandle(), expr, TOKdotscream,
            std::move(tempStr), s);
        break;
      }
      case TOKdotstar: {
        auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
            m_heap->GetAllocationHandle(),
            CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            m_heap->GetAllocationHandle(), expr, TOKdotstar, L"*", subexpr);
        break;
      }
      default:
        return expr;
    }
    if (!NextToken())
      return nullptr;
  }
}

// Argument lists are zero or more comma seperated simple expressions found
// between '(' and ')'
absl::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
CXFA_FMParser::ParseArgumentList() {
  if (m_token.m_type != TOKlparen || !NextToken())
    return absl::nullopt;

  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> expressions;
  bool first_arg = true;
  while (m_token.m_type != TOKrparen) {
    if (first_arg) {
      first_arg = false;
    } else {
      if (m_token.m_type != TOKcomma || !NextToken())
        return absl::nullopt;
    }

    CXFA_FMSimpleExpression* exp = ParseSimpleExpression();
    if (!exp)
      return absl::nullopt;

    expressions.push_back(exp);
    if (expressions.size() > kMaxPostExpressions)
      return absl::nullopt;
  }

  return expressions;
}

// Index := '[' ('*' | '+' SimpleExpression | '-' SimpleExpression) ']'
CXFA_FMSimpleExpression* CXFA_FMParser::ParseIndexExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKlbracket))
    return nullptr;

  if (m_token.m_type == TOKmul) {
    auto* pExp = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
        m_heap->GetAllocationHandle(),
        CXFA_FMIndexExpression::AccessorIndex::kNoRelativeIndex, nullptr, true);
    if (!pExp || !NextToken())
      return nullptr;

    // TODO(dsinclair): This should CheckThenNext(TOKrbracket) but need to clean
    // up the callsites.
    if (m_token.m_type != TOKrbracket)
      return nullptr;
    return pExp;
  }

  CXFA_FMIndexExpression::AccessorIndex accessorIndex =
      CXFA_FMIndexExpression::AccessorIndex::kNoRelativeIndex;
  if (m_token.m_type == TOKplus) {
    accessorIndex = CXFA_FMIndexExpression::AccessorIndex::kPositiveIndex;
    if (!NextToken())
      return nullptr;
  } else if (m_token.m_type == TOKminus) {
    accessorIndex = CXFA_FMIndexExpression::AccessorIndex::kNegativeIndex;
    if (!NextToken())
      return nullptr;
  }

  CXFA_FMSimpleExpression* s = ParseSimpleExpression();
  if (!s)
    return nullptr;
  if (m_token.m_type != TOKrbracket)
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
      m_heap->GetAllocationHandle(), accessorIndex, s, false);
}

// Paren := '(' SimpleExpression ')'
CXFA_FMSimpleExpression* CXFA_FMParser::ParseParenExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (!CheckThenNext(TOKlparen))
    return nullptr;
  if (m_token.m_type == TOKrparen)
    return nullptr;

  CXFA_FMSimpleExpression* pExp1 = ParseSimpleExpression();
  if (!pExp1)
    return nullptr;

  if (!CheckThenNext(TOKrparen))
    return nullptr;
  return pExp1;
}

// If := 'if' '(' SimpleExpression ')' 'then' ExpressionList
//       ('elseif' '(' SimpleExpression ')' 'then' ExpressionList)*
//       ('else' ExpressionList)?
//       'endif'
CXFA_FMExpression* CXFA_FMParser::ParseIfExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;

  if (!CheckThenNext(TOKif))
    return nullptr;

  CXFA_FMSimpleExpression* pCondition = ParseParenExpression();
  if (!pCondition)
    return nullptr;
  if (!CheckThenNext(TOKthen))
    return nullptr;

  auto* pIfExpressions = cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
      m_heap->GetAllocationHandle(), ParseExpressionList());

  std::vector<cppgc::Member<CXFA_FMIfExpression>> pElseIfExpressions;
  while (m_token.m_type == TOKelseif) {
    if (!NextToken())
      return nullptr;

    auto* elseIfCondition = ParseParenExpression();
    if (!elseIfCondition)
      return nullptr;
    if (!CheckThenNext(TOKthen))
      return nullptr;

    auto elseIfExprs = ParseExpressionList();
    pElseIfExpressions.push_back(
        cppgc::MakeGarbageCollected<CXFA_FMIfExpression>(
            m_heap->GetAllocationHandle(), elseIfCondition,
            cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
                m_heap->GetAllocationHandle(), std::move(elseIfExprs)),
            std::vector<cppgc::Member<CXFA_FMIfExpression>>(), nullptr));
  }

  CXFA_FMExpression* pElseExpression = nullptr;
  if (m_token.m_type == TOKelse) {
    if (!NextToken())
      return nullptr;

    pElseExpression = cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
        m_heap->GetAllocationHandle(), ParseExpressionList());
  }
  if (!CheckThenNext(TOKendif))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMIfExpression>(
      m_heap->GetAllocationHandle(), pCondition, pIfExpressions,
      std::move(pElseIfExpressions), pElseExpression);
}

// While := 'while' '(' SimpleExpression ')' 'do' ExpressionList 'endwhile'
CXFA_FMExpression* CXFA_FMParser::ParseWhileExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKwhile))
    return nullptr;

  CXFA_FMSimpleExpression* pCondition = ParseParenExpression();
  if (!pCondition || !CheckThenNext(TOKdo))
    return nullptr;

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendwhile))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMWhileExpression>(
      m_heap->GetAllocationHandle(), pCondition,
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          m_heap->GetAllocationHandle(), std::move(exprs)));
}

// For := 'for' Assignment 'upto' Accessor ('step' SimpleExpression)?
//            'do' ExpressionList 'endfor' |
//         'for' Assignment 'downto' Accessor ('step' SimpleExpression)?
//            'do' ExpressionList 'endfor'
CXFA_FMExpression* CXFA_FMParser::ParseForExpression() {
  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKfor))
    return nullptr;
  if (m_token.m_type != TOKidentifier)
    return nullptr;

  WideString wsVariant(m_token.m_string);
  if (!NextToken())
    return nullptr;
  if (!CheckThenNext(TOKassign))
    return nullptr;

  CXFA_FMSimpleExpression* pAssignment = ParseSimpleExpression();
  if (!pAssignment)
    return nullptr;

  int32_t iDirection = 0;
  if (m_token.m_type == TOKupto)
    iDirection = 1;
  else if (m_token.m_type == TOKdownto)
    iDirection = -1;
  else
    return nullptr;

  if (!NextToken())
    return nullptr;

  CXFA_FMSimpleExpression* pAccessor = ParseSimpleExpression();
  if (!pAccessor)
    return nullptr;

  CXFA_FMSimpleExpression* pStep = nullptr;
  if (m_token.m_type == TOKstep) {
    if (!NextToken())
      return nullptr;
    pStep = ParseSimpleExpression();
    if (!pStep)
      return nullptr;
  }
  if (!CheckThenNext(TOKdo))
    return nullptr;

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendfor))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMForExpression>(
      m_heap->GetAllocationHandle(), wsVariant, pAssignment, pAccessor,
      iDirection, pStep,
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          m_heap->GetAllocationHandle(), std::move(exprs)));
}

// Foreach := 'foreach' Identifier 'in' '(' ArgumentList ')'
//            'do' ExpressionList 'endfor'
CXFA_FMExpression* CXFA_FMParser::ParseForeachExpression() {
  if (m_token.m_type != TOKforeach)
    return nullptr;

  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKforeach))
    return nullptr;
  if (m_token.m_type != TOKidentifier)
    return nullptr;

  WideString wsIdentifier(m_token.m_string);
  if (!NextToken() || !CheckThenNext(TOKin) || !CheckThenNext(TOKlparen))
    return nullptr;

  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> pArgumentList;
  while (m_token.m_type != TOKrparen) {
    CXFA_FMSimpleExpression* s = ParseSimpleExpression();
    if (!s)
      return nullptr;

    pArgumentList.push_back(s);
    if (m_token.m_type != TOKcomma)
      break;
    if (!NextToken())
      return nullptr;
  }
  // We must have arguments.
  if (pArgumentList.empty())
    return nullptr;
  if (!CheckThenNext(TOKrparen))
    return nullptr;

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendfor))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMForeachExpression>(
      m_heap->GetAllocationHandle(), std::move(wsIdentifier),
      std::move(pArgumentList),
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          m_heap->GetAllocationHandle(), std::move(exprs)));
}

// Block := 'do' ExpressionList 'end'
CXFA_FMExpression* CXFA_FMParser::ParseDoExpression() {
  if (m_token.m_type != TOKdo)
    return nullptr;

  AutoRestorer<unsigned long> restorer(&m_parse_depth);
  if (HasError() || !IncrementParseDepthAndCheck())
    return nullptr;
  if (!CheckThenNext(TOKdo))
    return nullptr;

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKend))
    return nullptr;

  return cppgc::MakeGarbageCollected<CXFA_FMDoExpression>(
      m_heap->GetAllocationHandle(),
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          m_heap->GetAllocationHandle(), std::move(exprs)));
}

bool CXFA_FMParser::HasError() const {
  return m_error || m_token.m_type == TOKreserver;
}
