// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMSIMPLEEXPRESSION_H_
#define XFA_FXFA_FM2JS_CXFA_FMSIMPLEEXPRESSION_H_

#include <memory>
#include <vector>

#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"

#define RUNTIMEFUNCTIONRETURNVALUE L"pfm_ret"
#define EXCLAMATION_IN_IDENTIFIER L"pfm__excl__"

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

class CFX_WideTextBuf;

WideStringView XFA_FM_EXPTypeToString(
    XFA_FM_SimpleExpressionType simpleExpType);

enum XFA_FM_AccessorIndex {
  ACCESSOR_NO_INDEX,
  ACCESSOR_NO_RELATIVEINDEX,
  ACCESSOR_POSITIVE_INDEX,
  ACCESSOR_NEGATIVE_INDEX
};

class CXFA_FMSimpleExpression {
 public:
  CXFA_FMSimpleExpression(uint32_t line, XFA_FM_TOKEN op);
  virtual ~CXFA_FMSimpleExpression() {}
  virtual bool ToJavaScript(CFX_WideTextBuf& javascript);
  virtual bool ToImpliedReturnJS(CFX_WideTextBuf& javascript);

  XFA_FM_TOKEN GetOperatorToken() const;

 protected:
  uint32_t m_line;
  const XFA_FM_TOKEN m_op;
};

class CXFA_FMNullExpression : public CXFA_FMSimpleExpression {
 public:
  explicit CXFA_FMNullExpression(uint32_t line);
  ~CXFA_FMNullExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMNumberExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMNumberExpression(uint32_t line, WideStringView wsNumber);
  ~CXFA_FMNumberExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  WideStringView m_wsNumber;
};

class CXFA_FMStringExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMStringExpression(uint32_t line, WideStringView wsString);
  ~CXFA_FMStringExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  WideStringView m_wsString;
};

class CXFA_FMIdentifierExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMIdentifierExpression(uint32_t line, WideStringView wsIdentifier);
  ~CXFA_FMIdentifierExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMUnaryExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMUnaryExpression(uint32_t line,
                         XFA_FM_TOKEN op,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMUnaryExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 protected:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp;
};

class CXFA_FMBinExpression : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMBinExpression(uint32_t line,
                       XFA_FM_TOKEN op,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMBinExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 protected:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp1;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp2;
};

class CXFA_FMAssignExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMAssignExpression(uint32_t line,
                          XFA_FM_TOKEN op,
                          std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                          std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMAssignExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
  bool ToImpliedReturnJS(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMLogicalOrExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalOrExpression(uint32_t line,
                             XFA_FM_TOKEN op,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLogicalOrExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMLogicalAndExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalAndExpression(uint32_t line,
                              XFA_FM_TOKEN op,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLogicalAndExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMEqualityExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMEqualityExpression(uint32_t line,
                            XFA_FM_TOKEN op,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMEqualityExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMRelationalExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMRelationalExpression(uint32_t line,
                              XFA_FM_TOKEN op,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMRelationalExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMAdditiveExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMAdditiveExpression(uint32_t line,
                            XFA_FM_TOKEN op,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMAdditiveExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMMultiplicativeExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMMultiplicativeExpression(
      uint32_t line,
      XFA_FM_TOKEN op,
      std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
      std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMMultiplicativeExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMPosExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMPosExpression(uint32_t line,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMPosExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMNegExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMNegExpression(uint32_t line,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMNegExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMNotExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMNotExpression(uint32_t line,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMNotExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

class CXFA_FMCallExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMCallExpression(
      uint32_t line,
      std::unique_ptr<CXFA_FMSimpleExpression> pExp,
      std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pArguments,
      bool bIsSomMethod);
  ~CXFA_FMCallExpression() override;

  bool IsBuiltInFunc(CFX_WideTextBuf* funcName);
  uint32_t IsMethodWithObjParam(const WideString& methodName);
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  bool m_bIsSomMethod;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> m_Arguments;
};

class CXFA_FMDotAccessorExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMDotAccessorExpression(
      uint32_t line,
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
      XFA_FM_TOKEN op,
      WideStringView wsIdentifier,
      std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp);
  ~CXFA_FMDotAccessorExpression() override;
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMIndexExpression : public CXFA_FMUnaryExpression {
 public:
  CXFA_FMIndexExpression(uint32_t line,
                         XFA_FM_AccessorIndex accessorIndex,
                         std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp,
                         bool bIsStarIndex);
  ~CXFA_FMIndexExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  XFA_FM_AccessorIndex m_accessorIndex;
  bool m_bIsStarIndex;
};

class CXFA_FMDotDotAccessorExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMDotDotAccessorExpression(
      uint32_t line,
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
      XFA_FM_TOKEN op,
      WideStringView wsIdentifier,
      std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp);
  ~CXFA_FMDotDotAccessorExpression() override;

  bool ToJavaScript(CFX_WideTextBuf& javascript) override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMMethodCallExpression : public CXFA_FMBinExpression {
 public:
  CXFA_FMMethodCallExpression(
      uint32_t line,
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessorExp1,
      std::unique_ptr<CXFA_FMSimpleExpression> pCallExp);
  ~CXFA_FMMethodCallExpression() override {}
  bool ToJavaScript(CFX_WideTextBuf& javascript) override;
};

bool CXFA_IsTooBig(const CFX_WideTextBuf& javascript);

#endif  // XFA_FXFA_FM2JS_CXFA_FMSIMPLEEXPRESSION_H_
