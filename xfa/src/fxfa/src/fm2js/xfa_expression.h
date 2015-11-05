// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM_EXPRESSION_H
#define _XFA_FM_EXPRESSION_H
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
  CXFA_FMExpression(FX_DWORD line);
  CXFA_FMExpression(FX_DWORD line, XFA_FM_EXPTYPE type);
  virtual ~CXFA_FMExpression(){};
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
  virtual ~CXFA_FMFunctionDefinition();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

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
  virtual ~CXFA_FMVarExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CFX_WideStringC m_wsName;
  CXFA_FMExpression* m_pInit;
};
class CXFA_FMExpExpression : public CXFA_FMExpression {
 public:
  CXFA_FMExpExpression(FX_DWORD line, CXFA_FMSimpleExpression* pExpression);
  virtual ~CXFA_FMExpExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CXFA_FMSimpleExpression* m_pExpression;
};
class CXFA_FMBlockExpression : public CXFA_FMExpression {
 public:
  CXFA_FMBlockExpression(FX_DWORD line, CFX_PtrArray* pExpressionList);
  virtual ~CXFA_FMBlockExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CFX_PtrArray* m_pExpressionList;
};
class CXFA_FMDoExpression : public CXFA_FMExpression {
 public:
  CXFA_FMDoExpression(FX_DWORD line, CXFA_FMExpression* pList);
  virtual ~CXFA_FMDoExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CXFA_FMExpression* m_pList;
};
class CXFA_FMIfExpression : public CXFA_FMExpression {
 public:
  CXFA_FMIfExpression(FX_DWORD line,
                      CXFA_FMSimpleExpression* pExpression,
                      CXFA_FMExpression* pIfExpression,
                      CXFA_FMExpression* pElseExpression);
  virtual ~CXFA_FMIfExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CXFA_FMSimpleExpression* m_pExpression;
  CXFA_FMExpression* m_pIfExpression;
  CXFA_FMExpression* m_pElseExpression;
};
class CXFA_FMLoopExpression : public CXFA_FMExpression {
 public:
  CXFA_FMLoopExpression(FX_DWORD line) : CXFA_FMExpression(line) {}
  virtual ~CXFA_FMLoopExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);
};
class CXFA_FMWhileExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMWhileExpression(FX_DWORD line,
                         CXFA_FMSimpleExpression* pCodition,
                         CXFA_FMExpression* pExpression);
  virtual ~CXFA_FMWhileExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CXFA_FMSimpleExpression* m_pCondition;
  CXFA_FMExpression* m_pExpression;
};
class CXFA_FMBreakExpression : public CXFA_FMExpression {
 public:
  CXFA_FMBreakExpression(FX_DWORD line);
  virtual ~CXFA_FMBreakExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);
};
class CXFA_FMContinueExpression : public CXFA_FMExpression {
 public:
  CXFA_FMContinueExpression(FX_DWORD line);
  virtual ~CXFA_FMContinueExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);
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
  virtual ~CXFA_FMForExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CFX_WideStringC m_wsVariant;
  CXFA_FMSimpleExpression* m_pAssignment;
  CXFA_FMSimpleExpression* m_pAccessor;
  int32_t m_iDirection;
  CXFA_FMSimpleExpression* m_pStep;
  CXFA_FMExpression* m_pList;
};
class CXFA_FMForeachExpression : public CXFA_FMLoopExpression {
 public:
  CXFA_FMForeachExpression(FX_DWORD line,
                           const CFX_WideStringC& wsIdentifier,
                           CFX_PtrArray* pAccessors,
                           CXFA_FMExpression* pList);
  virtual ~CXFA_FMForeachExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf&);

 private:
  CFX_WideStringC m_wsIdentifier;
  CFX_PtrArray* m_pAccessors;
  CXFA_FMExpression* m_pList;
};
#endif
