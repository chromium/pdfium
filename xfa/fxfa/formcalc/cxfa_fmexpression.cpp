// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/formcalc/cxfa_fmexpression.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/widetext_buffer.h"
#include "fxjs/gc/container_trace.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/formcalc/cxfa_fmtojavascriptdepth.h"

namespace {

const wchar_t kLessEqual[] = L" <= ";
const wchar_t kGreaterEqual[] = L" >= ";
const wchar_t kPlusEqual[] = L" += ";
const wchar_t kMinusEqual[] = L" -= ";

const wchar_t* const kBuiltInFuncs[] = {
    L"Abs",          L"Apr",       L"At",       L"Avg",
    L"Ceil",         L"Choose",    L"Concat",   L"Count",
    L"Cterm",        L"Date",      L"Date2Num", L"DateFmt",
    L"Decode",       L"Encode",    L"Eval",     L"Exists",
    L"Floor",        L"Format",    L"FV",       L"Get",
    L"HasValue",     L"If",        L"Ipmt",     L"IsoDate2Num",
    L"IsoTime2Num",  L"Left",      L"Len",      L"LocalDateFmt",
    L"LocalTimeFmt", L"Lower",     L"Ltrim",    L"Max",
    L"Min",          L"Mod",       L"NPV",      L"Num2Date",
    L"Num2GMTime",   L"Num2Time",  L"Oneof",    L"Parse",
    L"Pmt",          L"Post",      L"PPmt",     L"Put",
    L"PV",           L"Rate",      L"Ref",      L"Replace",
    L"Right",        L"Round",     L"Rtrim",    L"Space",
    L"Str",          L"Stuff",     L"Substr",   L"Sum",
    L"Term",         L"Time",      L"Time2Num", L"TimeFmt",
    L"UnitType",     L"UnitValue", L"Upper",    L"Uuid",
    L"Within",       L"WordNum",
};

const size_t kBuiltInFuncsMaxLen = 12;

struct XFA_FMSOMMethod {
  const wchar_t* m_wsSomMethodName;  // Ok, POD struct.
  uint32_t m_dParameters;
};

const XFA_FMSOMMethod kFMSomMethods[] = {
    {L"absPage", 0x01},
    {L"absPageInBatch", 0x01},
    {L"absPageSpan", 0x01},
    {L"append", 0x01},
    {L"clear", 0x01},
    {L"formNodes", 0x01},
    {L"h", 0x01},
    {L"insert", 0x03},
    {L"isRecordGroup", 0x01},
    {L"page", 0x01},
    {L"pageSpan", 0x01},
    {L"remove", 0x01},
    {L"saveFilteredXML", 0x01},
    {L"setElement", 0x01},
    {L"sheet", 0x01},
    {L"sheetInBatch", 0x01},
    {L"sign", 0x61},
    {L"verify", 0x0d},
    {L"w", 0x01},
    {L"x", 0x01},
    {L"y", 0x01},
};

WideString IdentifierToName(const WideString& ident) {
  if (ident.IsEmpty() || ident[0] != L'!')
    return ident;
  return L"pfm__excl__" + ident.Last(ident.GetLength() - 1);
}

}  // namespace

CXFA_FMExpression::CXFA_FMExpression() = default;

CXFA_FMExpression::~CXFA_FMExpression() = default;

void CXFA_FMExpression::Trace(cppgc::Visitor* visitor) const {}

CXFA_FMSimpleExpression::CXFA_FMSimpleExpression(XFA_FM_TOKEN op) : m_op(op) {}

CXFA_FMSimpleExpression::~CXFA_FMSimpleExpression() = default;

CXFA_FMChainableExpression::CXFA_FMChainableExpression(
    XFA_FM_TOKEN op,
    CXFA_FMSimpleExpression* pExp1,
    CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMSimpleExpression(op), m_pExp1(pExp1), m_pExp2(pExp2) {}

CXFA_FMChainableExpression::~CXFA_FMChainableExpression() = default;

void CXFA_FMChainableExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMSimpleExpression::Trace(visitor);
  visitor->Trace(m_pExp1);
  visitor->Trace(m_pExp2);
}

CXFA_FMNullExpression::CXFA_FMNullExpression()
    : CXFA_FMSimpleExpression(TOKnull) {}

CXFA_FMNullExpression::~CXFA_FMNullExpression() = default;

bool CXFA_FMNullExpression::ToJavaScript(WideTextBuffer* js,
                                         ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "null";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMNumberExpression::CXFA_FMNumberExpression(WideString wsNumber)
    : CXFA_FMSimpleExpression(TOKnumber), m_wsNumber(std::move(wsNumber)) {}

CXFA_FMNumberExpression::~CXFA_FMNumberExpression() = default;

bool CXFA_FMNumberExpression::ToJavaScript(WideTextBuffer* js,
                                           ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << m_wsNumber;
  return !CXFA_IsTooBig(*js);
}

CXFA_FMStringExpression::CXFA_FMStringExpression(WideString wsString)
    : CXFA_FMSimpleExpression(TOKstring), m_wsString(std::move(wsString)) {}

CXFA_FMStringExpression::~CXFA_FMStringExpression() = default;

bool CXFA_FMStringExpression::ToJavaScript(WideTextBuffer* js,
                                           ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempStr(m_wsString);
  if (tempStr.GetLength() <= 2) {
    *js << tempStr;
    return !CXFA_IsTooBig(*js);
  }

  *js << "\"";
  for (size_t i = 1; i < tempStr.GetLength() - 1; i++) {
    wchar_t oneChar = tempStr[i];
    switch (oneChar) {
      case L'\"':
        ++i;
        *js << "\\\"";
        break;
      case 0x0d:
        break;
      case 0x0a:
        *js << "\\n";
        break;
      default:
        js->AppendChar(oneChar);
        break;
    }
  }
  *js << "\"";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMIdentifierExpression::CXFA_FMIdentifierExpression(
    WideString wsIdentifier)
    : CXFA_FMSimpleExpression(TOKidentifier),
      m_wsIdentifier(std::move(wsIdentifier)) {}

CXFA_FMIdentifierExpression::~CXFA_FMIdentifierExpression() = default;

bool CXFA_FMIdentifierExpression::ToJavaScript(WideTextBuffer* js,
                                               ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (m_wsIdentifier.EqualsASCII("$"))
    *js << "this";
  else if (m_wsIdentifier.EqualsASCII("!"))
    *js << "xfa.datasets";
  else if (m_wsIdentifier.EqualsASCII("$data"))
    *js << "xfa.datasets.data";
  else if (m_wsIdentifier.EqualsASCII("$event"))
    *js << "xfa.event";
  else if (m_wsIdentifier.EqualsASCII("$form"))
    *js << "xfa.form";
  else if (m_wsIdentifier.EqualsASCII("$host"))
    *js << "xfa.host";
  else if (m_wsIdentifier.EqualsASCII("$layout"))
    *js << "xfa.layout";
  else if (m_wsIdentifier.EqualsASCII("$template"))
    *js << "xfa.template";
  else if (m_wsIdentifier[0] == L'!')
    *js << "pfm__excl__" << m_wsIdentifier.Last(m_wsIdentifier.GetLength() - 1);
  else
    *js << m_wsIdentifier;

  return !CXFA_IsTooBig(*js);
}

CXFA_FMAssignExpression::CXFA_FMAssignExpression(XFA_FM_TOKEN op,
                                                 CXFA_FMSimpleExpression* pExp1,
                                                 CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMChainableExpression(op, pExp1, pExp2) {}

CXFA_FMAssignExpression::~CXFA_FMAssignExpression() = default;

bool CXFA_FMAssignExpression::ToJavaScript(WideTextBuffer* js,
                                           ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideTextBuffer tempExp1;
  const CXFA_FMSimpleExpression* exp1 = GetFirstExpression();
  if (!exp1->ToJavaScript(&tempExp1, ReturnType::kInferred))
    return false;

  *js << "if (pfm_rt.is_obj(" << tempExp1 << "))\n{\n";
  if (type == ReturnType::kImplied)
    *js << "pfm_ret = ";

  WideTextBuffer tempExp2;
  const CXFA_FMSimpleExpression* exp2 = GetSecondExpression();
  if (!exp2->ToJavaScript(&tempExp2, ReturnType::kInferred))
    return false;

  *js << "pfm_rt.asgn_val_op(" << tempExp1 << ", " << tempExp2 << ");\n}\n";

  if (exp1->GetOperatorToken() == TOKidentifier &&
      !tempExp1.AsStringView().EqualsASCII("this")) {
    *js << "else\n{\n";
    if (type == ReturnType::kImplied)
      *js << "pfm_ret = ";

    *js << tempExp1 << " = pfm_rt.asgn_val_op";
    *js << "(" << tempExp1 << ", " << tempExp2 << ");\n";
    *js << "}\n";
  }
  return !CXFA_IsTooBig(*js);
}

CXFA_FMBinExpression::CXFA_FMBinExpression(const WideString& opName,
                                           XFA_FM_TOKEN op,
                                           CXFA_FMSimpleExpression* pExp1,
                                           CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMChainableExpression(op, pExp1, pExp2), m_OpName(opName) {}

CXFA_FMBinExpression::~CXFA_FMBinExpression() = default;

bool CXFA_FMBinExpression::ToJavaScript(WideTextBuffer* js,
                                        ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "pfm_rt." << m_OpName << "(";
  if (!GetFirstExpression()->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << ", ";
  if (!GetSecondExpression()->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << ")";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMLogicalOrExpression::CXFA_FMLogicalOrExpression(
    XFA_FM_TOKEN op,
    CXFA_FMSimpleExpression* pExp1,
    CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"log_or_op", op, pExp1, pExp2) {}

CXFA_FMLogicalOrExpression::~CXFA_FMLogicalOrExpression() = default;

CXFA_FMLogicalAndExpression::CXFA_FMLogicalAndExpression(
    XFA_FM_TOKEN op,
    CXFA_FMSimpleExpression* pExp1,
    CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"log_and_op", op, pExp1, pExp2) {}

CXFA_FMLogicalAndExpression::~CXFA_FMLogicalAndExpression() = default;

CXFA_FMEqualExpression::CXFA_FMEqualExpression(XFA_FM_TOKEN op,
                                               CXFA_FMSimpleExpression* pExp1,
                                               CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"eq_op", op, pExp1, pExp2) {}

CXFA_FMEqualExpression::~CXFA_FMEqualExpression() = default;

CXFA_FMNotEqualExpression::CXFA_FMNotEqualExpression(
    XFA_FM_TOKEN op,
    CXFA_FMSimpleExpression* pExp1,
    CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"neq_op", op, pExp1, pExp2) {}

CXFA_FMNotEqualExpression::~CXFA_FMNotEqualExpression() = default;

CXFA_FMGtExpression::CXFA_FMGtExpression(XFA_FM_TOKEN op,
                                         CXFA_FMSimpleExpression* pExp1,
                                         CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"gt_op", op, pExp1, pExp2) {}

CXFA_FMGtExpression::~CXFA_FMGtExpression() = default;

CXFA_FMGeExpression::CXFA_FMGeExpression(XFA_FM_TOKEN op,
                                         CXFA_FMSimpleExpression* pExp1,
                                         CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"ge_op", op, pExp1, pExp2) {}

CXFA_FMGeExpression::~CXFA_FMGeExpression() = default;

CXFA_FMLtExpression::CXFA_FMLtExpression(XFA_FM_TOKEN op,
                                         CXFA_FMSimpleExpression* pExp1,
                                         CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"lt_op", op, pExp1, pExp2) {}

CXFA_FMLtExpression::~CXFA_FMLtExpression() = default;

CXFA_FMLeExpression::CXFA_FMLeExpression(XFA_FM_TOKEN op,
                                         CXFA_FMSimpleExpression* pExp1,
                                         CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"le_op", op, pExp1, pExp2) {}

CXFA_FMLeExpression::~CXFA_FMLeExpression() = default;

CXFA_FMPlusExpression::CXFA_FMPlusExpression(XFA_FM_TOKEN op,
                                             CXFA_FMSimpleExpression* pExp1,
                                             CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"plus_op", op, pExp1, pExp2) {}

CXFA_FMPlusExpression::~CXFA_FMPlusExpression() = default;

CXFA_FMMinusExpression::CXFA_FMMinusExpression(XFA_FM_TOKEN op,
                                               CXFA_FMSimpleExpression* pExp1,
                                               CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"minus_op", op, pExp1, pExp2) {}

CXFA_FMMinusExpression::~CXFA_FMMinusExpression() = default;

CXFA_FMMulExpression::CXFA_FMMulExpression(XFA_FM_TOKEN op,
                                           CXFA_FMSimpleExpression* pExp1,
                                           CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"mul_op", op, pExp1, pExp2) {}

CXFA_FMMulExpression::~CXFA_FMMulExpression() = default;

CXFA_FMDivExpression::CXFA_FMDivExpression(XFA_FM_TOKEN op,
                                           CXFA_FMSimpleExpression* pExp1,
                                           CXFA_FMSimpleExpression* pExp2)
    : CXFA_FMBinExpression(L"div_op", op, pExp1, pExp2) {}

CXFA_FMDivExpression::~CXFA_FMDivExpression() = default;

CXFA_FMUnaryExpression::CXFA_FMUnaryExpression(const WideString& opName,
                                               XFA_FM_TOKEN op,
                                               CXFA_FMSimpleExpression* pExp)
    : CXFA_FMSimpleExpression(op), m_OpName(opName), m_pExp(pExp) {}

CXFA_FMUnaryExpression::~CXFA_FMUnaryExpression() = default;

void CXFA_FMUnaryExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMSimpleExpression::Trace(visitor);
  visitor->Trace(m_pExp);
}

bool CXFA_FMUnaryExpression::ToJavaScript(WideTextBuffer* js,
                                          ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "pfm_rt." << m_OpName << "(";
  if (!m_pExp->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << ")";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMPosExpression::CXFA_FMPosExpression(CXFA_FMSimpleExpression* pExp)
    : CXFA_FMUnaryExpression(L"pos_op", TOKplus, pExp) {}

CXFA_FMPosExpression::~CXFA_FMPosExpression() = default;

CXFA_FMNegExpression::CXFA_FMNegExpression(CXFA_FMSimpleExpression* pExp)
    : CXFA_FMUnaryExpression(L"neg_op", TOKminus, pExp) {}

CXFA_FMNegExpression::~CXFA_FMNegExpression() = default;

CXFA_FMNotExpression::CXFA_FMNotExpression(CXFA_FMSimpleExpression* pExp)
    : CXFA_FMUnaryExpression(L"log_not_op", TOKksnot, pExp) {}

CXFA_FMNotExpression::~CXFA_FMNotExpression() = default;

CXFA_FMCallExpression::CXFA_FMCallExpression(
    CXFA_FMSimpleExpression* pExp,
    std::vector<cppgc::Member<CXFA_FMSimpleExpression>>&& pArguments,
    bool bIsSomMethod)
    : CXFA_FMSimpleExpression(TOKcall),
      m_pExp(pExp),
      m_Arguments(std::move(pArguments)),
      m_bIsSomMethod(bIsSomMethod) {}

CXFA_FMCallExpression::~CXFA_FMCallExpression() = default;

void CXFA_FMCallExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMSimpleExpression::Trace(visitor);
  visitor->Trace(m_pExp);
  ContainerTrace(visitor, m_Arguments);
}

bool CXFA_FMCallExpression::IsBuiltInFunc(WideTextBuffer* funcName) const {
  if (funcName->GetLength() > kBuiltInFuncsMaxLen)
    return false;

  WideString str = funcName->MakeString();
  const wchar_t* const* pMatchResult =
      std::lower_bound(std::begin(kBuiltInFuncs), std::end(kBuiltInFuncs), str,
                       [](const wchar_t* iter, const WideString& val) -> bool {
                         return val.CompareNoCase(iter) > 0;
                       });
  if (pMatchResult != std::end(kBuiltInFuncs) &&
      !str.CompareNoCase(*pMatchResult)) {
    funcName->Clear();
    *funcName << *pMatchResult;
    return true;
  }
  return false;
}

uint32_t CXFA_FMCallExpression::IsMethodWithObjParam(
    const WideString& methodName) const {
  const XFA_FMSOMMethod* result = std::lower_bound(
      std::begin(kFMSomMethods), std::end(kFMSomMethods), methodName,
      [](const XFA_FMSOMMethod iter, const WideString& val) {
        return val.Compare(iter.m_wsSomMethodName) > 0;
      });
  if (result != std::end(kFMSomMethods) &&
      methodName == result->m_wsSomMethodName) {
    return result->m_dParameters;
  }
  return 0;
}

bool CXFA_FMCallExpression::ToJavaScript(WideTextBuffer* js,
                                         ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideTextBuffer funcName;
  if (!m_pExp->ToJavaScript(&funcName, ReturnType::kInferred))
    return false;

  if (m_bIsSomMethod) {
    *js << funcName << "(";
    uint32_t methodPara = IsMethodWithObjParam(funcName.MakeString());
    if (methodPara > 0) {
      for (size_t i = 0; i < m_Arguments.size(); ++i) {
        // Currently none of our expressions use objects for a parameter over
        // the 6th. Make sure we don't overflow the shift when doing this
        // check. If we ever need more the 32 object params we can revisit.
        *js << "pfm_rt.get_";
        if (i < 32 && (methodPara & (0x01 << i)) > 0)
          *js << "jsobj";
        else
          *js << "val";

        *js << "(";
        if (!m_Arguments[i]->ToJavaScript(js, ReturnType::kInferred))
          return false;
        *js << ")";
        if (i + 1 < m_Arguments.size())
          *js << ", ";
      }
    } else {
      for (const auto& expr : m_Arguments) {
        *js << "pfm_rt.get_val(";
        if (!expr->ToJavaScript(js, ReturnType::kInferred))
          return false;
        *js << ")";
        if (expr != m_Arguments.back())
          *js << ", ";
      }
    }
    *js << ")";
    return !CXFA_IsTooBig(*js);
  }

  bool isEvalFunc = false;
  bool isExistsFunc = false;
  if (!IsBuiltInFunc(&funcName)) {
    // If a function is not a SomMethod or a built-in then the input was
    // invalid, so failing. The scanner/lexer should catch this, but currently
    // doesn't. This failure will bubble up to the top-level and cause the
    // transpile to fail.
    return false;
  }

  if (funcName.AsStringView().EqualsASCII("Eval")) {
    isEvalFunc = true;
    *js << "eval.call(this, pfm_rt.Translate";
  } else {
    if (funcName.AsStringView().EqualsASCII("Exists"))
      isExistsFunc = true;

    *js << "pfm_rt." << funcName;
  }

  *js << "(";
  if (isExistsFunc) {
    *js << "\n(\nfunction ()\n{\ntry\n{\n";
    if (!m_Arguments.empty()) {
      *js << "return ";
      if (!m_Arguments[0]->ToJavaScript(js, ReturnType::kInferred))
        return false;
      *js << ";\n}\n";
    } else {
      *js << "return 0;\n}\n";
    }
    *js << "catch(accessExceptions)\n";
    *js << "{\nreturn 0;\n}\n}\n).call(this)\n";
  } else {
    for (const auto& expr : m_Arguments) {
      if (!expr->ToJavaScript(js, ReturnType::kInferred))
        return false;
      if (expr != m_Arguments.back())
        *js << ", ";
    }
  }
  *js << ")";
  if (isEvalFunc)
    *js << ")";

  return !CXFA_IsTooBig(*js);
}

CXFA_FMDotAccessorExpression::CXFA_FMDotAccessorExpression(
    CXFA_FMSimpleExpression* pAccessor,
    XFA_FM_TOKEN op,
    WideString wsIdentifier,
    CXFA_FMSimpleExpression* pIndexExp)
    : CXFA_FMChainableExpression(op, pAccessor, pIndexExp),
      m_wsIdentifier(std::move(wsIdentifier)) {}

CXFA_FMDotAccessorExpression::~CXFA_FMDotAccessorExpression() = default;

bool CXFA_FMDotAccessorExpression::ToJavaScript(WideTextBuffer* js,
                                                ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "pfm_rt.dot_acc(";

  CXFA_FMSimpleExpression* exp1 = GetFirstExpression();
  if (exp1) {
    // Write directly to the buffer with each recursion. Creating
    // and copying temporaries here becomes expensive when there
    // is deep recursion, even though we may need to re-create the
    // same thing again below. See https://crbug.com/1274018.
    if (!exp1->ToJavaScript(js, ReturnType::kInferred))
      return false;
  } else {
    *js << "null";
  }
  *js << ", \"";
  if (exp1 && exp1->GetOperatorToken() == TOKidentifier) {
    if (!exp1->ToJavaScript(js, ReturnType::kInferred))
      return false;
  }
  *js << "\", ";
  if (GetOperatorToken() == TOKdotscream)
    *js << "\"#" << m_wsIdentifier << "\", ";
  else if (GetOperatorToken() == TOKdotstar)
    *js << "\"*\", ";
  else if (GetOperatorToken() == TOKcall)
    *js << "\"\", ";
  else
    *js << "\"" << m_wsIdentifier << "\", ";

  CXFA_FMSimpleExpression* exp2 = GetSecondExpression();
  if (!exp2->ToJavaScript(js, ReturnType::kInferred))
    return false;

  *js << ")";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMIndexExpression::CXFA_FMIndexExpression(
    AccessorIndex accessorIndex,
    CXFA_FMSimpleExpression* pIndexExp,
    bool bIsStarIndex)
    : CXFA_FMSimpleExpression(TOKlbracket),
      m_pExp(pIndexExp),
      m_accessorIndex(accessorIndex),
      m_bIsStarIndex(bIsStarIndex) {}

CXFA_FMIndexExpression::~CXFA_FMIndexExpression() = default;

void CXFA_FMIndexExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMSimpleExpression::Trace(visitor);
  visitor->Trace(m_pExp);
}

bool CXFA_FMIndexExpression::ToJavaScript(WideTextBuffer* js,
                                          ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_accessorIndex) {
    case AccessorIndex::kNoIndex:
      *js << "0";
      break;
    case AccessorIndex::kNoRelativeIndex:
      *js << "1";
      break;
    case AccessorIndex::kPositiveIndex:
      *js << "2";
      break;
    case AccessorIndex::kNegativeIndex:
      *js << "3";
      break;
  }
  if (m_bIsStarIndex)
    return !CXFA_IsTooBig(*js);

  *js << ", ";
  if (m_pExp) {
    if (!m_pExp->ToJavaScript(js, ReturnType::kInferred))
      return false;
  } else {
    *js << "0";
  }
  return !CXFA_IsTooBig(*js);
}

CXFA_FMDotDotAccessorExpression::CXFA_FMDotDotAccessorExpression(
    CXFA_FMSimpleExpression* pAccessor,
    XFA_FM_TOKEN op,
    WideString wsIdentifier,
    CXFA_FMSimpleExpression* pIndexExp)
    : CXFA_FMChainableExpression(op, pAccessor, pIndexExp),
      m_wsIdentifier(std::move(wsIdentifier)) {}

CXFA_FMDotDotAccessorExpression::~CXFA_FMDotDotAccessorExpression() = default;

bool CXFA_FMDotDotAccessorExpression::ToJavaScript(WideTextBuffer* js,
                                                   ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  CXFA_FMSimpleExpression* exp1 = GetFirstExpression();
  *js << "pfm_rt.dotdot_acc(";
  if (!exp1->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << ", "
      << "\"";
  if (exp1->GetOperatorToken() == TOKidentifier) {
    if (!exp1->ToJavaScript(js, ReturnType::kInferred))
      return false;
  }

  CXFA_FMSimpleExpression* exp2 = GetSecondExpression();
  *js << "\", \"" << m_wsIdentifier << "\", ";
  if (!exp2->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << ")";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMMethodCallExpression::CXFA_FMMethodCallExpression(
    CXFA_FMSimpleExpression* pAccessorExp1,
    CXFA_FMSimpleExpression* pCallExp)
    : CXFA_FMChainableExpression(TOKdot, pAccessorExp1, pCallExp) {}

CXFA_FMMethodCallExpression::~CXFA_FMMethodCallExpression() = default;

bool CXFA_FMMethodCallExpression::ToJavaScript(WideTextBuffer* js,
                                               ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "(function() {\n";
  *js << "  return pfm_method_runner(";
  if (!GetFirstExpression()->ToJavaScript(js, ReturnType::kInferred))
    return false;

  *js << ", function(obj) {\n";
  *js << "    return obj.";
  if (!GetSecondExpression()->ToJavaScript(js, ReturnType::kInferred))
    return false;

  *js << ";\n";
  *js << "  });\n";
  *js << "}).call(this)";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    WideString wsName,
    std::vector<WideString>&& arguments,
    std::vector<cppgc::Member<CXFA_FMExpression>>&& expressions)
    : m_wsName(std::move(wsName)),
      m_pArguments(std::move(arguments)),
      m_pExpressions(std::move(expressions)) {
  DCHECK(!m_wsName.IsEmpty());
}

CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() = default;

void CXFA_FMFunctionDefinition::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  ContainerTrace(visitor, m_pExpressions);
}

bool CXFA_FMFunctionDefinition::ToJavaScript(WideTextBuffer* js,
                                             ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (m_wsName.IsEmpty())
    return false;

  *js << "function " << IdentifierToName(m_wsName) << "(";
  for (const auto& identifier : m_pArguments) {
    if (identifier != m_pArguments.front())
      *js << ", ";

    *js << IdentifierToName(identifier);
  }
  *js << ") {\n";

  *js << "var pfm_ret = null;\n";
  for (const auto& expr : m_pExpressions) {
    ReturnType ret_type = expr == m_pExpressions.back() ? ReturnType::kImplied
                                                        : ReturnType::kInferred;
    if (!expr->ToJavaScript(js, ret_type))
      return false;
  }

  *js << "return pfm_ret;\n";
  *js << "}\n";

  return !CXFA_IsTooBig(*js);
}

CXFA_FMAST::CXFA_FMAST(
    std::vector<cppgc::Member<CXFA_FMExpression>> expressions)
    : expressions_(std::move(expressions)) {}

CXFA_FMAST::~CXFA_FMAST() = default;

void CXFA_FMAST::Trace(cppgc::Visitor* visitor) const {
  ContainerTrace(visitor, expressions_);
}

std::optional<WideTextBuffer> CXFA_FMAST::ToJavaScript() const {
  WideTextBuffer js;
  if (expressions_.empty()) {
    js << "// comments only";
    return js;
  }

  js << "(function() {\n";
  js << "let pfm_method_runner = function(obj, cb) {\n";
  js << "  if (pfm_rt.is_ary(obj)) {\n";
  js << "    let pfm_method_return = null;\n";
  js << "    for (var idx = obj.length -1; idx > 1; idx--) {\n";
  js << "      pfm_method_return = cb(obj[idx]);\n";
  js << "    }\n";
  js << "    return pfm_method_return;\n";
  js << "  }\n";
  js << "  return cb(obj);\n";
  js << "};\n";
  js << "var pfm_ret = null;\n";
  for (const auto& expr : expressions_) {
    CXFA_FMAssignExpression::ReturnType ret_type =
        expr == expressions_.back()
            ? CXFA_FMAssignExpression::ReturnType::kImplied
            : CXFA_FMAssignExpression::ReturnType::kInferred;
    if (!expr->ToJavaScript(&js, ret_type))
      return std::nullopt;
  }
  js << "return pfm_rt.get_val(pfm_ret);\n";
  js << "}).call(this);";

  if (CXFA_IsTooBig(js))
    return std::nullopt;

  return js;
}

CXFA_FMVarExpression::CXFA_FMVarExpression(WideString wsName,
                                           CXFA_FMSimpleExpression* pInit)
    : m_wsName(std::move(wsName)), m_pInit(pInit) {}

CXFA_FMVarExpression::~CXFA_FMVarExpression() = default;

void CXFA_FMVarExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pInit);
}

bool CXFA_FMVarExpression::ToJavaScript(WideTextBuffer* js,
                                        ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempName = IdentifierToName(m_wsName);
  *js << "var " << tempName << " = ";
  if (m_pInit) {
    if (!m_pInit->ToJavaScript(js, ReturnType::kInferred))
      return false;

    *js << ";\n";
    *js << tempName << " = pfm_rt.var_filter(" << tempName << ");\n";
  } else {
    *js << "\"\";\n";
  }

  if (type == ReturnType::kImplied)
    *js << "pfm_ret = " << tempName << ";\n";

  return !CXFA_IsTooBig(*js);
}

CXFA_FMExpExpression::CXFA_FMExpExpression(CXFA_FMSimpleExpression* pExpression)
    : m_pExpression(pExpression) {}

CXFA_FMExpExpression::~CXFA_FMExpExpression() = default;

void CXFA_FMExpExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pExpression);
}

bool CXFA_FMExpExpression::ToJavaScript(WideTextBuffer* js,
                                        ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kInferred) {
    bool ret = m_pExpression->ToJavaScript(js, ReturnType::kInferred);
    if (m_pExpression->GetOperatorToken() != TOKassign)
      *js << ";\n";

    return ret;
  }

  if (m_pExpression->GetOperatorToken() == TOKassign)
    return m_pExpression->ToJavaScript(js, ReturnType::kImplied);

  if (m_pExpression->GetOperatorToken() == TOKstar ||
      m_pExpression->GetOperatorToken() == TOKdotstar ||
      m_pExpression->GetOperatorToken() == TOKdotscream ||
      m_pExpression->GetOperatorToken() == TOKdotdot ||
      m_pExpression->GetOperatorToken() == TOKdot) {
    *js << "pfm_ret = pfm_rt.get_val(";
    if (!m_pExpression->ToJavaScript(js, ReturnType::kInferred))
      return false;

    *js << ");\n";
    return !CXFA_IsTooBig(*js);
  }

  *js << "pfm_ret = ";
  if (!m_pExpression->ToJavaScript(js, ReturnType::kInferred))
    return false;

  *js << ";\n";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMBlockExpression::CXFA_FMBlockExpression(
    std::vector<cppgc::Member<CXFA_FMExpression>>&& pExpressionList)
    : m_ExpressionList(std::move(pExpressionList)) {}

CXFA_FMBlockExpression::~CXFA_FMBlockExpression() = default;

void CXFA_FMBlockExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  ContainerTrace(visitor, m_ExpressionList);
}

bool CXFA_FMBlockExpression::ToJavaScript(WideTextBuffer* js,
                                          ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "{\n";
  for (const auto& expr : m_ExpressionList) {
    if (type == ReturnType::kInferred) {
      if (!expr->ToJavaScript(js, ReturnType::kInferred))
        return false;
    } else {
      ReturnType ret_type = expr == m_ExpressionList.back()
                                ? ReturnType::kImplied
                                : ReturnType::kInferred;
      if (!expr->ToJavaScript(js, ret_type))
        return false;
    }
  }
  *js << "}\n";

  return !CXFA_IsTooBig(*js);
}

CXFA_FMDoExpression::CXFA_FMDoExpression(CXFA_FMExpression* pList)
    : m_pList(pList) {}

CXFA_FMDoExpression::~CXFA_FMDoExpression() = default;

void CXFA_FMDoExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pList);
}

bool CXFA_FMDoExpression::ToJavaScript(WideTextBuffer* js,
                                       ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  return m_pList->ToJavaScript(js, type);
}

CXFA_FMIfExpression::CXFA_FMIfExpression(
    CXFA_FMSimpleExpression* pExpression,
    CXFA_FMExpression* pIfExpression,
    std::vector<cppgc::Member<CXFA_FMIfExpression>>&& pElseIfExpressions,
    CXFA_FMExpression* pElseExpression)
    : m_pExpression(pExpression),
      m_pIfExpression(pIfExpression),
      m_pElseIfExpressions(std::move(pElseIfExpressions)),
      m_pElseExpression(pElseExpression) {
  DCHECK(m_pExpression);
}

CXFA_FMIfExpression::~CXFA_FMIfExpression() = default;

void CXFA_FMIfExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pExpression);
  visitor->Trace(m_pIfExpression);
  ContainerTrace(visitor, m_pElseIfExpressions);
  visitor->Trace(m_pElseExpression);
}

bool CXFA_FMIfExpression::ToJavaScript(WideTextBuffer* js,
                                       ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    *js << "pfm_ret = 0;\n";

  *js << "if (pfm_rt.get_val(";
  if (!m_pExpression->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << "))\n";

  if (CXFA_IsTooBig(*js))
    return false;

  if (m_pIfExpression) {
    if (!m_pIfExpression->ToJavaScript(js, type))
      return false;
    if (CXFA_IsTooBig(*js))
      return false;
  }

  for (auto& expr : m_pElseIfExpressions) {
    *js << "else ";
    if (!expr->ToJavaScript(js, ReturnType::kInferred))
      return false;
  }

  if (m_pElseExpression) {
    *js << "else ";
    if (!m_pElseExpression->ToJavaScript(js, type))
      return false;
  }
  return !CXFA_IsTooBig(*js);
}

CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    CXFA_FMSimpleExpression* pCondition,
    CXFA_FMExpression* pExpression)
    : m_pCondition(pCondition), m_pExpression(pExpression) {}

CXFA_FMWhileExpression::~CXFA_FMWhileExpression() = default;

void CXFA_FMWhileExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pCondition);
  visitor->Trace(m_pExpression);
}

bool CXFA_FMWhileExpression::ToJavaScript(WideTextBuffer* js,
                                          ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    *js << "pfm_ret = 0;\n";

  *js << "while (";
  if (!m_pCondition->ToJavaScript(js, ReturnType::kInferred))
    return false;

  *js << ")\n";
  if (CXFA_IsTooBig(*js))
    return false;

  if (!m_pExpression->ToJavaScript(js, type))
    return false;

  return !CXFA_IsTooBig(*js);
}

CXFA_FMBreakExpression::CXFA_FMBreakExpression() = default;

CXFA_FMBreakExpression::~CXFA_FMBreakExpression() = default;

bool CXFA_FMBreakExpression::ToJavaScript(WideTextBuffer* js,
                                          ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "pfm_ret = 0;\nbreak;\n";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMContinueExpression::CXFA_FMContinueExpression() = default;

CXFA_FMContinueExpression::~CXFA_FMContinueExpression() = default;

bool CXFA_FMContinueExpression::ToJavaScript(WideTextBuffer* js,
                                             ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  *js << "pfm_ret = 0;\ncontinue;\n";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMForExpression::CXFA_FMForExpression(WideString wsVariant,
                                           CXFA_FMSimpleExpression* pAssignment,
                                           CXFA_FMSimpleExpression* pAccessor,
                                           int32_t iDirection,
                                           CXFA_FMSimpleExpression* pStep,
                                           CXFA_FMExpression* pList)
    : m_wsVariant(std::move(wsVariant)),
      m_bDirection(iDirection == 1),
      m_pAssignment(pAssignment),
      m_pAccessor(pAccessor),
      m_pStep(pStep),
      m_pList(pList) {}

CXFA_FMForExpression::~CXFA_FMForExpression() = default;

void CXFA_FMForExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  visitor->Trace(m_pAssignment);
  visitor->Trace(m_pAccessor);
  visitor->Trace(m_pStep);
  visitor->Trace(m_pList);
}

bool CXFA_FMForExpression::ToJavaScript(WideTextBuffer* js,
                                        ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    *js << "pfm_ret = 0;\n";

  *js << "{\n";

  WideString tmpName = IdentifierToName(m_wsVariant);
  *js << "var " << tmpName << " = null;\n";

  *js << "for (" << tmpName << " = pfm_rt.get_val(";
  if (!m_pAssignment->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << "); ";

  *js << tmpName << (m_bDirection ? kLessEqual : kGreaterEqual);
  *js << "pfm_rt.get_val(";
  if (!m_pAccessor->ToJavaScript(js, ReturnType::kInferred))
    return false;
  *js << "); ";

  *js << tmpName << (m_bDirection ? kPlusEqual : kMinusEqual);
  if (m_pStep) {
    *js << "pfm_rt.get_val(";
    if (!m_pStep->ToJavaScript(js, ReturnType::kInferred))
      return false;
    *js << ")";
  } else {
    *js << "1";
  }
  *js << ")\n";
  if (CXFA_IsTooBig(*js))
    return false;

  if (!m_pList->ToJavaScript(js, type))
    return false;

  *js << "}\n";
  return !CXFA_IsTooBig(*js);
}

CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    WideString wsIdentifier,
    std::vector<cppgc::Member<CXFA_FMSimpleExpression>>&& pAccessors,
    CXFA_FMExpression* pList)
    : m_wsIdentifier(std::move(wsIdentifier)),
      m_pAccessors(std::move(pAccessors)),
      m_pList(pList) {}

CXFA_FMForeachExpression::~CXFA_FMForeachExpression() = default;

void CXFA_FMForeachExpression::Trace(cppgc::Visitor* visitor) const {
  CXFA_FMExpression::Trace(visitor);
  ContainerTrace(visitor, m_pAccessors);
  visitor->Trace(m_pList);
}

bool CXFA_FMForeachExpression::ToJavaScript(WideTextBuffer* js,
                                            ReturnType type) const {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(*js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    *js << "pfm_ret = 0;\n";

  *js << "{\n";

  WideString tmpName = IdentifierToName(m_wsIdentifier);
  *js << "var " << tmpName << " = null;\n";
  *js << "var pfm_ary = pfm_rt.concat_obj(";
  for (const auto& expr : m_pAccessors) {
    if (!expr->ToJavaScript(js, ReturnType::kInferred))
      return false;
    if (expr != m_pAccessors.back())
      *js << ", ";
  }
  *js << ");\n";

  *js << "var pfm_ary_idx = 0;\n";
  *js << "while(pfm_ary_idx < pfm_ary.length)\n{\n";
  *js << tmpName << " = pfm_ary[pfm_ary_idx++];\n";
  if (!m_pList->ToJavaScript(js, type))
    return false;

  *js << "}\n";  // while
  *js << "}\n";  // block
  return !CXFA_IsTooBig(*js);
}

bool CXFA_IsTooBig(const WideTextBuffer& js) {
  return js.GetSize() >= 256 * 1024 * 1024;
}
