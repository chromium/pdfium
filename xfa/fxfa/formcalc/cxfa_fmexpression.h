// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FORMCALC_CXFA_FMEXPRESSION_H_
#define XFA_FXFA_FORMCALC_CXFA_FMEXPRESSION_H_

#include <vector>

#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fxfa/formcalc/cxfa_fmlexer.h"

class CFX_WideTextBuf;

class CXFA_FMExpression : public cppgc::GarbageCollected<CXFA_FMExpression> {
 public:
  enum class ReturnType { kImplied, kInferred };

  virtual ~CXFA_FMExpression();
  virtual void Trace(cppgc::Visitor* visitor) const;

  virtual bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const = 0;

 protected:
  CXFA_FMExpression();
};

class CXFA_FMSimpleExpression : public CXFA_FMExpression {
 public:
  ~CXFA_FMSimpleExpression() override;

  XFA_FM_TOKEN GetOperatorToken() const { return m_op; }

 protected:
  explicit CXFA_FMSimpleExpression(XFA_FM_TOKEN op);

 private:
  const XFA_FM_TOKEN m_op;
};

class CXFA_FMChainableExpression : public CXFA_FMSimpleExpression {
 public:
  ~CXFA_FMChainableExpression() override;
  void Trace(cppgc::Visitor* visitor) const override;

 protected:
  CXFA_FMChainableExpression(XFA_FM_TOKEN op,
                             CXFA_FMSimpleExpression* pExp1,
                             CXFA_FMSimpleExpression* pExp2);

  CXFA_FMSimpleExpression* GetFirstExpression() const { return m_pExp1; }
  CXFA_FMSimpleExpression* GetSecondExpression() const { return m_pExp2; }

 private:
  cppgc::Member<CXFA_FMSimpleExpression> m_pExp1;
  cppgc::Member<CXFA_FMSimpleExpression> m_pExp2;
};

class CXFA_FMNullExpression final : public CXFA_FMSimpleExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMNullExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMNullExpression();
};

class CXFA_FMNumberExpression final : public CXFA_FMSimpleExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMNumberExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  explicit CXFA_FMNumberExpression(WideString wsNumber);

  WideString m_wsNumber;
};

class CXFA_FMStringExpression final : public CXFA_FMSimpleExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMStringExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  explicit CXFA_FMStringExpression(WideString wsString);

  WideString m_wsString;
};

class CXFA_FMIdentifierExpression final : public CXFA_FMSimpleExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMIdentifierExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  explicit CXFA_FMIdentifierExpression(WideString wsIdentifier);

  WideString m_wsIdentifier;
};

class CXFA_FMAssignExpression final : public CXFA_FMChainableExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMAssignExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMAssignExpression(XFA_FM_TOKEN op,
                          CXFA_FMSimpleExpression* pExp1,
                          CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMBinExpression : public CXFA_FMChainableExpression {
 public:
  ~CXFA_FMBinExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 protected:
  CXFA_FMBinExpression(const WideString& opName,
                       XFA_FM_TOKEN op,
                       CXFA_FMSimpleExpression* pExp1,
                       CXFA_FMSimpleExpression* pExp2);

 private:
  WideString m_OpName;
};

class CXFA_FMLogicalOrExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMLogicalOrExpression() override;

 private:
  CXFA_FMLogicalOrExpression(XFA_FM_TOKEN op,
                             CXFA_FMSimpleExpression* pExp1,
                             CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMLogicalAndExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMLogicalAndExpression() override;

 private:
  CXFA_FMLogicalAndExpression(XFA_FM_TOKEN op,
                              CXFA_FMSimpleExpression* pExp1,
                              CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMEqualExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMEqualExpression() override;

 private:
  CXFA_FMEqualExpression(XFA_FM_TOKEN op,
                         CXFA_FMSimpleExpression* pExp1,
                         CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMNotEqualExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMNotEqualExpression() override;

 private:
  CXFA_FMNotEqualExpression(XFA_FM_TOKEN op,
                            CXFA_FMSimpleExpression* pExp1,
                            CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMGtExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMGtExpression() override;

 private:
  CXFA_FMGtExpression(XFA_FM_TOKEN op,
                      CXFA_FMSimpleExpression* pExp1,
                      CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMGeExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMGeExpression() override;

 private:
  CXFA_FMGeExpression(XFA_FM_TOKEN op,
                      CXFA_FMSimpleExpression* pExp1,
                      CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMLtExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMLtExpression() override;

 private:
  CXFA_FMLtExpression(XFA_FM_TOKEN op,
                      CXFA_FMSimpleExpression* pExp1,
                      CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMLeExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMLeExpression() override;

 private:
  CXFA_FMLeExpression(XFA_FM_TOKEN op,
                      CXFA_FMSimpleExpression* pExp1,
                      CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMPlusExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMPlusExpression() override;

 private:
  CXFA_FMPlusExpression(XFA_FM_TOKEN op,
                        CXFA_FMSimpleExpression* pExp1,
                        CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMMinusExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMMinusExpression() override;

 private:
  CXFA_FMMinusExpression(XFA_FM_TOKEN op,
                         CXFA_FMSimpleExpression* pExp1,
                         CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMMulExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMMulExpression() override;

 private:
  CXFA_FMMulExpression(XFA_FM_TOKEN op,
                       CXFA_FMSimpleExpression* pExp1,
                       CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMDivExpression final : public CXFA_FMBinExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMDivExpression() override;

 private:
  CXFA_FMDivExpression(XFA_FM_TOKEN op,
                       CXFA_FMSimpleExpression* pExp1,
                       CXFA_FMSimpleExpression* pExp2);
};

class CXFA_FMUnaryExpression : public CXFA_FMSimpleExpression {
 public:
  ~CXFA_FMUnaryExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 protected:
  CXFA_FMUnaryExpression(const WideString& opName,
                         XFA_FM_TOKEN op,
                         CXFA_FMSimpleExpression* pExp);

 private:
  WideString m_OpName;
  cppgc::Member<CXFA_FMSimpleExpression> m_pExp;
};

class CXFA_FMPosExpression final : public CXFA_FMUnaryExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMPosExpression() override;

 private:
  explicit CXFA_FMPosExpression(CXFA_FMSimpleExpression* pExp);
};

class CXFA_FMNegExpression final : public CXFA_FMUnaryExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMNegExpression() override;

 private:
  explicit CXFA_FMNegExpression(CXFA_FMSimpleExpression* pExp);
};

class CXFA_FMNotExpression final : public CXFA_FMUnaryExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMNotExpression() override;

 private:
  explicit CXFA_FMNotExpression(CXFA_FMSimpleExpression* pExp);
};

class CXFA_FMCallExpression final : public CXFA_FMSimpleExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMCallExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

  bool IsBuiltInFunc(CFX_WideTextBuf* funcName) const;
  uint32_t IsMethodWithObjParam(const WideString& methodName) const;

 private:
  CXFA_FMCallExpression(
      CXFA_FMSimpleExpression* pExp,
      std::vector<cppgc::Member<CXFA_FMSimpleExpression>>&& pArguments,
      bool bIsSomMethod);

  cppgc::Member<CXFA_FMSimpleExpression> m_pExp;
  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> m_Arguments;
  bool m_bIsSomMethod;
};

class CXFA_FMDotAccessorExpression final : public CXFA_FMChainableExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMDotAccessorExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMDotAccessorExpression(CXFA_FMSimpleExpression* pAccessor,
                               XFA_FM_TOKEN op,
                               WideString wsIdentifier,
                               CXFA_FMSimpleExpression* pIndexExp);

  WideString m_wsIdentifier;
};

class CXFA_FMIndexExpression final : public CXFA_FMSimpleExpression {
 public:
  enum class AccessorIndex : uint8_t {
    kNoIndex,
    kNoRelativeIndex,
    kPositiveIndex,
    kNegativeIndex
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMIndexExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMIndexExpression(AccessorIndex accessorIndex,
                         CXFA_FMSimpleExpression* pIndexExp,
                         bool bIsStarIndex);

  cppgc::Member<CXFA_FMSimpleExpression> m_pExp;
  AccessorIndex m_accessorIndex;
  bool m_bIsStarIndex;
};

class CXFA_FMDotDotAccessorExpression final
    : public CXFA_FMChainableExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMDotDotAccessorExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMDotDotAccessorExpression(CXFA_FMSimpleExpression* pAccessor,
                                  XFA_FM_TOKEN op,
                                  WideString wsIdentifier,
                                  CXFA_FMSimpleExpression* pIndexExp);

  WideString m_wsIdentifier;
};

class CXFA_FMMethodCallExpression final : public CXFA_FMChainableExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMMethodCallExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMMethodCallExpression(CXFA_FMSimpleExpression* pAccessorExp1,
                              CXFA_FMSimpleExpression* pCallExp);
};

class CXFA_FMFunctionDefinition final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMFunctionDefinition() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMFunctionDefinition(
      WideString wsName,
      std::vector<WideString>&& arguments,
      std::vector<cppgc::Member<CXFA_FMExpression>>&& expressions);

  const WideString m_wsName;
  std::vector<WideString> const m_pArguments;
  std::vector<cppgc::Member<CXFA_FMExpression>> const m_pExpressions;
};

class CXFA_FMVarExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMVarExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMVarExpression(WideString wsName, CXFA_FMSimpleExpression* pInit);

  WideString const m_wsName;
  cppgc::Member<CXFA_FMSimpleExpression> const m_pInit;
};

class CXFA_FMExpExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMExpExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  explicit CXFA_FMExpExpression(CXFA_FMSimpleExpression* pExpression);

  cppgc::Member<CXFA_FMSimpleExpression> const m_pExpression;
};

class CXFA_FMBlockExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMBlockExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMBlockExpression(
      std::vector<cppgc::Member<CXFA_FMExpression>>&& pExpressionList);

  std::vector<cppgc::Member<CXFA_FMExpression>> const m_ExpressionList;
};

class CXFA_FMDoExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMDoExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  explicit CXFA_FMDoExpression(CXFA_FMExpression* pList);

  cppgc::Member<CXFA_FMExpression> const m_pList;
};

class CXFA_FMIfExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMIfExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMIfExpression(
      CXFA_FMSimpleExpression* pExpression,
      CXFA_FMExpression* pIfExpression,
      std::vector<cppgc::Member<CXFA_FMIfExpression>>&& pElseIfExpressions,
      CXFA_FMExpression* pElseExpression);

  cppgc::Member<CXFA_FMSimpleExpression> const m_pExpression;
  cppgc::Member<CXFA_FMExpression> const m_pIfExpression;
  std::vector<cppgc::Member<CXFA_FMIfExpression>> const m_pElseIfExpressions;
  cppgc::Member<CXFA_FMExpression> const m_pElseExpression;
};

class CXFA_FMWhileExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMWhileExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMWhileExpression(CXFA_FMSimpleExpression* pCodition,
                         CXFA_FMExpression* pExpression);

  cppgc::Member<CXFA_FMSimpleExpression> const m_pCondition;
  cppgc::Member<CXFA_FMExpression> const m_pExpression;
};

class CXFA_FMBreakExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMBreakExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMBreakExpression();
};

class CXFA_FMContinueExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMContinueExpression() override;

  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMContinueExpression();
};

class CXFA_FMForExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMForExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  CXFA_FMForExpression(WideString wsVariant,
                       CXFA_FMSimpleExpression* pAssignment,
                       CXFA_FMSimpleExpression* pAccessor,
                       int32_t iDirection,
                       CXFA_FMSimpleExpression* pStep,
                       CXFA_FMExpression* pList);

  const WideString m_wsVariant;
  const bool m_bDirection;
  cppgc::Member<CXFA_FMSimpleExpression> const m_pAssignment;
  cppgc::Member<CXFA_FMSimpleExpression> const m_pAccessor;
  cppgc::Member<CXFA_FMSimpleExpression> const m_pStep;
  cppgc::Member<CXFA_FMExpression> const m_pList;
};

class CXFA_FMForeachExpression final : public CXFA_FMExpression {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMForeachExpression() override;

  void Trace(cppgc::Visitor* visitor) const override;
  bool ToJavaScript(CFX_WideTextBuf* js, ReturnType type) const override;

 private:
  // Takes ownership of |pAccessors|.
  CXFA_FMForeachExpression(
      WideString wsIdentifier,
      std::vector<cppgc::Member<CXFA_FMSimpleExpression>>&& pAccessors,
      CXFA_FMExpression* pList);

  const WideString m_wsIdentifier;
  std::vector<cppgc::Member<CXFA_FMSimpleExpression>> const m_pAccessors;
  cppgc::Member<CXFA_FMExpression> const m_pList;
};

class CXFA_FMAST : public cppgc::GarbageCollected<CXFA_FMAST> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FMAST();

  void Trace(cppgc::Visitor* visitor) const;
  absl::optional<CFX_WideTextBuf> ToJavaScript() const;

 private:
  explicit CXFA_FMAST(
      std::vector<cppgc::Member<CXFA_FMExpression>> expressions);

  std::vector<cppgc::Member<CXFA_FMExpression>> const expressions_;
};

bool CXFA_IsTooBig(const CFX_WideTextBuf& js);

#endif  // XFA_FXFA_FORMCALC_CXFA_FMEXPRESSION_H_
