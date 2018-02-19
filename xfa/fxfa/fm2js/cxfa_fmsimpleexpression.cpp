// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmsimpleexpression.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/logging.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

namespace {

const wchar_t* const g_BuiltInFuncs[] = {
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

const size_t g_BuiltInFuncsMaxLen = 12;

struct XFA_FMSOMMethod {
  const wchar_t* m_wsSomMethodName;
  uint32_t m_dParameters;
};

const XFA_FMSOMMethod gs_FMSomMethods[] = {
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

}  // namespace

CXFA_FMSimpleExpression::CXFA_FMSimpleExpression(uint32_t line, XFA_FM_TOKEN op)
    : m_line(line), m_op(op) {}

XFA_FM_TOKEN CXFA_FMSimpleExpression::GetOperatorToken() const {
  return m_op;
}

CXFA_FMNullExpression::CXFA_FMNullExpression(uint32_t line)
    : CXFA_FMSimpleExpression(line, TOKnull) {}

bool CXFA_FMNullExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << L"null";
  return !CXFA_IsTooBig(js);
}

CXFA_FMNumberExpression::CXFA_FMNumberExpression(uint32_t line,
                                                 WideStringView wsNumber)
    : CXFA_FMSimpleExpression(line, TOKnumber), m_wsNumber(wsNumber) {}

CXFA_FMNumberExpression::~CXFA_FMNumberExpression() {}

bool CXFA_FMNumberExpression::ToJavaScript(CFX_WideTextBuf& js,
                                           ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << m_wsNumber;
  return !CXFA_IsTooBig(js);
}

CXFA_FMStringExpression::CXFA_FMStringExpression(uint32_t line,
                                                 WideStringView wsString)
    : CXFA_FMSimpleExpression(line, TOKstring), m_wsString(wsString) {}

CXFA_FMStringExpression::~CXFA_FMStringExpression() {}

bool CXFA_FMStringExpression::ToJavaScript(CFX_WideTextBuf& js,
                                           ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempStr(m_wsString);
  if (tempStr.GetLength() <= 2) {
    js << tempStr;
    return !CXFA_IsTooBig(js);
  }

  js << L"\"";
  for (size_t i = 1; i < tempStr.GetLength() - 1; i++) {
    wchar_t oneChar = tempStr[i];
    switch (oneChar) {
      case L'\"':
        ++i;
        js << L"\\\"";
        break;
      case 0x0d:
        break;
      case 0x0a:
        js << L"\\n";
        break;
      default:
        js.AppendChar(oneChar);
        break;
    }
  }
  js << L"\"";
  return !CXFA_IsTooBig(js);
}

CXFA_FMIdentifierExpression::CXFA_FMIdentifierExpression(
    uint32_t line,
    WideStringView wsIdentifier)
    : CXFA_FMSimpleExpression(line, TOKidentifier),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMIdentifierExpression::~CXFA_FMIdentifierExpression() {}

bool CXFA_FMIdentifierExpression::ToJavaScript(CFX_WideTextBuf& js,
                                               ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (m_wsIdentifier == L"$")
    js << L"this";
  else if (m_wsIdentifier == L"!")
    js << L"xfa.datasets";
  else if (m_wsIdentifier == L"$data")
    js << L"xfa.datasets.data";
  else if (m_wsIdentifier == L"$event")
    js << L"xfa.event";
  else if (m_wsIdentifier == L"$form")
    js << L"xfa.form";
  else if (m_wsIdentifier == L"$host")
    js << L"xfa.host";
  else if (m_wsIdentifier == L"$layout")
    js << L"xfa.layout";
  else if (m_wsIdentifier == L"$template")
    js << L"xfa.template";
  else if (m_wsIdentifier[0] == L'!')
    js << L"pfm__excl__" + m_wsIdentifier.Right(m_wsIdentifier.GetLength() - 1);
  else
    js << m_wsIdentifier;

  return !CXFA_IsTooBig(js);
}

CXFA_FMUnaryExpression::CXFA_FMUnaryExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMSimpleExpression(line, op), m_pExp(std::move(pExp)) {}

CXFA_FMUnaryExpression::~CXFA_FMUnaryExpression() = default;

CXFA_FMBinExpression::CXFA_FMBinExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMSimpleExpression(line, op),
      m_pExp1(std::move(pExp1)),
      m_pExp2(std::move(pExp2)) {}

CXFA_FMBinExpression::~CXFA_FMBinExpression() = default;

CXFA_FMAssignExpression::CXFA_FMAssignExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAssignExpression::ToJavaScript(CFX_WideTextBuf& js,
                                           ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1, ReturnType::kInfered))
    return false;

  js << L"if (pfm_rt.is_obj(" << tempExp1 << L"))\n{\n";
  if (type == ReturnType::kImplied)
    js << L"pfm_ret = ";

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.asgn_val_op(" << tempExp1 << L", " << tempExp2 << L");\n}\n";

  if (m_pExp1->GetOperatorToken() == TOKidentifier &&
      tempExp1.AsStringView() != L"this") {
    js << L"else\n{\n";
    if (type == ReturnType::kImplied)
      js << L"pfm_ret = ";

    js << tempExp1 << L" = pfm_rt.asgn_val_op";
    js << L"(" << tempExp1 << L", " << tempExp2 << L");\n";
    js << L"}\n";
  }
  return !CXFA_IsTooBig(js);
}

CXFA_FMLogicalOrExpression::CXFA_FMLogicalOrExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalOrExpression::ToJavaScript(CFX_WideTextBuf& js,
                                              ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.log_or_op(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMLogicalAndExpression::CXFA_FMLogicalAndExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalAndExpression::ToJavaScript(CFX_WideTextBuf& js,
                                               ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.log_and_op(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMEqualityExpression::CXFA_FMEqualityExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMEqualityExpression::ToJavaScript(CFX_WideTextBuf& js,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKeq:
    case TOKkseq:
      js << L"pfm_rt.eq_op";
      break;
    case TOKne:
    case TOKksne:
      js << L"pfm_rt.neq_op";
      break;
    default:
      NOTREACHED();
      break;
  }

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMRelationalExpression::CXFA_FMRelationalExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMRelationalExpression::ToJavaScript(CFX_WideTextBuf& js,
                                               ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKlt:
    case TOKkslt:
      js << L"pfm_rt.lt_op";
      break;
    case TOKgt:
    case TOKksgt:
      js << L"pfm_rt.gt_op";
      break;
    case TOKle:
    case TOKksle:
      js << L"pfm_rt.le_op";
      break;
    case TOKge:
    case TOKksge:
      js << L"pfm_rt.ge_op";
      break;
    default:
      NOTREACHED();
      break;
  }

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMAdditiveExpression::CXFA_FMAdditiveExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAdditiveExpression::ToJavaScript(CFX_WideTextBuf& js,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKplus:
      js << L"pfm_rt.plus_op";
      break;
    case TOKminus:
      js << L"pfm_rt.minus_op";
      break;
    default:
      NOTREACHED();
      break;
  }

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMMultiplicativeExpression::CXFA_FMMultiplicativeExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMMultiplicativeExpression::ToJavaScript(CFX_WideTextBuf& js,
                                                   ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKmul:
      js << L"pfm_rt.mul_op";
      break;
    case TOKdiv:
      js << L"pfm_rt.div_op";
      break;
    default:
      NOTREACHED();
      break;
  }

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"(" << exp1_txt << L", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMPosExpression::CXFA_FMPosExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKplus, std::move(pExp)) {}

bool CXFA_FMPosExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp_txt;
  if (!m_pExp->ToJavaScript(exp_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.pos_op(" << exp_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMNegExpression::CXFA_FMNegExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKminus, std::move(pExp)) {}

bool CXFA_FMNegExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp_txt;
  if (!m_pExp->ToJavaScript(exp_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.neg_op(" << exp_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMNotExpression::CXFA_FMNotExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKksnot, std::move(pExp)) {}

bool CXFA_FMNotExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp_txt;
  if (!m_pExp->ToJavaScript(exp_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.log_not_op(" << exp_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMCallExpression::CXFA_FMCallExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pArguments,
    bool bIsSomMethod)
    : CXFA_FMUnaryExpression(line, TOKcall, std::move(pExp)),
      m_bIsSomMethod(bIsSomMethod),
      m_Arguments(std::move(pArguments)) {}

CXFA_FMCallExpression::~CXFA_FMCallExpression() {}

bool CXFA_FMCallExpression::IsBuiltInFunc(CFX_WideTextBuf* funcName) {
  if (funcName->GetLength() > g_BuiltInFuncsMaxLen)
    return false;

  WideString str = funcName->MakeString();
  const wchar_t* const* pMatchResult = std::lower_bound(
      std::begin(g_BuiltInFuncs), std::end(g_BuiltInFuncs), str,
      [](const wchar_t* iter, const WideString& val) -> bool {
        return val.CompareNoCase(iter) > 0;
      });
  if (pMatchResult != std::end(g_BuiltInFuncs) &&
      !str.CompareNoCase(*pMatchResult)) {
    funcName->Clear();
    *funcName << *pMatchResult;
    return true;
  }
  return false;
}

uint32_t CXFA_FMCallExpression::IsMethodWithObjParam(
    const WideString& methodName) {
  const XFA_FMSOMMethod* result = std::lower_bound(
      std::begin(gs_FMSomMethods), std::end(gs_FMSomMethods), methodName,
      [](const XFA_FMSOMMethod iter, const WideString& val) {
        return val.Compare(iter.m_wsSomMethodName) > 0;
      });
  if (result != std::end(gs_FMSomMethods) &&
      !methodName.Compare(result->m_wsSomMethodName)) {
    return result->m_dParameters;
  }
  return 0;
}

bool CXFA_FMCallExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf funcName;
  if (!m_pExp->ToJavaScript(funcName, ReturnType::kInfered))
    return false;

  if (m_bIsSomMethod) {
    js << funcName << L"(";
    uint32_t methodPara = IsMethodWithObjParam(funcName.MakeString());
    if (methodPara > 0) {
      for (size_t i = 0; i < m_Arguments.size(); ++i) {
        CFX_WideTextBuf expr_txt;
        if (!m_Arguments[i]->ToJavaScript(expr_txt, ReturnType::kInfered))
          return false;

        // Currently none of our expressions use objects for a parameter over
        // the 6th. Make sure we don't overflow the shift when doing this
        // check. If we ever need more the 32 object params we can revisit.
        if (i < 32 && (methodPara & (0x01 << i)) > 0)
          js << L"pfm_rt.get_jsobj(" << expr_txt << L")";
        else
          js << L"pfm_rt.get_val(" << expr_txt << L")";

        if (i + 1 < m_Arguments.size())
          js << L", ";
      }
    } else {
      for (const auto& expr : m_Arguments) {
        CFX_WideTextBuf expr_txt;
        if (!expr->ToJavaScript(expr_txt, ReturnType::kInfered))
          return false;

        js << L"pfm_rt.get_val(" << expr_txt << L")";
        if (expr != m_Arguments.back())
          js << L", ";
      }
    }
    js << L")";
    return !CXFA_IsTooBig(js);
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

  if (funcName.AsStringView() == L"Eval") {
    isEvalFunc = true;
    js << L"eval.call(this, pfm_rt.Translate";
  } else {
    if (funcName.AsStringView() == L"Exists")
      isExistsFunc = true;

    js << L"pfm_rt." << funcName;
  }

  js << L"(";
  if (isExistsFunc) {
    js << L"\n(\nfunction ()\n{\ntry\n{\n";
    if (!m_Arguments.empty()) {
      CFX_WideTextBuf expr_txt;
      if (!m_Arguments[0]->ToJavaScript(expr_txt, ReturnType::kInfered))
        return false;

      js << L"return " << expr_txt << L";\n}\n";
    } else {
      js << L"return 0;\n}\n";
    }
    js << L"catch(accessExceptions)\n";
    js << L"{\nreturn 0;\n}\n}\n).call(this)\n";
  } else {
    for (const auto& expr : m_Arguments) {
      if (!expr->ToJavaScript(js, ReturnType::kInfered))
        return false;
      if (expr != m_Arguments.back())
        js << L", ";
    }
  }
  js << L")";
  if (isEvalFunc)
    js << L")";

  return !CXFA_IsTooBig(js);
}

CXFA_FMDotAccessorExpression::CXFA_FMDotAccessorExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    XFA_FM_TOKEN op,
    WideStringView wsIdentifier,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp)
    : CXFA_FMBinExpression(line,
                           op,
                           std::move(pAccessor),
                           std::move(pIndexExp)),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMDotAccessorExpression::~CXFA_FMDotAccessorExpression() {}

bool CXFA_FMDotAccessorExpression::ToJavaScript(CFX_WideTextBuf& js,
                                                ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << L"pfm_rt.dot_acc(";

  CFX_WideTextBuf tempExp1;
  if (m_pExp1) {
    if (!m_pExp1->ToJavaScript(tempExp1, ReturnType::kInfered))
      return false;

    js << tempExp1;
  } else {
    js << L"null";
  }
  js << L", \"";

  if (m_pExp1 && m_pExp1->GetOperatorToken() == TOKidentifier)
    js << tempExp1;

  js << L"\", ";
  if (m_op == TOKdotscream)
    js << L"\"#" << m_wsIdentifier << L"\", ";
  else if (m_op == TOKdotstar)
    js << L"\"*\", ";
  else if (m_op == TOKcall)
    js << L"\"\", ";
  else
    js << L"\"" << m_wsIdentifier << L"\", ";

  if (!m_pExp2->ToJavaScript(js, ReturnType::kInfered))
    return false;

  js << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMIndexExpression::CXFA_FMIndexExpression(
    uint32_t line,
    XFA_FM_AccessorIndex accessorIndex,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp,
    bool bIsStarIndex)
    : CXFA_FMUnaryExpression(line, TOKlbracket, std::move(pIndexExp)),
      m_accessorIndex(accessorIndex),
      m_bIsStarIndex(bIsStarIndex) {}

bool CXFA_FMIndexExpression::ToJavaScript(CFX_WideTextBuf& js,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_accessorIndex) {
    case ACCESSOR_NO_INDEX:
      js << L"0";
      break;
    case ACCESSOR_NO_RELATIVEINDEX:
      js << L"1";
      break;
    case ACCESSOR_POSITIVE_INDEX:
      js << L"2";
      break;
    case ACCESSOR_NEGATIVE_INDEX:
      js << L"3";
      break;
    default:
      js << L"0";
  }
  if (m_bIsStarIndex)
    return !CXFA_IsTooBig(js);

  js << L", ";
  if (m_pExp) {
    if (!m_pExp->ToJavaScript(js, ReturnType::kInfered))
      return false;
  } else {
    js << L"0";
  }
  return !CXFA_IsTooBig(js);
}

CXFA_FMDotDotAccessorExpression::CXFA_FMDotDotAccessorExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    XFA_FM_TOKEN op,
    WideStringView wsIdentifier,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp)
    : CXFA_FMBinExpression(line,
                           op,
                           std::move(pAccessor),
                           std::move(pIndexExp)),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMDotDotAccessorExpression::~CXFA_FMDotDotAccessorExpression() {}

bool CXFA_FMDotDotAccessorExpression::ToJavaScript(CFX_WideTextBuf& js,
                                                   ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"pfm_rt.dotdot_acc(" << exp1_txt << L", " << L"\"";
  if (m_pExp1->GetOperatorToken() == TOKidentifier)
    js << exp1_txt;

  js << L"\", \"" << m_wsIdentifier << L"\", " << exp2_txt << L")";
  return !CXFA_IsTooBig(js);
}

CXFA_FMMethodCallExpression::CXFA_FMMethodCallExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessorExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pCallExp)
    : CXFA_FMBinExpression(line,
                           TOKdot,
                           std::move(pAccessorExp1),
                           std::move(pCallExp)) {}

bool CXFA_FMMethodCallExpression::ToJavaScript(CFX_WideTextBuf& js,
                                               ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf exp1_txt;
  if (!m_pExp1->ToJavaScript(exp1_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf exp2_txt;
  if (!m_pExp2->ToJavaScript(exp2_txt, ReturnType::kInfered))
    return false;

  js << L"(\nfunction ()\n{\n";
  js << L"var method_return_value = null;\n";
  js << L"var accessor_object = " << exp1_txt << L";\n";
  js << L"if (pfm_rt.is_ary(accessor_object))\n{\n";
  js << L"for(var index = accessor_object.length - 1; index > 1; "
        L"index--)\n{\n";
  js << L"method_return_value = accessor_object[index]." << exp2_txt << L";\n";
  js << L"}\n}\nelse\n{\n";
  js << L"method_return_value = accessor_object." << exp2_txt << L";\n";
  js << L"}\n";
  js << L"return method_return_value;\n";
  js << L"}\n).call(this)";
  return !CXFA_IsTooBig(js);
}

bool CXFA_IsTooBig(const CFX_WideTextBuf& javascript) {
  return javascript.GetSize() >= 256 * 1024 * 1024;
}
