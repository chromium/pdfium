// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM_SIMPLEEXPRESSION_H
#define _XFA_FM_SIMPLEEXPRESSION_H
enum XFA_FM_SimpleExpressionType {
  ASSIGN,
  LOGICALOR,
  LOGICALAND,
  EQUALITY,
  NOTEQUALITY,
  LESS,
  LESSEQUAL,
  GREATER,
  GREATEREQUAL,
  PLUS,
  MINUS,
  MULTIPLE,
  DIVIDE,
  POSITIVE,
  NEGATIVE,
  NOT,
  CALL,
  DOT,
  DOTDOT,
  CONCATFMOBJECT,
  ISFMOBJECT,
  ISFMARRAY,
  GETFMVALUE,
  GETFMJSOBJ,
  VARFILTER
};
CFX_WideStringC XFA_FM_EXPTypeToString(
    XFA_FM_SimpleExpressionType simpleExpType);
struct XFA_FMBuildInFunc {
  uint32_t m_uHash;
  const FX_WCHAR* m_buildinfunc;
};
struct XFA_FMSOMMethod {
  uint32_t m_uHash;
  const FX_WCHAR* m_wsSomMethodName;
  FX_DWORD m_dParameters;
};
enum XFA_FM_AccessorIndex {
  ACCESSOR_NO_INDEX,
  ACCESSOR_NO_RELATIVEINDEX,
  ACCESSOR_POSITIVE_INDEX,
  ACCESSOR_NEGATIVE_INDEX
};
class CXFA_FMSimpleExpression {
 public:
  CXFA_FMSimpleExpression(FX_DWORD line, XFA_FM_TOKEN op);
  virtual ~CXFA_FMSimpleExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf& javascript);

  XFA_FM_TOKEN GetOperatorToken() const;

 protected:
  FX_DWORD m_line;
  XFA_FM_TOKEN m_op;
};
class CXFA_FMNullExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMNullExpression(FX_DWORD line);
  virtual ~CXFA_FMNullExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMNumberExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMNumberExpression(FX_DWORD line, CFX_WideStringC wsNumber);
  virtual ~CXFA_FMNumberExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CFX_WideStringC m_wsNumber;
};
class CXFA_FMStringExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMStringExpression(FX_DWORD line, CFX_WideStringC wsString);
  virtual ~CXFA_FMStringExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CFX_WideStringC m_wsString;
};
class CXFA_FMIdentifierExpressionn : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMIdentifierExpressionn(FX_DWORD line, CFX_WideStringC wsIdentifier);
  virtual ~CXFA_FMIdentifierExpressionn(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CFX_WideStringC m_wsIdentifier;
};
class CXFA_FMUnaryExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMUnaryExpression(FX_DWORD line,
                         XFA_FM_TOKEN op,
                         CXFA_FMSimpleExpression* pExp);
  virtual ~CXFA_FMUnaryExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CXFA_FMSimpleExpression* m_pExp;
};
class CXFA_FMBinExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMBinExpression(FX_DWORD line,
                       XFA_FM_TOKEN op,
                       CXFA_FMSimpleExpression* pExp1,
                       CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMBinExpression();
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CXFA_FMSimpleExpression* m_pExp1;
  CXFA_FMSimpleExpression* m_pExp2;
};
class CXFA_FMAssignExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMAssignExpression(FX_DWORD line,
                          XFA_FM_TOKEN op,
                          CXFA_FMSimpleExpression* pExp1,
                          CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMAssignExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
  virtual void ToImpliedReturnJS(CFX_WideTextBuf& javascript);
};
class CXFA_FMLogicalOrExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalOrExpression(FX_DWORD line,
                             XFA_FM_TOKEN op,
                             CXFA_FMSimpleExpression* pExp1,
                             CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMLogicalOrExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMLogicalAndExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalAndExpression(FX_DWORD line,
                              XFA_FM_TOKEN op,
                              CXFA_FMSimpleExpression* pExp1,
                              CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMLogicalAndExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMEqualityExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMEqualityExpression(FX_DWORD line,
                            XFA_FM_TOKEN op,
                            CXFA_FMSimpleExpression* pExp1,
                            CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMEqualityExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMRelationalExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMRelationalExpression(FX_DWORD line,
                              XFA_FM_TOKEN op,
                              CXFA_FMSimpleExpression* pExp1,
                              CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMRelationalExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMAdditiveExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMAdditiveExpression(FX_DWORD line,
                            XFA_FM_TOKEN op,
                            CXFA_FMSimpleExpression* pExp1,
                            CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMAdditiveExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMMultiplicativeExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMMultiplicativeExpression(FX_DWORD line,
                                  XFA_FM_TOKEN op,
                                  CXFA_FMSimpleExpression* pExp1,
                                  CXFA_FMSimpleExpression* pExp2);
  virtual ~CXFA_FMMultiplicativeExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMPosExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMPosExpression(FX_DWORD line, CXFA_FMSimpleExpression* pExp);
  virtual ~CXFA_FMPosExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMNegExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMNegExpression(FX_DWORD line, CXFA_FMSimpleExpression* pExp);
  virtual ~CXFA_FMNegExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMNotExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMNotExpression(FX_DWORD line, CXFA_FMSimpleExpression* pExp);
  virtual ~CXFA_FMNotExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);
};
class CXFA_FMCallExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMCallExpression(FX_DWORD line,
                        CXFA_FMSimpleExpression* pExp,
                        CFX_PtrArray* pArguments,
                        FX_BOOL bIsSomMethod);
  virtual ~CXFA_FMCallExpression();
  virtual FX_BOOL IsBuildInFunc(CFX_WideTextBuf& funcName);
  virtual FX_DWORD IsSomMethodWithObjPara(const CFX_WideStringC& methodName);
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 private:
  FX_BOOL m_bIsSomMethod;
  CFX_PtrArray* m_pArguments;
};
class CXFA_FMDotAccessorExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMDotAccessorExpression(FX_DWORD line,
                               CXFA_FMSimpleExpression* pAccessor,
                               XFA_FM_TOKEN op,
                               CFX_WideStringC wsIdentifier,
                               CXFA_FMSimpleExpression* pIndexExp);
  virtual ~CXFA_FMDotAccessorExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CFX_WideStringC m_wsIdentifier;
};
class CXFA_FMIndexExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMIndexExpression(FX_DWORD line,
                         XFA_FM_AccessorIndex accessorIndex,
                         CXFA_FMSimpleExpression* pIndexExp,
                         FX_BOOL bIsStarIndex);
  virtual ~CXFA_FMIndexExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  XFA_FM_AccessorIndex m_accessorIndex;
  FX_BOOL m_bIsStarIndex;
};
class CXFA_FMDotDotAccessorExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMDotDotAccessorExpression(FX_DWORD line,
                                  CXFA_FMSimpleExpression* pAccessor,
                                  XFA_FM_TOKEN op,
                                  CFX_WideStringC wsIdentifier,
                                  CXFA_FMSimpleExpression* pIndexExp);
  virtual ~CXFA_FMDotDotAccessorExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
  CFX_WideStringC m_wsIdentifier;
};
class CXFA_FMMethodCallExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMMethodCallExpression(FX_DWORD line,
                              CXFA_FMSimpleExpression* pAccessorExp1,
                              CXFA_FMSimpleExpression* pCallExp);
  virtual ~CXFA_FMMethodCallExpression(){};
  virtual void ToJavaScript(CFX_WideTextBuf& javascript);

 protected:
};
#endif
