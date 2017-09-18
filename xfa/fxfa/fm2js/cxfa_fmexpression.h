// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_
#define XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_

#include <memory>
#include <vector>

#include "xfa/fxfa/fm2js/cxfa_fmsimpleexpression.h"

enum XFA_FM_EXPTYPE {
  XFA_FM_EXPTYPE_UNKNOWN,
  XFA_FM_EXPTYPE_FUNC,
  XFA_FM_EXPTYPE_VAR,
  XFA_FM_EXPTYPE_EXP,
  XFA_FM_EXPTYPE_BLOCK,
  XFA_FM_EXPTYPE_IF,
  XFA_FM_EXPTYPE_BREAK,
  XFA_FM_EXPTYPE_CONTINUE,
};

class CFX_WideTextBuf;

class CXFA_FMExpression {
 public:
  explicit CXFA_FMExpression(uint32_t line);
  CXFA_FMExpression(uint32_t line, XFA_FM_EXPTYPE type);
  virtual ~CXFA_FMExpression() {}

  virtual bool ToJavaScript(CFX_WideTextBuf& javascript);
  virtual bool ToImpliedReturnJS(CFX_WideTextBuf&);
  uint32_t GetLine() { return m_line; }
  XFA_FM_EXPTYPE GetExpType() const { return m_type; }

 private:
  XFA_FM_EXPTYPE m_type;
  uint32_t m_line;
};

class CXFA_FMFunctionDefinition : public CXFA_FMExpression {
 public:
  // Takes ownership of |arguments| and |expressions|.
  CXFA_FMFunctionDefinition(
      uint32_t line,
      bool isGlobal,
      const WideStringView& wsName,
      std::vector<WideStringView>&& arguments,
      std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions);
  ~CXFA_FMFunctionDefinition() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  WideStringView m_wsName;
  std::vector<WideStringView> m_pArguments;
  std::vector<std::unique_ptr<CXFA_FMExpression>> m_pExpressions;
  bool m_isGlobal;
};

class CXFA_FMVarExpression : public CXFA_FMExpression {
 public:
  CXFA_FMVarExpression(uint32_t line,
                       const WideStringView& wsName,
                       std::unique_ptr<CXFA_FMExpression> pInit);
  ~CXFA_FMVarExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  WideStringView m_wsName;
  std::unique_ptr<CXFA_FMExpression> m_pInit;
};

class CXFA_FMExpExpression : public CXFA_FMExpression {
 public:
  CXFA_FMExpExpression(uint32_t line,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExpression);
  ~CXFA_FMExpExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExpression;
};

class CXFA_FMBlockExpression : public CXFA_FMExpression {
 public:
  CXFA_FMBlockExpression(
      uint32_t line,
      std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList);
  ~CXFA_FMBlockExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::vector<std::unique_ptr<CXFA_FMExpression>> m_ExpressionList;
};

class CXFA_FMDoExpression : public CXFA_FMExpression {
 public:
  CXFA_FMDoExpression(uint32_t line, std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMDoExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

class CXFA_FMIfExpression : public CXFA_FMExpression {
 public:
  CXFA_FMIfExpression(uint32_t line,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExpression,
                      std::unique_ptr<CXFA_FMExpression> pIfExpression,
                      std::unique_ptr<CXFA_FMExpression> pElseExpression);
  ~CXFA_FMIfExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExpression;
  std::unique_ptr<CXFA_FMExpression> m_pIfExpression;
  std::unique_ptr<CXFA_FMExpression> m_pElseExpression;
};

class CXFA_FMLoopExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMLoopExpression(uint32_t line) : CXFA_FMExpression(line) {}
  ~CXFA_FMLoopExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMWhileExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMWhileExpression(uint32_t line,
                         std::unique_ptr<CXFA_FMSimpleExpression> pCodition,
                         std::unique_ptr<CXFA_FMExpression> pExpression);
  ~CXFA_FMWhileExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pCondition;
  std::unique_ptr<CXFA_FMExpression> m_pExpression;
};

class CXFA_FMBreakExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMBreakExpression(uint32_t line);
  ~CXFA_FMBreakExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMContinueExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMContinueExpression(uint32_t line);
  ~CXFA_FMContinueExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMForExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMForExpression(uint32_t line,
                       const WideStringView& wsVariant,
                       std::unique_ptr<CXFA_FMSimpleExpression> pAssignment,
                       std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
                       int32_t iDirection,
                       std::unique_ptr<CXFA_FMSimpleExpression> pStep,
                       std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMForExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  WideStringView m_wsVariant;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pAssignment;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pAccessor;
  const bool m_bDirection;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pStep;
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

class CXFA_FMForeachExpression : public CXFA_FMLoopExpression {
 public:
  // Takes ownership of |pAccessors|.
  CXFA_FMForeachExpression(
      uint32_t line,
      const WideStringView& wsIdentifier,
      std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
      std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMForeachExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  WideStringView m_wsIdentifier;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> m_pAccessors;
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_
