// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/formcalc/cxfa_fmparser.h"

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
    : heap_(pHeap), lexer_(pLexer), max_parse_depth_(kMaxParseDepth) {}

CXFA_FMParser::~CXFA_FMParser() = default;

CXFA_FMAST* CXFA_FMParser::Parse() {
  token_ = lexer_->NextToken();
  if (HasError()) {
    return nullptr;
  }

  auto expressions = ParseExpressionList();
  if (HasError()) {
    return nullptr;
  }

  // We failed to parse all of the input so something has gone wrong.
  if (!lexer_->IsComplete()) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMAST>(heap_->GetAllocationHandle(),
                                                 std::move(expressions));
}

bool CXFA_FMParser::NextToken() {
  if (HasError()) {
    return false;
  }

  token_ = lexer_->NextToken();
  while (!HasError() && token_.GetType() == TOKreserver) {
    token_ = lexer_->NextToken();
  }
  return !HasError();
}

bool CXFA_FMParser::CheckThenNext(XFA_FM_TOKEN op) {
  if (HasError()) {
    return false;
  }

  if (token_.GetType() != op) {
    error_ = true;
    return false;
  }
  return NextToken();
}

bool CXFA_FMParser::IncrementParseDepthAndCheck() {
  return ++parse_depth_ < max_parse_depth_;
}

std::vector<cppgc::Member<CXFA_FMExpression>>
CXFA_FMParser::ParseExpressionList() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return std::vector<cppgc::Member<CXFA_FMExpression>>();
  }

  std::vector<cppgc::Member<CXFA_FMExpression>> expressions;
  while (!HasError()) {
    if (token_.GetType() == TOKeof || token_.GetType() == TOKendfunc ||
        token_.GetType() == TOKendif || token_.GetType() == TOKelseif ||
        token_.GetType() == TOKelse || token_.GetType() == TOKendwhile ||
        token_.GetType() == TOKendfor || token_.GetType() == TOKend ||
        token_.GetType() == TOKendfunc || token_.GetType() == TOKreserver) {
      break;
    }

    CXFA_FMExpression* expr =
        token_.GetType() == TOKfunc ? ParseFunction() : ParseExpression();
    if (!expr) {
      error_ = true;
      return std::vector<cppgc::Member<CXFA_FMExpression>>();
    }
    if (expressions.size() >= kMaxExpressionListSize) {
      error_ = true;
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKfunc)) {
    return nullptr;
  }
  if (token_.GetType() != TOKidentifier) {
    return nullptr;
  }

  WideString ident(token_.GetString());
  if (!NextToken()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKlparen)) {
    return nullptr;
  }

  std::vector<WideString> arguments;
  bool last_was_comma = false;
  while (true) {
    if (token_.GetType() == TOKrparen) {
      break;
    }
    if (token_.GetType() != TOKidentifier) {
      return nullptr;
    }

    last_was_comma = false;

    arguments.emplace_back(token_.GetString());
    if (!NextToken()) {
      return nullptr;
    }
    if (token_.GetType() != TOKcomma) {
      continue;
    }

    last_was_comma = true;
    if (!NextToken()) {
      return nullptr;
    }
  }
  if (last_was_comma || !CheckThenNext(TOKrparen)) {
    return nullptr;
  }
  if (!CheckThenNext(TOKdo)) {
    return nullptr;
  }

  std::vector<cppgc::Member<CXFA_FMExpression>> expressions;
  if (token_.GetType() != TOKendfunc) {
    expressions = ParseExpressionList();
  }

  if (!CheckThenNext(TOKendfunc)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMFunctionDefinition>(
      heap_->GetAllocationHandle(), std::move(ident), std::move(arguments),
      std::move(expressions));
}

// Expression := IfExpression | WhileExpression | ForExpression |
//               ForEachExpression | AssignmentExpression |
//               DeclarationExpression | SimpleExpression
CXFA_FMExpression* CXFA_FMParser::ParseExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMExpression* expr = nullptr;
  switch (token_.GetType()) {
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
          heap_->GetAllocationHandle());
      if (!NextToken()) {
        return nullptr;
      }
      break;
    case TOKcontinue:
      expr = cppgc::MakeGarbageCollected<CXFA_FMContinueExpression>(
          heap_->GetAllocationHandle());
      if (!NextToken()) {
        return nullptr;
      }
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  if (!NextToken() || token_.GetType() != TOKidentifier) {
    return nullptr;
  }

  WideString ident(token_.GetString());
  if (!NextToken()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* expr = nullptr;
  if (token_.GetType() == TOKassign) {
    if (!NextToken()) {
      return nullptr;
    }

    expr = ParseSimpleExpression();
    if (!expr) {
      return nullptr;
    }
  }

  return cppgc::MakeGarbageCollected<CXFA_FMVarExpression>(
      heap_->GetAllocationHandle(), std::move(ident), expr);
}

// SimpleExpression := LogicalOrExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseSimpleExpression() {
  if (HasError()) {
    return nullptr;
  }

  return ParseLogicalOrExpression();
}

// Exp := SimpleExpression ( '=' SimpleExpression )?
CXFA_FMExpression* CXFA_FMParser::ParseExpExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pExp1 = ParseSimpleExpression();
  if (!pExp1) {
    return nullptr;
  }

  if (token_.GetType() == TOKassign) {
    if (!NextToken()) {
      return nullptr;
    }

    CXFA_FMSimpleExpression* pExp2 = ParseSimpleExpression();
    if (!pExp2) {
      return nullptr;
    }

    pExp1 = cppgc::MakeGarbageCollected<CXFA_FMAssignExpression>(
        heap_->GetAllocationHandle(), TOKassign, pExp1, pExp2);
  }
  return cppgc::MakeGarbageCollected<CXFA_FMExpExpression>(
      heap_->GetAllocationHandle(), pExp1);
}

// LogicalOr := LogicalAndExpression |
//              LogicalOrExpression LogicalOrOperator LogicalAndExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseLogicalOrExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseLogicalAndExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKor:
      case TOKksor: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseLogicalAndExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLogicalOrExpression>(
            heap_->GetAllocationHandle(), TOKor, e1, e2);
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseEqualityExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKand:
      case TOKksand: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseEqualityExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLogicalAndExpression>(
            heap_->GetAllocationHandle(), TOKand, e1, e2);
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseRelationalExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKeq:
      case TOKkseq: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseRelationalExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMEqualExpression>(
            heap_->GetAllocationHandle(), TOKeq, e1, e2);
        break;
      }
      case TOKne:
      case TOKksne: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseRelationalExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMNotEqualExpression>(
            heap_->GetAllocationHandle(), TOKne, e1, e2);
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseAdditiveExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKlt:
      case TOKkslt: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLtExpression>(
            heap_->GetAllocationHandle(), TOKlt, e1, e2);
        break;
      }
      case TOKgt:
      case TOKksgt: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMGtExpression>(
            heap_->GetAllocationHandle(), TOKgt, e1, e2);
        break;
      }
      case TOKle:
      case TOKksle: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMLeExpression>(
            heap_->GetAllocationHandle(), TOKle, e1, e2);
        break;
      }
      case TOKge:
      case TOKksge: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseAdditiveExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMGeExpression>(
            heap_->GetAllocationHandle(), TOKge, e1, e2);
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseMultiplicativeExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKplus: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseMultiplicativeExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMPlusExpression>(
            heap_->GetAllocationHandle(), TOKplus, e1, e2);
        break;
      }
      case TOKminus: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseMultiplicativeExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMMinusExpression>(
            heap_->GetAllocationHandle(), TOKminus, e1, e2);
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
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* e1 = ParseUnaryExpression();
  if (!e1) {
    return nullptr;
  }

  while (true) {
    if (!IncrementParseDepthAndCheck()) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKmul: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseUnaryExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMMulExpression>(
            heap_->GetAllocationHandle(), TOKmul, e1, e2);
        break;
      }
      case TOKdiv: {
        if (!NextToken()) {
          return nullptr;
        }
        CXFA_FMSimpleExpression* e2 = ParseUnaryExpression();
        if (!e2) {
          return nullptr;
        }
        e1 = cppgc::MakeGarbageCollected<CXFA_FMDivExpression>(
            heap_->GetAllocationHandle(), TOKdiv, e1, e2);
        break;
      }
      default:
        return e1;
    }
  }
}

// Unary := PrimaryExpression | UnaryOperator UnaryExpression
CXFA_FMSimpleExpression* CXFA_FMParser::ParseUnaryExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  switch (token_.GetType()) {
    case TOKplus: {
      if (!NextToken()) {
        return nullptr;
      }
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr) {
        return nullptr;
      }
      return cppgc::MakeGarbageCollected<CXFA_FMPosExpression>(
          heap_->GetAllocationHandle(), expr);
    }
    case TOKminus: {
      if (!NextToken()) {
        return nullptr;
      }
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr) {
        return nullptr;
      }
      return cppgc::MakeGarbageCollected<CXFA_FMNegExpression>(
          heap_->GetAllocationHandle(), expr);
    }
    case TOKksnot: {
      if (!NextToken()) {
        return nullptr;
      }
      CXFA_FMSimpleExpression* expr = ParseUnaryExpression();
      if (!expr) {
        return nullptr;
      }
      return cppgc::MakeGarbageCollected<CXFA_FMNotExpression>(
          heap_->GetAllocationHandle(), expr);
    }
    default:
      return ParsePrimaryExpression();
  }
}

// Primary := Literal | FunctionCall | Accessor ('.*' )? |
//           '(' SimpleExpression ')'
CXFA_FMSimpleExpression* CXFA_FMParser::ParsePrimaryExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* expr = ParseLiteral();
  if (expr) {
    return NextToken() ? expr : nullptr;
  }

  switch (token_.GetType()) {
    case TOKidentifier: {
      WideString wsIdentifier(token_.GetString());
      if (!NextToken()) {
        return nullptr;
      }
      if (token_.GetType() == TOKlbracket) {
        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s) {
          return nullptr;
        }
        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            heap_->GetAllocationHandle(), nullptr, TOKdot,
            std::move(wsIdentifier), s);
        if (!expr) {
          return nullptr;
        }
        if (!NextToken()) {
          return nullptr;
        }
      } else {
        expr = cppgc::MakeGarbageCollected<CXFA_FMIdentifierExpression>(
            heap_->GetAllocationHandle(), wsIdentifier);
      }
      break;
    }
    case TOKlparen: {
      expr = ParseParenExpression();
      if (!expr) {
        return nullptr;
      }
      break;
    }
    default:
      return nullptr;
  }
  return ParsePostExpression(expr);
}

// Literal := String | Number | Null
CXFA_FMSimpleExpression* CXFA_FMParser::ParseLiteral() {
  switch (token_.GetType()) {
    case TOKnumber:
      return cppgc::MakeGarbageCollected<CXFA_FMNumberExpression>(
          heap_->GetAllocationHandle(), WideString(token_.GetString()));
    case TOKstring:
      return cppgc::MakeGarbageCollected<CXFA_FMStringExpression>(
          heap_->GetAllocationHandle(), WideString(token_.GetString()));
    case TOKnull:
      return cppgc::MakeGarbageCollected<CXFA_FMNullExpression>(
          heap_->GetAllocationHandle());
    default:
      return nullptr;
  }
}

// TODO(dsinclair): Make this match up to the grammar
// I believe this is parsing the accessor ( '.' | '..' | '.#' )
CXFA_FMSimpleExpression* CXFA_FMParser::ParsePostExpression(
    CXFA_FMSimpleExpression* expr) {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  size_t expr_count = 0;
  while (true) {
    ++expr_count;
    // Limit the number of expressions allowed in the post expression statement.
    // If we don't do this then its possible to generate a stack overflow
    // by having a very large number of things like .. expressions.
    if (expr_count > kMaxPostExpressions) {
      return nullptr;
    }

    switch (token_.GetType()) {
      case TOKlparen: {
        std::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
            expressions = ParseArgumentList();
        if (!expressions.has_value()) {
          return nullptr;
        }

        expr = cppgc::MakeGarbageCollected<CXFA_FMCallExpression>(
            heap_->GetAllocationHandle(), expr, std::move(expressions.value()),
            false);
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() != TOKlbracket) {
          continue;
        }

        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s) {
          return nullptr;
        }

        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            heap_->GetAllocationHandle(), expr, TOKcall, WideString(), s);
        break;
      }
      case TOKdot: {
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() != TOKidentifier) {
          return nullptr;
        }

        WideString tempStr(token_.GetString());
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() == TOKlparen) {
          std::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
              expressions = ParseArgumentList();
          if (!expressions.has_value()) {
            return nullptr;
          }

          auto* pIdentifier =
              cppgc::MakeGarbageCollected<CXFA_FMIdentifierExpression>(
                  heap_->GetAllocationHandle(), std::move(tempStr));
          auto* pExpCall = cppgc::MakeGarbageCollected<CXFA_FMCallExpression>(
              heap_->GetAllocationHandle(), pIdentifier,
              std::move(expressions.value()), true);
          expr = cppgc::MakeGarbageCollected<CXFA_FMMethodCallExpression>(
              heap_->GetAllocationHandle(), expr, pExpCall);
          if (!NextToken()) {
            return nullptr;
          }
          if (token_.GetType() != TOKlbracket) {
            continue;
          }

          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s) {
            return nullptr;
          }

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKcall, WideString(), s);
        } else if (token_.GetType() == TOKlbracket) {
          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s) {
            return nullptr;
          }

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKdot, std::move(tempStr),
              s);
        } else {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              heap_->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKdot, std::move(tempStr),
              subexpr);
          continue;
        }
        break;
      }
      case TOKdotdot: {
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() != TOKidentifier) {
          return nullptr;
        }

        WideString tempStr(token_.GetString());
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() == TOKlbracket) {
          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (!s) {
            return nullptr;
          }

          expr = cppgc::MakeGarbageCollected<CXFA_FMDotDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKdotdot, std::move(tempStr),
              s);
        } else {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              heap_->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKdotdot, std::move(tempStr),
              subexpr);
          continue;
        }
        break;
      }
      case TOKdotscream: {
        if (!NextToken()) {
          return nullptr;
        }
        if (token_.GetType() != TOKidentifier) {
          return nullptr;
        }

        WideString tempStr(token_.GetString());
        if (!NextToken()) {
          return nullptr;
        }

        if (token_.GetType() != TOKlbracket) {
          auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
              heap_->GetAllocationHandle(),
              CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
          expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
              heap_->GetAllocationHandle(), expr, TOKdotscream,
              std::move(tempStr), subexpr);
          continue;
        }

        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (!s) {
          return nullptr;
        }

        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            heap_->GetAllocationHandle(), expr, TOKdotscream,
            std::move(tempStr), s);
        break;
      }
      case TOKdotstar: {
        auto* subexpr = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
            heap_->GetAllocationHandle(),
            CXFA_FMIndexExpression::AccessorIndex::kNoIndex, nullptr, false);
        expr = cppgc::MakeGarbageCollected<CXFA_FMDotAccessorExpression>(
            heap_->GetAllocationHandle(), expr, TOKdotstar, L"*", subexpr);
        break;
      }
      default:
        return expr;
    }
    if (!NextToken()) {
      return nullptr;
    }
  }
}

// Argument lists are zero or more comma seperated simple expressions found
// between '(' and ')'
std::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
CXFA_FMParser::ParseArgumentList() {
  if (token_.GetType() != TOKlparen || !NextToken()) {
    return std::nullopt;
  }

  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> expressions;
  bool first_arg = true;
  while (token_.GetType() != TOKrparen) {
    if (first_arg) {
      first_arg = false;
    } else {
      if (token_.GetType() != TOKcomma || !NextToken()) {
        return std::nullopt;
      }
    }

    CXFA_FMSimpleExpression* exp = ParseSimpleExpression();
    if (!exp) {
      return std::nullopt;
    }

    expressions.push_back(exp);
    if (expressions.size() > kMaxPostExpressions) {
      return std::nullopt;
    }
  }

  return expressions;
}

// Index := '[' ('*' | '+' SimpleExpression | '-' SimpleExpression) ']'
CXFA_FMSimpleExpression* CXFA_FMParser::ParseIndexExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKlbracket)) {
    return nullptr;
  }

  if (token_.GetType() == TOKmul) {
    auto* pExp = cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
        heap_->GetAllocationHandle(),
        CXFA_FMIndexExpression::AccessorIndex::kNoRelativeIndex, nullptr, true);
    if (!pExp || !NextToken()) {
      return nullptr;
    }

    // TODO(dsinclair): This should CheckThenNext(TOKrbracket) but need to clean
    // up the callsites.
    if (token_.GetType() != TOKrbracket) {
      return nullptr;
    }
    return pExp;
  }

  CXFA_FMIndexExpression::AccessorIndex accessorIndex =
      CXFA_FMIndexExpression::AccessorIndex::kNoRelativeIndex;
  if (token_.GetType() == TOKplus) {
    accessorIndex = CXFA_FMIndexExpression::AccessorIndex::kPositiveIndex;
    if (!NextToken()) {
      return nullptr;
    }
  } else if (token_.GetType() == TOKminus) {
    accessorIndex = CXFA_FMIndexExpression::AccessorIndex::kNegativeIndex;
    if (!NextToken()) {
      return nullptr;
    }
  }

  CXFA_FMSimpleExpression* s = ParseSimpleExpression();
  if (!s) {
    return nullptr;
  }
  if (token_.GetType() != TOKrbracket) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMIndexExpression>(
      heap_->GetAllocationHandle(), accessorIndex, s, false);
}

// Paren := '(' SimpleExpression ')'
CXFA_FMSimpleExpression* CXFA_FMParser::ParseParenExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  if (!CheckThenNext(TOKlparen)) {
    return nullptr;
  }
  if (token_.GetType() == TOKrparen) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pExp1 = ParseSimpleExpression();
  if (!pExp1) {
    return nullptr;
  }

  if (!CheckThenNext(TOKrparen)) {
    return nullptr;
  }
  return pExp1;
}

// If := 'if' '(' SimpleExpression ')' 'then' ExpressionList
//       ('elseif' '(' SimpleExpression ')' 'then' ExpressionList)*
//       ('else' ExpressionList)?
//       'endif'
CXFA_FMExpression* CXFA_FMParser::ParseIfExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }

  if (!CheckThenNext(TOKif)) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pCondition = ParseParenExpression();
  if (!pCondition) {
    return nullptr;
  }
  if (!CheckThenNext(TOKthen)) {
    return nullptr;
  }

  auto* pIfExpressions = cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
      heap_->GetAllocationHandle(), ParseExpressionList());

  std::vector<cppgc::Member<CXFA_FMIfExpression>> pElseIfExpressions;
  while (token_.GetType() == TOKelseif) {
    if (!NextToken()) {
      return nullptr;
    }

    auto* elseIfCondition = ParseParenExpression();
    if (!elseIfCondition) {
      return nullptr;
    }
    if (!CheckThenNext(TOKthen)) {
      return nullptr;
    }

    auto elseIfExprs = ParseExpressionList();
    pElseIfExpressions.push_back(
        cppgc::MakeGarbageCollected<CXFA_FMIfExpression>(
            heap_->GetAllocationHandle(), elseIfCondition,
            cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
                heap_->GetAllocationHandle(), std::move(elseIfExprs)),
            std::vector<cppgc::Member<CXFA_FMIfExpression>>(), nullptr));
  }

  CXFA_FMExpression* pElseExpression = nullptr;
  if (token_.GetType() == TOKelse) {
    if (!NextToken()) {
      return nullptr;
    }

    pElseExpression = cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
        heap_->GetAllocationHandle(), ParseExpressionList());
  }
  if (!CheckThenNext(TOKendif)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMIfExpression>(
      heap_->GetAllocationHandle(), pCondition, pIfExpressions,
      std::move(pElseIfExpressions), pElseExpression);
}

// While := 'while' '(' SimpleExpression ')' 'do' ExpressionList 'endwhile'
CXFA_FMExpression* CXFA_FMParser::ParseWhileExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKwhile)) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pCondition = ParseParenExpression();
  if (!pCondition || !CheckThenNext(TOKdo)) {
    return nullptr;
  }

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendwhile)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMWhileExpression>(
      heap_->GetAllocationHandle(), pCondition,
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          heap_->GetAllocationHandle(), std::move(exprs)));
}

// For := 'for' Assignment 'upto' Accessor ('step' SimpleExpression)?
//            'do' ExpressionList 'endfor' |
//         'for' Assignment 'downto' Accessor ('step' SimpleExpression)?
//            'do' ExpressionList 'endfor'
CXFA_FMExpression* CXFA_FMParser::ParseForExpression() {
  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKfor)) {
    return nullptr;
  }
  if (token_.GetType() != TOKidentifier) {
    return nullptr;
  }

  WideString wsVariant(token_.GetString());
  if (!NextToken()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKassign)) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pAssignment = ParseSimpleExpression();
  if (!pAssignment) {
    return nullptr;
  }

  int32_t iDirection = 0;
  if (token_.GetType() == TOKupto) {
    iDirection = 1;
  } else if (token_.GetType() == TOKdownto) {
    iDirection = -1;
  } else {
    return nullptr;
  }

  if (!NextToken()) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pAccessor = ParseSimpleExpression();
  if (!pAccessor) {
    return nullptr;
  }

  CXFA_FMSimpleExpression* pStep = nullptr;
  if (token_.GetType() == TOKstep) {
    if (!NextToken()) {
      return nullptr;
    }
    pStep = ParseSimpleExpression();
    if (!pStep) {
      return nullptr;
    }
  }
  if (!CheckThenNext(TOKdo)) {
    return nullptr;
  }

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendfor)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMForExpression>(
      heap_->GetAllocationHandle(), wsVariant, pAssignment, pAccessor,
      iDirection, pStep,
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          heap_->GetAllocationHandle(), std::move(exprs)));
}

// Foreach := 'foreach' Identifier 'in' '(' ArgumentList ')'
//            'do' ExpressionList 'endfor'
CXFA_FMExpression* CXFA_FMParser::ParseForeachExpression() {
  if (token_.GetType() != TOKforeach) {
    return nullptr;
  }

  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKforeach)) {
    return nullptr;
  }
  if (token_.GetType() != TOKidentifier) {
    return nullptr;
  }

  WideString wsIdentifier(token_.GetString());
  if (!NextToken() || !CheckThenNext(TOKin) || !CheckThenNext(TOKlparen)) {
    return nullptr;
  }

  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> pArgumentList;
  while (token_.GetType() != TOKrparen) {
    CXFA_FMSimpleExpression* s = ParseSimpleExpression();
    if (!s) {
      return nullptr;
    }

    pArgumentList.push_back(s);
    if (token_.GetType() != TOKcomma) {
      break;
    }
    if (!NextToken()) {
      return nullptr;
    }
  }
  // We must have arguments.
  if (pArgumentList.empty()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKrparen)) {
    return nullptr;
  }

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKendfor)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMForeachExpression>(
      heap_->GetAllocationHandle(), std::move(wsIdentifier),
      std::move(pArgumentList),
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          heap_->GetAllocationHandle(), std::move(exprs)));
}

// Block := 'do' ExpressionList 'end'
CXFA_FMExpression* CXFA_FMParser::ParseDoExpression() {
  if (token_.GetType() != TOKdo) {
    return nullptr;
  }

  AutoRestorer<unsigned long> restorer(&parse_depth_);
  if (HasError() || !IncrementParseDepthAndCheck()) {
    return nullptr;
  }
  if (!CheckThenNext(TOKdo)) {
    return nullptr;
  }

  auto exprs = ParseExpressionList();
  if (!CheckThenNext(TOKend)) {
    return nullptr;
  }

  return cppgc::MakeGarbageCollected<CXFA_FMDoExpression>(
      heap_->GetAllocationHandle(),
      cppgc::MakeGarbageCollected<CXFA_FMBlockExpression>(
          heap_->GetAllocationHandle(), std::move(exprs)));
}

bool CXFA_FMParser::HasError() const {
  return error_ || token_.GetType() == TOKreserver;
}
