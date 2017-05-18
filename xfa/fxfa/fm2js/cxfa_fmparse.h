// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMPARSE_H_
#define XFA_FXFA_FM2JS_CXFA_FMPARSE_H_

#include <memory>
#include <vector>

#include "xfa/fxfa/fm2js/cxfa_fmexpression.h"
#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"

class CXFA_FMParse {
 public:
  explicit CXFA_FMParse(const CFX_WideStringC& wsFormcalc);
  ~CXFA_FMParse();

  std::unique_ptr<CXFA_FMFunctionDefinition> Parse();
  bool HasError() const;

 private:
  void NextToken();
  void Check(XFA_FM_TOKEN op);

  std::vector<std::unique_ptr<CXFA_FMExpression>> ParseTopExpression();
  std::unique_ptr<CXFA_FMExpression> ParseFunction();
  std::unique_ptr<CXFA_FMExpression> ParseExpression();
  std::unique_ptr<CXFA_FMExpression> ParseVarExpression();
  std::unique_ptr<CXFA_FMExpression> ParseExpExpression();
  std::unique_ptr<CXFA_FMExpression> ParseBlockExpression();
  std::unique_ptr<CXFA_FMExpression> ParseIfExpression();
  std::unique_ptr<CXFA_FMExpression> ParseWhileExpression();
  std::unique_ptr<CXFA_FMExpression> ParseForExpression();
  std::unique_ptr<CXFA_FMExpression> ParseForeachExpression();
  std::unique_ptr<CXFA_FMExpression> ParseDoExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseParenExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseSimpleExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseSubassignmentInForExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseLogicalOrExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseLogicalAndExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseEqualityExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseRelationalExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseAddtiveExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseMultiplicativeExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParseUnaryExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParsePrimaryExpression();
  std::unique_ptr<CXFA_FMSimpleExpression> ParsePostExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> e);
  std::unique_ptr<CXFA_FMSimpleExpression> ParseIndexExpression();

  std::unique_ptr<CXFA_FMLexer> m_lexer;
  CXFA_FMToken* m_pToken;
  bool m_ParserError;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMPARSE_H_
