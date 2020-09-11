// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_
#define XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_

#include <memory>
#include <vector>

#include "third_party/base/optional.h"
#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"

class CFX_WideTextBuf;

enum XFA_FM_AccessorIndex {
  ACCESSOR_NO_INDEX,
  ACCESSOR_NO_RELATIVEINDEX,
  ACCESSOR_POSITIVE_INDEX,
  ACCESSOR_NEGATIVE_INDEX
};

enum class ReturnType { kImplied, kInfered };

class CXFA_FMExpression {
 public:
  virtual ~CXFA_FMExpression();
  virtual bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const = 0;

 protected:
  CXFA_FMExpression();
};

class CXFA_FMSimpleExpression : public CXFA_FMExpression {
 public:
  ~CXFA_FMSimpleExpression() override;

  XFA_FM_TOKEN GetOperatorToken() const { return m_op; }
  bool chainable() const { return m_bChainable; }

 protected:
  explicit CXFA_FMSimpleExpression(XFA_FM_TOKEN op);
  CXFA_FMSimpleExpression(XFA_FM_TOKEN op, bool chainable);

 private:
  const XFA_FM_TOKEN m_op;
  const bool m_bChainable;
};

class CXFA_FMChainableExpression : public CXFA_FMSimpleExpression {
 public:
  ~CXFA_FMChainableExpression() override;

 protected:
  CXFA_FMChainableExpression(XFA_FM_TOKEN op,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp2);

  CXFA_FMSimpleExpression* GetFirstExpression() const;
  CXFA_FMSimpleExpression* GetSecondExpression() const;

 private:
  // Iteratively delete a chainable expression tree in linear time and constant
  // space.
  static void DeleteChain(std::unique_ptr<CXFA_FMSimpleExpression> pRoot);

  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp1;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp2;
};

class CXFA_FMNullExpression final : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMNullExpression();
  ~CXFA_FMNullExpression() override = default;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;
};

class CXFA_FMNumberExpression final : public CXFA_FMSimpleExpression {
 public:
  explicit CXFA_FMNumberExpression(WideStringView wsNumber);
  ~CXFA_FMNumberExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView m_wsNumber;
};

class CXFA_FMStringExpression final : public CXFA_FMSimpleExpression {
 public:
  explicit CXFA_FMStringExpression(WideStringView wsString);
  ~CXFA_FMStringExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView m_wsString;
};

class CXFA_FMIdentifierExpression final : public CXFA_FMSimpleExpression {
 public:
  explicit CXFA_FMIdentifierExpression(WideStringView wsIdentifier);
  ~CXFA_FMIdentifierExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMAssignExpression final : public CXFA_FMChainableExpression {
 public:
  CXFA_FMAssignExpression(XFA_FM_TOKEN op,
                          std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                          std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMAssignExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;
};

class CXFA_FMBinExpression : public CXFA_FMChainableExpression {
 public:
  ~CXFA_FMBinExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 protected:
  CXFA_FMBinExpression(const WideString& opName,
                       XFA_FM_TOKEN op,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp2);

 private:
  WideString m_OpName;
};

class CXFA_FMLogicalOrExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalOrExpression(XFA_FM_TOKEN op,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                             std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLogicalOrExpression() override = default;
};

class CXFA_FMLogicalAndExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMLogicalAndExpression(XFA_FM_TOKEN op,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                              std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLogicalAndExpression() override = default;
};

class CXFA_FMEqualExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMEqualExpression(XFA_FM_TOKEN op,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMEqualExpression() override = default;
};

class CXFA_FMNotEqualExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMNotEqualExpression(XFA_FM_TOKEN op,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                            std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMNotEqualExpression() override = default;
};

class CXFA_FMGtExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMGtExpression(XFA_FM_TOKEN op,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMGtExpression() override = default;
};

class CXFA_FMGeExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMGeExpression(XFA_FM_TOKEN op,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMGeExpression() override = default;
};

class CXFA_FMLtExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMLtExpression(XFA_FM_TOKEN op,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLtExpression() override = default;
};

class CXFA_FMLeExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMLeExpression(XFA_FM_TOKEN op,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                      std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMLeExpression() override = default;
};

class CXFA_FMPlusExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMPlusExpression(XFA_FM_TOKEN op,
                        std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                        std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMPlusExpression() override = default;
};

class CXFA_FMMinusExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMMinusExpression(XFA_FM_TOKEN op,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMMinusExpression() override = default;
};

class CXFA_FMMulExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMMulExpression(XFA_FM_TOKEN op,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMMulExpression() override = default;
};

class CXFA_FMDivExpression final : public CXFA_FMBinExpression {
 public:
  CXFA_FMDivExpression(XFA_FM_TOKEN op,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
                       std::unique_ptr<CXFA_FMSimpleExpression> pExp2);
  ~CXFA_FMDivExpression() override = default;
};

class CXFA_FMUnaryExpression : public CXFA_FMSimpleExpression {
 public:
  ~CXFA_FMUnaryExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 protected:
  CXFA_FMUnaryExpression(const WideString& opName,
                         XFA_FM_TOKEN op,
                         std::unique_ptr<CXFA_FMSimpleExpression> pExp);

 private:
  WideString m_OpName;
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp;
};

class CXFA_FMPosExpression final : public CXFA_FMUnaryExpression {
 public:
  explicit CXFA_FMPosExpression(std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMPosExpression() override = default;
};

class CXFA_FMNegExpression final : public CXFA_FMUnaryExpression {
 public:
  explicit CXFA_FMNegExpression(std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMNegExpression() override = default;
};

class CXFA_FMNotExpression final : public CXFA_FMUnaryExpression {
 public:
  explicit CXFA_FMNotExpression(std::unique_ptr<CXFA_FMSimpleExpression> pExp);
  ~CXFA_FMNotExpression() override = default;
};

class CXFA_FMCallExpression final : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMCallExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pExp,
      std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pArguments,
      bool bIsSomMethod);
  ~CXFA_FMCallExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

  bool IsBuiltInFunc(CFX_WideTextBuf* funcName) const;
  uint32_t IsMethodWithObjParam(const WideString& methodName) const;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp;
  bool m_bIsSomMethod;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> m_Arguments;
};

class CXFA_FMDotAccessorExpression final : public CXFA_FMChainableExpression {
 public:
  CXFA_FMDotAccessorExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
      XFA_FM_TOKEN op,
      WideStringView wsIdentifier,
      std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp);
  ~CXFA_FMDotAccessorExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMIndexExpression final : public CXFA_FMSimpleExpression {
 public:
  CXFA_FMIndexExpression(XFA_FM_AccessorIndex accessorIndex,
                         std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp,
                         bool bIsStarIndex);
  ~CXFA_FMIndexExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> m_pExp;
  XFA_FM_AccessorIndex m_accessorIndex;
  bool m_bIsStarIndex;
};

class CXFA_FMDotDotAccessorExpression final
    : public CXFA_FMChainableExpression {
 public:
  CXFA_FMDotDotAccessorExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
      XFA_FM_TOKEN op,
      WideStringView wsIdentifier,
      std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp);
  ~CXFA_FMDotDotAccessorExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView m_wsIdentifier;
};

class CXFA_FMMethodCallExpression final : public CXFA_FMChainableExpression {
 public:
  CXFA_FMMethodCallExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pAccessorExp1,
      std::unique_ptr<CXFA_FMSimpleExpression> pCallExp);
  ~CXFA_FMMethodCallExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;
};

bool CXFA_IsTooBig(const CFX_WideTextBuf& js);

class CXFA_FMFunctionDefinition final : public CXFA_FMExpression {
 public:
  CXFA_FMFunctionDefinition(
      WideStringView wsName,
      std::vector<WideStringView>&& arguments,
      std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions);
  ~CXFA_FMFunctionDefinition() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  const WideStringView m_wsName;
  std::vector<WideStringView> const m_pArguments;
  std::vector<std::unique_ptr<CXFA_FMExpression>> const m_pExpressions;
};

class CXFA_FMVarExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMVarExpression(WideStringView wsName,
                       std::unique_ptr<CXFA_FMSimpleExpression> pInit);
  ~CXFA_FMVarExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  WideStringView const m_wsName;
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pInit;
};

class CXFA_FMExpExpression final : public CXFA_FMExpression {
 public:
  explicit CXFA_FMExpExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pExpression);
  ~CXFA_FMExpExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pExpression;
};

class CXFA_FMBlockExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMBlockExpression(
      std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList);
  ~CXFA_FMBlockExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::vector<std::unique_ptr<CXFA_FMExpression>> const m_ExpressionList;
};

class CXFA_FMDoExpression final : public CXFA_FMExpression {
 public:
  explicit CXFA_FMDoExpression(std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMDoExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::unique_ptr<CXFA_FMExpression> const m_pList;
};

class CXFA_FMIfExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMIfExpression(
      std::unique_ptr<CXFA_FMSimpleExpression> pExpression,
      std::unique_ptr<CXFA_FMExpression> pIfExpression,
      std::vector<std::unique_ptr<CXFA_FMIfExpression>> pElseIfExpressions,
      std::unique_ptr<CXFA_FMExpression> pElseExpression);
  ~CXFA_FMIfExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pExpression;
  std::unique_ptr<CXFA_FMExpression> const m_pIfExpression;
  std::vector<std::unique_ptr<CXFA_FMIfExpression>> const m_pElseIfExpressions;
  std::unique_ptr<CXFA_FMExpression> const m_pElseExpression;
};

class CXFA_FMWhileExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMWhileExpression(std::unique_ptr<CXFA_FMSimpleExpression> pCodition,
                         std::unique_ptr<CXFA_FMExpression> pExpression);
  ~CXFA_FMWhileExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pCondition;
  std::unique_ptr<CXFA_FMExpression> const m_pExpression;
};

class CXFA_FMBreakExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMBreakExpression();
  ~CXFA_FMBreakExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;
};

class CXFA_FMContinueExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMContinueExpression();
  ~CXFA_FMContinueExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;
};

class CXFA_FMForExpression final : public CXFA_FMExpression {
 public:
  CXFA_FMForExpression(WideStringView wsVariant,
                       std::unique_ptr<CXFA_FMSimpleExpression> pAssignment,
                       std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
                       int32_t iDirection,
                       std::unique_ptr<CXFA_FMSimpleExpression> pStep,
                       std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMForExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  const WideStringView m_wsVariant;
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pAssignment;
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pAccessor;
  const bool m_bDirection;
  std::unique_ptr<CXFA_FMSimpleExpression> const m_pStep;
  std::unique_ptr<CXFA_FMExpression> const m_pList;
};

class CXFA_FMForeachExpression final : public CXFA_FMExpression {
 public:
  // Takes ownership of |pAccessors|.
  CXFA_FMForeachExpression(
      WideStringView wsIdentifier,
      std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
      std::unique_ptr<CXFA_FMExpression> pList);
  ~CXFA_FMForeachExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  const WideStringView m_wsIdentifier;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> const m_pAccessors;
  std::unique_ptr<CXFA_FMExpression> const m_pList;
};

class CXFA_FMAST {
 public:
  explicit CXFA_FMAST(
      std::vector<std::unique_ptr<CXFA_FMExpression>> expressions);
  ~CXFA_FMAST();

  Optional<CFX_WideTextBuf> ToJavaScript() const;

 private:
  std::vector<std::unique_ptr<CXFA_FMExpression>> const expressions_;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMEXPRESSION_H_
