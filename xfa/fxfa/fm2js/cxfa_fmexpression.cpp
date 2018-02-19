// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmexpression.h"

#include <utility>

#include "core/fxcrt/cfx_widetextbuf.h"
#include "xfa/fxfa/fm2js/cxfa_fmsimpleexpression.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

namespace {

const wchar_t kLessEqual[] = L" <= ";
const wchar_t kGreaterEqual[] = L" >= ";
const wchar_t kPlusEqual[] = L" += ";
const wchar_t kMinusEqual[] = L" -= ";

}  // namespace

CXFA_FMExpression::CXFA_FMExpression(uint32_t line)
    : m_type(XFA_FM_EXPTYPE_UNKNOWN), m_line(line) {}

CXFA_FMExpression::CXFA_FMExpression(uint32_t line, XFA_FM_EXPTYPE type)
    : m_type(type), m_line(line) {}

bool CXFA_FMExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                     ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    uint32_t line,
    bool isGlobal,
    const WideStringView& wsName,
    std::vector<WideStringView>&& arguments,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_FUNC),
      m_wsName(wsName),
      m_pArguments(std::move(arguments)),
      m_pExpressions(std::move(expressions)),
      m_isGlobal(isGlobal) {}

CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() {}

bool CXFA_FMFunctionDefinition::ToJavaScript(CFX_WideTextBuf& javascript,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (type == ReturnType::kImplied) {
    return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
  }

  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (m_isGlobal && m_pExpressions.empty()) {
    javascript << L"// comments only";
    return !CXFA_IsTooBig(javascript);
  }
  if (m_isGlobal) {
    javascript << L"(\n";
  }
  javascript << L"function ";
  if (!m_wsName.IsEmpty() && m_wsName[0] == L'!') {
    WideString tempName =
        L"pfm__excl__" + m_wsName.Right(m_wsName.GetLength() - 1);
    javascript << tempName;
  } else {
    javascript << m_wsName;
  }
  javascript << L"(";
  bool bNeedComma = false;
  for (const auto& identifier : m_pArguments) {
    if (bNeedComma)
      javascript << L", ";
    if (identifier[0] == L'!') {
      WideString tempIdentifier =
          L"pfm__excl__" + identifier.Right(identifier.GetLength() - 1);
      javascript << tempIdentifier;
    } else {
      javascript << identifier;
    }
    bNeedComma = true;
  }
  javascript << L")\n{\n";
  javascript << L"var pfm_ret = null;\n";
  for (const auto& expr : m_pExpressions) {
    bool ret;
    if (expr == m_pExpressions.back())
      ret = expr->ToJavaScript(javascript, ReturnType::kImplied);
    else
      ret = expr->ToJavaScript(javascript, ReturnType::kInfered);

    if (!ret)
      return false;
  }
  javascript << L"return ";
  if (m_isGlobal)
    javascript << L"pfm_rt.get_val(pfm_ret)";
  else
    javascript << L"pfm_ret";

  javascript << L";\n}\n";
  if (m_isGlobal) {
    javascript << L").call(this);\n";
  }
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMVarExpression::CXFA_FMVarExpression(
    uint32_t line,
    const WideStringView& wsName,
    std::unique_ptr<CXFA_FMExpression> pInit)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_VAR),
      m_wsName(wsName),
      m_pInit(std::move(pInit)) {}

CXFA_FMVarExpression::~CXFA_FMVarExpression() {}

bool CXFA_FMVarExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                        ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"var ";
  WideString tempName(m_wsName);
  if (m_wsName[0] == L'!') {
    tempName = L"pfm__excl__" + m_wsName.Right(m_wsName.GetLength() - 1);
  }
  javascript << tempName;
  javascript << L" = ";
  if (m_pInit) {
    if (!m_pInit->ToJavaScript(javascript, ReturnType::kInfered))
      return false;
    javascript << tempName;
    javascript << L" = pfm_rt.var_filter(";
    javascript << tempName;
    javascript << L");\n";
  } else {
    javascript << L"\"\";\n";
  }

  if (type == ReturnType::kImplied) {
    javascript << L"pfm_ret = ";
    javascript << tempName;
    javascript << L";\n";
  }
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMExpExpression::CXFA_FMExpExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_EXP),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMExpExpression::~CXFA_FMExpExpression() {}

bool CXFA_FMExpExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                        ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kInfered) {
    bool ret = m_pExpression->ToJavaScript(javascript, ReturnType::kInfered);
    if (m_pExpression->GetOperatorToken() != TOKassign)
      javascript << L";\n";
    return ret;
  }

  if (m_pExpression->GetOperatorToken() == TOKassign)
    return m_pExpression->ToJavaScript(javascript, ReturnType::kImplied);

  if (m_pExpression->GetOperatorToken() == TOKstar ||
      m_pExpression->GetOperatorToken() == TOKdotstar ||
      m_pExpression->GetOperatorToken() == TOKdotscream ||
      m_pExpression->GetOperatorToken() == TOKdotdot ||
      m_pExpression->GetOperatorToken() == TOKdot) {
    javascript << L"pfm_ret = pfm_rt.get_val(";
    if (!m_pExpression->ToJavaScript(javascript, ReturnType::kInfered))
      return false;
    javascript << L");\n";
    return !CXFA_IsTooBig(javascript);
  }

  javascript << L"pfm_ret = ";
  if (!m_pExpression->ToJavaScript(javascript, ReturnType::kInfered))
    return false;
  javascript << L";\n";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMBlockExpression::CXFA_FMBlockExpression(
    uint32_t line,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BLOCK),
      m_ExpressionList(std::move(pExpressionList)) {}

CXFA_FMBlockExpression::~CXFA_FMBlockExpression() {}

bool CXFA_FMBlockExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"{\n";
  for (const auto& expr : m_ExpressionList) {
    if (type == ReturnType::kInfered) {
      if (!expr->ToJavaScript(javascript, ReturnType::kInfered))
        return false;
    } else {
      bool ret;
      if (expr == m_ExpressionList.back())
        ret = expr->ToJavaScript(javascript, ReturnType::kImplied);
      else
        ret = expr->ToJavaScript(javascript, ReturnType::kInfered);

      if (!ret)
        return false;
    }
  }
  javascript << L"}\n";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMDoExpression::CXFA_FMDoExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(line), m_pList(std::move(pList)) {}

CXFA_FMDoExpression::~CXFA_FMDoExpression() {}

bool CXFA_FMDoExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                       ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  return m_pList->ToJavaScript(javascript, type);
}

CXFA_FMIfExpression::CXFA_FMIfExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression,
    std::unique_ptr<CXFA_FMExpression> pIfExpression,
    std::unique_ptr<CXFA_FMExpression> pElseExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_IF),
      m_pExpression(std::move(pExpression)),
      m_pIfExpression(std::move(pIfExpression)),
      m_pElseExpression(std::move(pElseExpression)) {}

CXFA_FMIfExpression::~CXFA_FMIfExpression() {}

bool CXFA_FMIfExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                       ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    javascript << L"pfm_ret = 0;\n";

  javascript << L"if (";
  if (m_pExpression) {
    javascript << L"pfm_rt.get_val(";
    if (!m_pExpression->ToJavaScript(javascript, ReturnType::kInfered))
      return false;
    javascript << L")";
  }
  javascript << L")\n";
  if (CXFA_IsTooBig(javascript))
    return false;

  if (m_pIfExpression) {
    if (!m_pIfExpression->ToJavaScript(javascript, type))
      return false;
    if (CXFA_IsTooBig(javascript))
      return false;
  }

  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      javascript << L"else\n";
      javascript << L"{\n";
      if (!m_pElseExpression->ToJavaScript(javascript, type))
        return false;
      javascript << L"}\n";
    } else {
      javascript << L"else\n";
      if (!m_pElseExpression->ToJavaScript(javascript, type))
        return false;
    }
  }
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMLoopExpression::~CXFA_FMLoopExpression() {}

bool CXFA_FMLoopExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                         ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  return !CXFA_IsTooBig(javascript) && depthManager.IsWithinMaxDepth();
}

CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pCondition,
    std::unique_ptr<CXFA_FMExpression> pExpression)
    : CXFA_FMLoopExpression(line),
      m_pCondition(std::move(pCondition)),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMWhileExpression::~CXFA_FMWhileExpression() {}

bool CXFA_FMWhileExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    javascript << L"pfm_ret = 0;\n";

  javascript << L"while (";
  if (!m_pCondition->ToJavaScript(javascript, ReturnType::kInfered))
    return false;
  javascript << L")\n";
  if (CXFA_IsTooBig(javascript))
    return false;

  if (!m_pExpression->ToJavaScript(javascript, type))
    return false;
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMBreakExpression::CXFA_FMBreakExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BREAK) {}

CXFA_FMBreakExpression::~CXFA_FMBreakExpression() {}

bool CXFA_FMBreakExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"pfm_ret = 0;\n";
  javascript << L"break;\n";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMContinueExpression::CXFA_FMContinueExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_CONTINUE) {}

CXFA_FMContinueExpression::~CXFA_FMContinueExpression() {}

bool CXFA_FMContinueExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  javascript << L"pfm_ret = 0;\n";
  javascript << L"continue;\n";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMForExpression::CXFA_FMForExpression(
    uint32_t line,
    const WideStringView& wsVariant,
    std::unique_ptr<CXFA_FMSimpleExpression> pAssignment,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    int32_t iDirection,
    std::unique_ptr<CXFA_FMSimpleExpression> pStep,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMLoopExpression(line),
      m_wsVariant(wsVariant),
      m_pAssignment(std::move(pAssignment)),
      m_pAccessor(std::move(pAccessor)),
      m_bDirection(iDirection == 1),
      m_pStep(std::move(pStep)),
      m_pList(std::move(pList)) {}

CXFA_FMForExpression::~CXFA_FMForExpression() {}

bool CXFA_FMForExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                        ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    javascript << L"pfm_ret = 0;\n";

  javascript << L"{\nvar ";
  WideString tempVariant;
  if (m_wsVariant[0] == L'!') {
    tempVariant =
        L"pfm__excl__" + m_wsVariant.Right(m_wsVariant.GetLength() - 1);
    javascript << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    javascript << m_wsVariant;
  }
  javascript << L" = null;\n";
  javascript << L"for (";
  javascript << tempVariant;
  javascript << L" = pfm_rt.get_val(";
  if (!m_pAssignment->ToJavaScript(javascript, ReturnType::kInfered))
    return false;
  javascript << L"); ";
  javascript << tempVariant;

  javascript << (m_bDirection ? kLessEqual : kGreaterEqual);
  javascript << L"pfm_rt.get_val(";
  if (!m_pAccessor->ToJavaScript(javascript, ReturnType::kInfered))
    return false;
  javascript << L"); ";
  javascript << tempVariant;
  javascript << L" += ";
  javascript << (m_bDirection ? kPlusEqual : kMinusEqual);
  if (CXFA_IsTooBig(javascript))
    return false;

  if (m_pStep) {
    javascript << L"pfm_rt.get_val(";
    if (!m_pStep->ToJavaScript(javascript, ReturnType::kInfered))
      return false;
    javascript << L")";
  } else {
    javascript << L"1";
  }
  if (CXFA_IsTooBig(javascript))
    return false;

  javascript << L")\n";
  if (!m_pList->ToJavaScript(javascript, type))
    return false;
  javascript << L"}\n";
  return !CXFA_IsTooBig(javascript);
}

CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    uint32_t line,
    const WideStringView& wsIdentifier,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMLoopExpression(line),
      m_wsIdentifier(wsIdentifier),
      m_pAccessors(std::move(pAccessors)),
      m_pList(std::move(pList)) {}

CXFA_FMForeachExpression::~CXFA_FMForeachExpression() {}

bool CXFA_FMForeachExpression::ToJavaScript(CFX_WideTextBuf& javascript,
                                            ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(javascript) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    javascript << L"pfm_ret = 0;\n";

  javascript << L"{\n";
  javascript << L"var ";
  if (m_wsIdentifier[0] == L'!') {
    WideString tempIdentifier =
        L"pfm__excl__" + m_wsIdentifier.Right(m_wsIdentifier.GetLength() - 1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = null;\n";
  javascript << L"var pfm_ary = pfm_rt.concat_obj(";

  for (const auto& expr : m_pAccessors) {
    if (!expr->ToJavaScript(javascript, ReturnType::kInfered))
      return false;
    if (expr != m_pAccessors.back())
      javascript << L", ";
  }
  javascript << L");\n";
  javascript << L"var pfm_ary_idx = 0;\n";
  javascript << L"while(pfm_ary_idx < pfm_ary.length)\n{\n";
  if (m_wsIdentifier[0] == L'!') {
    WideString tempIdentifier =
        L"pfm__excl__" + m_wsIdentifier.Right(m_wsIdentifier.GetLength() - 1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = pfm_ary[pfm_ary_idx++];\n";
  if (!m_pList->ToJavaScript(javascript, type))
    return false;
  javascript << L"}\n";
  javascript << L"}\n";
  return !CXFA_IsTooBig(javascript);
}
