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

// Indexed by XFA_FM_SimpleExpressionType
const wchar_t* const gs_lpStrExpFuncName[] = {
    L"pfm_rt.asgn_val_op", L"pfm_rt.log_or_op",  L"pfm_rt.log_and_op",
    L"pfm_rt.eq_op",       L"pfm_rt.neq_op",     L"pfm_rt.lt_op",
    L"pfm_rt.le_op",       L"pfm_rt.gt_op",      L"pfm_rt.ge_op",
    L"pfm_rt.plus_op",     L"pfm_rt.minus_op",   L"pfm_rt.mul_op",
    L"pfm_rt.div_op",      L"pfm_rt.pos_op",     L"pfm_rt.neg_op",
    L"pfm_rt.log_not_op",  L"pfm_rt.",           L"pfm_rt.dot_acc",
    L"pfm_rt.dotdot_acc",  L"pfm_rt.concat_obj", L"pfm_rt.is_obj",
    L"pfm_rt.is_ary",      L"pfm_rt.get_val",    L"pfm_rt.get_jsobj",
    L"pfm_rt.var_filter",
};

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

WideStringView XFA_FM_EXPTypeToString(
    XFA_FM_SimpleExpressionType simpleExpType) {
  return gs_lpStrExpFuncName[simpleExpType];
}

CXFA_FMSimpleExpression::CXFA_FMSimpleExpression(uint32_t line, XFA_FM_TOKEN op)
    : m_line(line), m_op(op) {}

bool CXFA_FMSimpleExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

bool CXFA_FMSimpleExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

XFA_FM_TOKEN CXFA_FMSimpleExpression::GetOperatorToken() const {
  return m_op;
}

CXFA_FMNullExpression::CXFA_FMNullExpression(uint32_t line)
    : CXFA_FMSimpleExpression(line, TOKnull) {}

bool CXFA_FMNullExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"null";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMNumberExpression::CXFA_FMNumberExpression(uint32_t line,
                                                 WideStringView wsNumber)
    : CXFA_FMSimpleExpression(line, TOKnumber), m_wsNumber(wsNumber) {}

CXFA_FMNumberExpression::~CXFA_FMNumberExpression() {}

bool CXFA_FMNumberExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << m_wsNumber;
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMStringExpression::CXFA_FMStringExpression(uint32_t line,
                                                 WideStringView wsString)
    : CXFA_FMSimpleExpression(line, TOKstring), m_wsString(wsString) {}

CXFA_FMStringExpression::~CXFA_FMStringExpression() {}

bool CXFA_FMStringExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempStr(m_wsString);
  if (tempStr.GetLength() <= 2) {
    javascript << tempStr;
    return !CXFA_IsTooBig(javascript);
  }
  javascript.AppendChar(L'\"');
  for (size_t i = 1; i < tempStr.GetLength() - 1; i++) {
    wchar_t oneChar = tempStr[i];
    switch (oneChar) {
      case L'\"':
        i++;
        javascript << L"\\\"";
        break;
      case 0x0d:
        break;
      case 0x0a:
        javascript << L"\\n";
        break;
      default:
        javascript.AppendChar(oneChar);
        break;
    }
  }
  javascript.AppendChar(L'\"');
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMIdentifierExpression::CXFA_FMIdentifierExpression(
    uint32_t line,
    WideStringView wsIdentifier)
    : CXFA_FMSimpleExpression(line, TOKidentifier),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMIdentifierExpression::~CXFA_FMIdentifierExpression() {}

bool CXFA_FMIdentifierExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempStr(m_wsIdentifier);
  if (tempStr == L"$") {
    tempStr = L"this";
  } else if (tempStr == L"!") {
    tempStr = L"xfa.datasets";
  } else if (tempStr == L"$data") {
    tempStr = L"xfa.datasets.data";
  } else if (tempStr == L"$event") {
    tempStr = L"xfa.event";
  } else if (tempStr == L"$form") {
    tempStr = L"xfa.form";
  } else if (tempStr == L"$host") {
    tempStr = L"xfa.host";
  } else if (tempStr == L"$layout") {
    tempStr = L"xfa.layout";
  } else if (tempStr == L"$template") {
    tempStr = L"xfa.template";
  } else if (tempStr[0] == L'!') {
    tempStr =
        EXCLAMATION_IN_IDENTIFIER + tempStr.Right(tempStr.GetLength() - 1);
  }
  javascript << tempStr;
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMUnaryExpression::CXFA_FMUnaryExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMSimpleExpression(line, op), m_pExp(std::move(pExp)) {}

CXFA_FMUnaryExpression::~CXFA_FMUnaryExpression() {}

bool CXFA_FMUnaryExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

CXFA_FMBinExpression::CXFA_FMBinExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMSimpleExpression(line, op),
      m_pExp1(std::move(pExp1)),
      m_pExp2(std::move(pExp2)) {}

CXFA_FMBinExpression::~CXFA_FMBinExpression() {}

bool CXFA_FMBinExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

CXFA_FMAssignExpression::CXFA_FMAssignExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAssignExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMOBJECT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L"))\n{\n";
  javascript << gs_lpStrExpFuncName[ASSIGN];
  javascript << L"(";
  javascript << tempExp1;
  javascript << L", ";

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L");\n}\n";
  if (m_pExp1->GetOperatorToken() == TOKidentifier &&
      tempExp1.AsStringView() != L"this") {
    javascript << L"else\n{\n";
    javascript << tempExp1;
    javascript << L" = ";
    javascript << gs_lpStrExpFuncName[ASSIGN];
    javascript << L"(";
    javascript << tempExp1;
    javascript << L", ";
    javascript << tempExp2;
    javascript << L");\n}\n";
  }
  return !CXFA_IsTooBig(javascript);
}

bool CXFA_FMAssignExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMOBJECT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L"))\n{\n";
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = ";
  javascript << gs_lpStrExpFuncName[ASSIGN];
  javascript << L"(";
  javascript << tempExp1;
  javascript << L", ";

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L");\n}\n";
  if (m_pExp1->GetOperatorToken() == TOKidentifier &&
      tempExp1.AsStringView() != L"this") {
    javascript << L"else\n{\n";
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
    javascript << L" = ";
    javascript << tempExp1;
    javascript << L" = ";
    javascript << gs_lpStrExpFuncName[ASSIGN];
    javascript << L"(";
    javascript << tempExp1;
    javascript << L", ";
    javascript << tempExp2;
    javascript << L");\n}\n";
  }
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMLogicalOrExpression::CXFA_FMLogicalOrExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalOrExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[LOGICALOR];
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMLogicalAndExpression::CXFA_FMLogicalAndExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalAndExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[LOGICALAND];
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMEqualityExpression::CXFA_FMEqualityExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMEqualityExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKeq:
    case TOKkseq:
      javascript << gs_lpStrExpFuncName[EQUALITY];
      break;
    case TOKne:
    case TOKksne:
      javascript << gs_lpStrExpFuncName[NOTEQUALITY];
      break;
    default:
      NOTREACHED();
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMRelationalExpression::CXFA_FMRelationalExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMRelationalExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKlt:
    case TOKkslt:
      javascript << gs_lpStrExpFuncName[LESS];
      break;
    case TOKgt:
    case TOKksgt:
      javascript << gs_lpStrExpFuncName[GREATER];
      break;
    case TOKle:
    case TOKksle:
      javascript << gs_lpStrExpFuncName[LESSEQUAL];
      break;
    case TOKge:
    case TOKksge:
      javascript << gs_lpStrExpFuncName[GREATEREQUAL];
      break;
    default:
      NOTREACHED();
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMAdditiveExpression::CXFA_FMAdditiveExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAdditiveExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKplus:
      javascript << gs_lpStrExpFuncName[PLUS];
      break;
    case TOKminus:
      javascript << gs_lpStrExpFuncName[MINUS];
      break;
    default:
      NOTREACHED();
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMMultiplicativeExpression::CXFA_FMMultiplicativeExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMMultiplicativeExpression::ToJavaScript(
    CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_op) {
    case TOKmul:
      javascript << gs_lpStrExpFuncName[MULTIPLE];
      break;
    case TOKdiv:
      javascript << gs_lpStrExpFuncName[DIVIDE];
      break;
    default:
      NOTREACHED();
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMPosExpression::CXFA_FMPosExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKplus, std::move(pExp)) {}

bool CXFA_FMPosExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[POSITIVE];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMNegExpression::CXFA_FMNegExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKminus, std::move(pExp)) {}

bool CXFA_FMNegExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[NEGATIVE];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMNotExpression::CXFA_FMNotExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKksnot, std::move(pExp)) {}

bool CXFA_FMNotExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[NOT];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
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

  auto cmpFunc = [](const wchar_t* iter, const WideString& val) -> bool {
    return val.CompareNoCase(iter) > 0;
  };
  WideString str = funcName->MakeString();
  const wchar_t* const* pMatchResult = std::lower_bound(
      std::begin(g_BuiltInFuncs), std::end(g_BuiltInFuncs), str, cmpFunc);
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
  auto cmpFunc = [](const XFA_FMSOMMethod iter, const WideString& val) {
    return val.Compare(iter.m_wsSomMethodName) > 0;
  };
  const XFA_FMSOMMethod* result =
      std::lower_bound(std::begin(gs_FMSomMethods), std::end(gs_FMSomMethods),
                       methodName, cmpFunc);
  if (result != std::end(gs_FMSomMethods) &&
      !methodName.Compare(result->m_wsSomMethodName)) {
    return result->m_dParameters;
  }
  return 0;
}

bool CXFA_FMCallExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  CFX_WideTextBuf funcName;
  if (!m_pExp->ToJavaScript(funcName))
    return false;
  if (m_bIsSomMethod) {
    javascript << funcName;
    javascript << L"(";
    uint32_t methodPara = IsMethodWithObjParam(funcName.MakeString());
    if (methodPara > 0) {
      for (size_t i = 0; i < m_Arguments.size(); ++i) {
        // Currently none of our expressions use objects for a parameter over
        // the 6th. Make sure we don't overflow the shift when doing this
        // check. If we ever need more the 32 object params we can revisit.
        if (i < 32 && (methodPara & (0x01 << i)) > 0) {
          javascript << gs_lpStrExpFuncName[GETFMJSOBJ];
        } else {
          javascript << gs_lpStrExpFuncName[GETFMVALUE];
        }
        javascript << L"(";
        const auto& expr = m_Arguments[i];
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L")";
        if (i + 1 < m_Arguments.size()) {
          javascript << L", ";
        }
      }
    } else {
      for (const auto& expr : m_Arguments) {
        javascript << gs_lpStrExpFuncName[GETFMVALUE];
        javascript << L"(";
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L")";
        if (expr != m_Arguments.back())
          javascript << L", ";
      }
    }
    javascript << L")";
  } else {
    bool isEvalFunc = false;
    bool isExistsFunc = false;
    if (IsBuiltInFunc(&funcName)) {
      if (funcName.AsStringView() == L"Eval") {
        isEvalFunc = true;
        javascript << L"eval.call(this, ";
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << L"Translate";
      } else if (funcName.AsStringView() == L"Exists") {
        isExistsFunc = true;
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << funcName;
      } else {
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << funcName;
      }
    } else {
      // If a function is not a SomMethod or a built-in then the input was
      // invalid, so failing. The scanner/lexer should catch this, but currently
      // doesn't. This failure will bubble up to the top-level and cause the
      // transpile to fail.
      return false;
    }
    javascript << L"(";
    if (isExistsFunc) {
      javascript << L"\n(\nfunction ()\n{\ntry\n{\n";
      if (!m_Arguments.empty()) {
        const auto& expr = m_Arguments[0];
        javascript << L"return ";
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L";\n}\n";
      } else {
        javascript << L"return 0;\n}\n";
      }
      javascript << L"catch(accessExceptions)\n";
      javascript << L"{\nreturn 0;\n}\n}\n).call(this)\n";
    } else {
      for (const auto& expr : m_Arguments) {
        if (!expr->ToJavaScript(javascript))
          return false;
        if (expr != m_Arguments.back())
          javascript << L", ";
      }
    }
    javascript << L")";
    if (isEvalFunc) {
      javascript << L")";
    }
  }
  return !CXFA_IsTooBig(javascript);
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

bool CXFA_FMDotAccessorExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[DOT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (m_pExp1) {
    if (!m_pExp1->ToJavaScript(tempExp1))
      return false;
    javascript << tempExp1;
  } else {
    javascript << L"null";
  }
  javascript << L", ";
  javascript << L"\"";

  if (m_pExp1 && m_pExp1->GetOperatorToken() == TOKidentifier)
    javascript << tempExp1;
  javascript << L"\", ";
  if (m_op == TOKdotscream) {
    javascript << L"\"#";
    javascript << m_wsIdentifier;
    javascript << L"\", ";
  } else if (m_op == TOKdotstar) {
    javascript << L"\"*\", ";
  } else if (m_op == TOKcall) {
    javascript << L"\"\", ";
  } else {
    javascript << L"\"";
    javascript << m_wsIdentifier;
    javascript << L"\", ";
  }
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMIndexExpression::CXFA_FMIndexExpression(
    uint32_t line,
    XFA_FM_AccessorIndex accessorIndex,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp,
    bool bIsStarIndex)
    : CXFA_FMUnaryExpression(line, TOKlbracket, std::move(pIndexExp)),
      m_accessorIndex(accessorIndex),
      m_bIsStarIndex(bIsStarIndex) {}

bool CXFA_FMIndexExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  switch (m_accessorIndex) {
    case ACCESSOR_NO_INDEX:
      javascript << L"0";
      break;
    case ACCESSOR_NO_RELATIVEINDEX:
      javascript << L"1";
      break;
    case ACCESSOR_POSITIVE_INDEX:
      javascript << L"2";
      break;
    case ACCESSOR_NEGATIVE_INDEX:
      javascript << L"3";
      break;
    default:
      javascript << L"0";
  }
  if (!m_bIsStarIndex) {
    javascript << L", ";
    if (m_pExp) {
      if (!m_pExp->ToJavaScript(javascript))
        return false;
    } else {
      javascript << L"0";
    }
  }
  return !CXFA_IsTooBig(javascript);
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

bool CXFA_FMDotDotAccessorExpression::ToJavaScript(
    CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << gs_lpStrExpFuncName[DOTDOT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L", ";
  javascript << L"\"";

  if (m_pExp1->GetOperatorToken() == TOKidentifier)
    javascript << tempExp1;
  javascript << L"\", ";
  javascript << L"\"";
  javascript << m_wsIdentifier;
  javascript << L"\", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMMethodCallExpression::CXFA_FMMethodCallExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessorExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pCallExp)
    : CXFA_FMBinExpression(line,
                           TOKdot,
                           std::move(pAccessorExp1),
                           std::move(pCallExp)) {}

bool CXFA_FMMethodCallExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"(\nfunction ()\n{\n";
  javascript << L"var method_return_value = null;\n";
  javascript << L"var accessor_object = ";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L";\n";
  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMARRAY];
  javascript << L"(accessor_object))\n{\n";
  javascript << L"for(var index = accessor_object.length - 1; index > 1; "
                L"index--)\n{\n";
  javascript << L"method_return_value = accessor_object[index].";

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L";\n}\n}\n";
  javascript << L"else\n{\nmethod_return_value = accessor_object.";
  javascript << tempExp2;
  javascript << L";\n}\n";
  javascript << L"return method_return_value;\n";
  javascript << L"}\n).call(this)";
  return !CXFA_IsTooBig(javascript);
}

bool CXFA_IsTooBig(const CFX_WideTextBuf& javascript) {
  return javascript.GetSize() >= 256 * 1024 * 1024;
}
