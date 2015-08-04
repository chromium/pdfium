// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM_PARSE_H
#define _XFA_FM_PARSE_H
class CXFA_FMParse {
 public:
  CXFA_FMParse();
  ~CXFA_FMParse();
  int32_t Init(const CFX_WideStringC& wsFormcalc, CXFA_FMErrorInfo* pErrorInfo);
  void NextToken();
  void Check(XFA_FM_TOKEN op);
  void Error(FX_DWORD lineNum, XFA_FM_ERRMSG msg, ...);
  CFX_PtrArray* ParseTopExpression();
  CXFA_FMExpression* ParseFunction();
  CXFA_FMExpression* ParseExpression();
  CXFA_FMExpression* ParseVarExpression();
  CXFA_FMExpression* ParseExpExpression();
  CXFA_FMExpression* ParseBlockExpression();
  CXFA_FMExpression* ParseIfExpression();
  CXFA_FMExpression* ParseWhileExpression();
  CXFA_FMExpression* ParseForExpression();
  CXFA_FMExpression* ParseForeachExpression();
  CXFA_FMExpression* ParseDoExpression();
  CXFA_FMSimpleExpression* ParseParenExpression();
  CXFA_FMSimpleExpression* ParseSimpleExpression();
  CXFA_FMSimpleExpression* ParseSubassignmentInForExpression();
  CXFA_FMSimpleExpression* ParseLogicalOrExpression();
  CXFA_FMSimpleExpression* ParseLogicalAndExpression();
  CXFA_FMSimpleExpression* ParseEqualityExpression();
  CXFA_FMSimpleExpression* ParseRelationalExpression();
  CXFA_FMSimpleExpression* ParseAddtiveExpression();
  CXFA_FMSimpleExpression* ParseMultiplicativeExpression();
  CXFA_FMSimpleExpression* ParseUnaryExpression();
  CXFA_FMSimpleExpression* ParsePrimaryExpression();
  CXFA_FMSimpleExpression* ParsePostExpression(CXFA_FMSimpleExpression* e);
  CXFA_FMSimpleExpression* ParseIndexExpression();

 private:
  const FX_WCHAR* m_pScript;
  FX_STRSIZE m_uLength;
  CXFA_FMLexer* m_lexer;
  CXFA_FMToken* m_pToken;
  CXFA_FMErrorInfo* m_pErrorInfo;
};
#endif
