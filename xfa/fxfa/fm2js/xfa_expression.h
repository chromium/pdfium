// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_XFA_EXPRESSION_H_
#define XFA_FXFA_FM2JS_XFA_EXPRESSION_H_

#include <memory>

#include "xfa/fxfa/fm2js/xfa_simpleexpression.h"

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

class CXFA_FMExpression {
 public:
  explicit CXFA_FMExpression(FX_DWORD line);
  CXFA_FMExpression(FX_DWORD line, XFA_FM_EXPTYPE type);
  virtual ~CXFA_FMExpression() {}
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);
  FX_DWORD GetLine() { return m_line; }
  XFA_FM_EXPTYPE GetExpType() const { return m_type; }

 protected:
  XFA_FM_EXPTYPE m_type;
  FX_DWORD m_line;
};

class CXFA_FMFunctionDefinition : public CXFA_FMExpression {
 public:
  CXFA_FMFunctionDefinition(FX_DWORD line,
                            FX_BOOL isGlobal,
                            const CFX_WideStringC& wsName,
                            CFX_WideStringCArray* pArguments,
                            CFX_PtrArray* pExpressions);
  ~CXFA_FMFunctionDefinition() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  CFX_WideStringC m_wsName;
  CFX_WideStringCArray* m_pArguments;
  CFX_PtrArray* m_pExpressions;
  FX_BOOL m_isGlobal;
};

class CXFA_FMVarExpression : public CXFA_FMExpression {
 public:
  CXFA_FMVarExpression(FX_DWORD line,
                       const CFX_WideStringC& wsName,
                       CXFA_FMExpression* pInit);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  CFX_WideStringC m_wsName;
  std::unique_ptr<CXFA_FMExpression> m_pInit;
};

class CXFA_FMExpExpression : public CXFA_FMExpression {
 public:
  CXFA_FMExpExpression(FX_DWORD line, CXFA_FMSimpleExpression* pExpression);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExpression;
};

class CXFA_FMBlockExpression : public CXFA_FMExpression {
 public:
  CXFA_FMBlockExpression(FX_DWORD line, CFX_PtrArray* pExpressionList);
  ~CXFA_FMBlockExpression() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  CFX_PtrArray* m_pExpressionList;
};

class CXFA_FMDoExpression : public CXFA_FMExpression {
 public:
  CXFA_FMDoExpression(FX_DWORD line, CXFA_FMExpression* pList);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

class CXFA_FMIfExpression : public CXFA_FMExpression {
 public:
  CXFA_FMIfExpression(FX_DWORD line,
                      CXFA_FMSimpleExpression* pExpression,
                      CXFA_FMExpression* pIfExpression,
                      CXFA_FMExpression* pElseExpression);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExpression;
  std::unique_ptr<CXFA_FMExpression> m_pIfExpression;
  std::unique_ptr<CXFA_FMExpression> m_pElseExpression;
};

class CXFA_FMLoopExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMLoopExpression(FX_DWORD line) : CXFA_FMExpression(line) {}
  ~CXFA_FMLoopExpression() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMWhileExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMWhileExpression(FX_DWORD line,
                         CXFA_FMSimpleExpression* pCodition,
                         CXFA_FMExpression* pExpression);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pCondition;
  std::unique_ptr<CXFA_FMExpression> m_pExpression;
};

class CXFA_FMBreakExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMBreakExpression(FX_DWORD line);
  ~CXFA_FMBreakExpression() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMContinueExpression : public CXFA_FMExpression {
 public:
  explicit CXFA_FMContinueExpression(FX_DWORD line);
  ~CXFA_FMContinueExpression() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;
};

class CXFA_FMForExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMForExpression(FX_DWORD line,
                       const CFX_WideStringC& wsVariant,
                       CXFA_FMSimpleExpression* pAssignment,
                       CXFA_FMSimpleExpression* pAccessor,
                       int32_t iDirection,
                       CXFA_FMSimpleExpression* pStep,
                       CXFA_FMExpression* pList);
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  CFX_WideStringC m_wsVariant;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pAssignment;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pAccessor;
  int32_t m_iDirection;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pStep;
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

class CXFA_FMForeachExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMForeachExpression(FX_DWORD line,
                           const CFX_WideStringC& wsIdentifier,
                           CFX_PtrArray* pAccessors,
                           CXFA_FMExpression* pList);
  ~CXFA_FMForeachExpression() override;
  void ToJavaScript(CFX_WideTextBuf& javascript) override;
  void ToImpliedReturnJS(CFX_WideTextBuf&) override;

 private:
  CFX_WideStringC m_wsIdentifier;
  CFX_PtrArray* m_pAccessors;
  std::unique_ptr<CXFA_FMExpression> m_pList;
};

#endif  // XFA_FXFA_FM2JS_XFA_EXPRESSION_H_
