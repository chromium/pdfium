// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FORMCALC_CXFA_FMPARSER_H_
#define XFA_FXFA_FORMCALC_CXFA_FMPARSER_H_

#include <optional>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/macros.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/formcalc/cxfa_fmexpression.h"
#include "xfa/fxfa/formcalc/cxfa_fmlexer.h"

class CXFA_FMParser {
  CPPGC_STACK_ALLOCATED();  // Allow Raw/Unowned pointers.

 public:
  CXFA_FMParser(cppgc::Heap* heap, CXFA_FMLexer* pLexer);
  ~CXFA_FMParser();

  // Returned object is owned by cppgc heap.
  CXFA_FMAST* Parse();
  bool HasError() const;

  void SetMaxParseDepthForTest(unsigned long max_depth) {
    max_parse_depth_ = max_depth;
  }

 private:
  bool NextToken();
  bool CheckThenNext(XFA_FM_TOKEN op);
  bool IncrementParseDepthAndCheck();

  std::vector<cppgc::Member<CXFA_FMExpression>> ParseExpressionList();
  CXFA_FMExpression* ParseFunction();
  CXFA_FMExpression* ParseExpression();
  CXFA_FMExpression* ParseDeclarationExpression();
  CXFA_FMExpression* ParseExpExpression();
  CXFA_FMExpression* ParseIfExpression();
  CXFA_FMExpression* ParseWhileExpression();
  CXFA_FMExpression* ParseForExpression();
  CXFA_FMExpression* ParseForeachExpression();
  CXFA_FMExpression* ParseDoExpression();
  CXFA_FMSimpleExpression* ParseParenExpression();
  CXFA_FMSimpleExpression* ParseSimpleExpression();
  CXFA_FMSimpleExpression* ParseLogicalOrExpression();
  CXFA_FMSimpleExpression* ParseLogicalAndExpression();
  CXFA_FMSimpleExpression* ParseEqualityExpression();
  CXFA_FMSimpleExpression* ParseRelationalExpression();
  CXFA_FMSimpleExpression* ParseAdditiveExpression();
  CXFA_FMSimpleExpression* ParseMultiplicativeExpression();
  CXFA_FMSimpleExpression* ParseUnaryExpression();
  CXFA_FMSimpleExpression* ParsePrimaryExpression();
  CXFA_FMSimpleExpression* ParsePostExpression(CXFA_FMSimpleExpression* e);
  CXFA_FMSimpleExpression* ParseIndexExpression();
  CXFA_FMSimpleExpression* ParseLiteral();
  std::optional<std::vector<cppgc::Member<CXFA_FMSimpleExpression>>>
  ParseArgumentList();

  UnownedPtr<cppgc::Heap> const heap_;
  UNOWNED_PTR_EXCLUSION CXFA_FMLexer* const lexer_;  // Stack allocated.
  CXFA_FMLexer::Token token_;
  bool error_ = false;
  unsigned long parse_depth_ = 0;
  unsigned long max_parse_depth_;
};

#endif  // XFA_FXFA_FORMCALC_CXFA_FMPARSER_H_
