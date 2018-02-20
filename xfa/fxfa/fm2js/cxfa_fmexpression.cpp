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

CXFA_FMExpression::CXFA_FMExpression() = default;

CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    bool isGlobal,
    const WideStringView& wsName,
    std::vector<WideStringView>&& arguments,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions)
    : CXFA_FMExpression(),
      m_wsName(wsName),
      m_pArguments(std::move(arguments)),
      m_pExpressions(std::move(expressions)),
      m_isGlobal(isGlobal) {}

CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() {}

bool CXFA_FMFunctionDefinition::ToJavaScript(CFX_WideTextBuf& js,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (type == ReturnType::kImplied)
    return !CXFA_IsTooBig(js) && depthManager.IsWithinMaxDepth();

  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (m_isGlobal && m_pExpressions.empty()) {
    js << L"// comments only";
    return !CXFA_IsTooBig(js);
  }
  if (m_isGlobal)
    js << L"(\n";

  js << L"function ";
  if (!m_wsName.IsEmpty() && m_wsName[0] == L'!') {
    WideString tempName =
        L"pfm__excl__" + m_wsName.Right(m_wsName.GetLength() - 1);
    js << tempName;
  } else {
    js << m_wsName;
  }

  js << L"(";
  bool bNeedComma = false;
  for (const auto& identifier : m_pArguments) {
    if (bNeedComma)
      js << L", ";
    if (identifier[0] == L'!') {
      WideString tempIdentifier =
          L"pfm__excl__" + identifier.Right(identifier.GetLength() - 1);
      js << tempIdentifier;
    } else {
      js << identifier;
    }
    bNeedComma = true;
  }
  js << L")\n{\n";

  js << L"var pfm_ret = null;\n";
  for (const auto& expr : m_pExpressions) {
    ReturnType ret_type = expr == m_pExpressions.back() ? ReturnType::kImplied
                                                        : ReturnType::kInfered;
    if (!expr->ToJavaScript(js, ret_type))
      return false;
  }

  js << L"return ";
  if (m_isGlobal)
    js << L"pfm_rt.get_val(pfm_ret);\n";
  else
    js << L"pfm_ret;\n";

  js << L"}\n";
  if (m_isGlobal)
    js << L").call(this);\n";

  return !CXFA_IsTooBig(js);
}

CXFA_FMVarExpression::CXFA_FMVarExpression(
    const WideStringView& wsName,
    std::unique_ptr<CXFA_FMSimpleExpression> pInit)
    : CXFA_FMExpression(), m_wsName(wsName), m_pInit(std::move(pInit)) {}

CXFA_FMVarExpression::~CXFA_FMVarExpression() {}

bool CXFA_FMVarExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  WideString tempName(m_wsName);
  if (m_wsName[0] == L'!')
    tempName = L"pfm__excl__" + m_wsName.Right(m_wsName.GetLength() - 1);

  js << L"var " << tempName << L" = ";
  if (m_pInit) {
    if (!m_pInit->ToJavaScript(js, ReturnType::kInfered))
      return false;

    js << tempName << L" = pfm_rt.var_filter(" << tempName << L");\n";
  } else {
    js << L"\"\";\n";
  }

  if (type == ReturnType::kImplied)
    js << L"pfm_ret = " << tempName << L";\n";

  return !CXFA_IsTooBig(js);
}

CXFA_FMExpExpression::CXFA_FMExpExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression)
    : CXFA_FMExpression(), m_pExpression(std::move(pExpression)) {}

CXFA_FMExpExpression::~CXFA_FMExpExpression() {}

bool CXFA_FMExpExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kInfered) {
    bool ret = m_pExpression->ToJavaScript(js, ReturnType::kInfered);
    if (m_pExpression->GetOperatorToken() != TOKassign)
      js << L";\n";

    return ret;
  }

  if (m_pExpression->GetOperatorToken() == TOKassign)
    return m_pExpression->ToJavaScript(js, ReturnType::kImplied);

  if (m_pExpression->GetOperatorToken() == TOKstar ||
      m_pExpression->GetOperatorToken() == TOKdotstar ||
      m_pExpression->GetOperatorToken() == TOKdotscream ||
      m_pExpression->GetOperatorToken() == TOKdotdot ||
      m_pExpression->GetOperatorToken() == TOKdot) {
    js << L"pfm_ret = pfm_rt.get_val(";
    if (!m_pExpression->ToJavaScript(js, ReturnType::kInfered))
      return false;

    js << L");\n";
    return !CXFA_IsTooBig(js);
  }

  js << L"pfm_ret = ";
  if (!m_pExpression->ToJavaScript(js, ReturnType::kInfered))
    return false;

  js << L";\n";
  return !CXFA_IsTooBig(js);
}

CXFA_FMBlockExpression::CXFA_FMBlockExpression(
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList)
    : CXFA_FMExpression(), m_ExpressionList(std::move(pExpressionList)) {}

CXFA_FMBlockExpression::~CXFA_FMBlockExpression() {}

bool CXFA_FMBlockExpression::ToJavaScript(CFX_WideTextBuf& js,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << L"{\n";
  for (const auto& expr : m_ExpressionList) {
    if (type == ReturnType::kInfered) {
      if (!expr->ToJavaScript(js, ReturnType::kInfered))
        return false;
    } else {
      ReturnType ret_type = expr == m_ExpressionList.back()
                                ? ReturnType::kImplied
                                : ReturnType::kInfered;
      if (!expr->ToJavaScript(js, ret_type))
        return false;
    }
  }
  js << L"}\n";

  return !CXFA_IsTooBig(js);
}

CXFA_FMDoExpression::CXFA_FMDoExpression(
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(), m_pList(std::move(pList)) {}

CXFA_FMDoExpression::~CXFA_FMDoExpression() {}

bool CXFA_FMDoExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  return m_pList->ToJavaScript(js, type);
}

CXFA_FMIfExpression::CXFA_FMIfExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression,
    std::unique_ptr<CXFA_FMExpression> pIfExpression,
    std::unique_ptr<CXFA_FMExpression> pElseExpression)
    : CXFA_FMExpression(),
      m_pExpression(std::move(pExpression)),
      m_pIfExpression(std::move(pIfExpression)),
      m_pElseExpression(std::move(pElseExpression)) {}

CXFA_FMIfExpression::~CXFA_FMIfExpression() {}

bool CXFA_FMIfExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    js << L"pfm_ret = 0;\n";

  js << L"if (";
  if (m_pExpression) {
    js << L"pfm_rt.get_val(";
    if (!m_pExpression->ToJavaScript(js, ReturnType::kInfered))
      return false;

    js << L")";
  }
  js << L")\n";

  if (CXFA_IsTooBig(js))
    return false;

  if (m_pIfExpression) {
    if (!m_pIfExpression->ToJavaScript(js, type))
      return false;
    if (CXFA_IsTooBig(js))
      return false;
  }

  if (m_pElseExpression) {
    js << L"else\n{\n";
    if (!m_pElseExpression->ToJavaScript(js, type))
      return false;
    js << L"}\n";
  }
  return !CXFA_IsTooBig(js);
}

CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> pCondition,
    std::unique_ptr<CXFA_FMExpression> pExpression)
    : CXFA_FMExpression(),
      m_pCondition(std::move(pCondition)),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMWhileExpression::~CXFA_FMWhileExpression() {}

bool CXFA_FMWhileExpression::ToJavaScript(CFX_WideTextBuf& js,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    js << L"pfm_ret = 0;\n";

  js << L"while (";
  if (!m_pCondition->ToJavaScript(js, ReturnType::kInfered))
    return false;

  js << L")\n";
  if (CXFA_IsTooBig(js))
    return false;

  if (!m_pExpression->ToJavaScript(js, type))
    return false;

  return !CXFA_IsTooBig(js);
}

CXFA_FMBreakExpression::CXFA_FMBreakExpression() : CXFA_FMExpression() {}

CXFA_FMBreakExpression::~CXFA_FMBreakExpression() {}

bool CXFA_FMBreakExpression::ToJavaScript(CFX_WideTextBuf& js,
                                          ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << L"pfm_ret = 0;\nbreak;\n";
  return !CXFA_IsTooBig(js);
}

CXFA_FMContinueExpression::CXFA_FMContinueExpression() : CXFA_FMExpression() {}

CXFA_FMContinueExpression::~CXFA_FMContinueExpression() {}

bool CXFA_FMContinueExpression::ToJavaScript(CFX_WideTextBuf& js,
                                             ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  js << L"pfm_ret = 0;\ncontinue;\n";
  return !CXFA_IsTooBig(js);
}

CXFA_FMForExpression::CXFA_FMForExpression(
    const WideStringView& wsVariant,
    std::unique_ptr<CXFA_FMSimpleExpression> pAssignment,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    int32_t iDirection,
    std::unique_ptr<CXFA_FMSimpleExpression> pStep,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(),
      m_wsVariant(wsVariant),
      m_pAssignment(std::move(pAssignment)),
      m_pAccessor(std::move(pAccessor)),
      m_bDirection(iDirection == 1),
      m_pStep(std::move(pStep)),
      m_pList(std::move(pList)) {}

CXFA_FMForExpression::~CXFA_FMForExpression() {}

bool CXFA_FMForExpression::ToJavaScript(CFX_WideTextBuf& js, ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    js << L"pfm_ret = 0;\n";

  js << L"{\n";

  WideString tmpName;
  if (m_wsVariant[0] == L'!')
    tmpName = L"pfm__excl__" + m_wsVariant.Right(m_wsVariant.GetLength() - 1);
  else
    tmpName = m_wsVariant;

  js << L"var " << tmpName << L" = null;\n";

  CFX_WideTextBuf assign_txt;
  if (!m_pAssignment->ToJavaScript(assign_txt, ReturnType::kInfered))
    return false;

  CFX_WideTextBuf accessor_txt;
  if (!m_pAccessor->ToJavaScript(accessor_txt, ReturnType::kInfered))
    return false;

  js << L"for (" << tmpName << L" = pfm_rt.get_val(" << assign_txt << L"); ";
  js << tmpName << (m_bDirection ? kLessEqual : kGreaterEqual);
  js << L"pfm_rt.get_val(" << accessor_txt << L"); ";
  js << tmpName << (m_bDirection ? kPlusEqual : kMinusEqual);
  if (m_pStep) {
    CFX_WideTextBuf step_txt;
    if (!m_pStep->ToJavaScript(step_txt, ReturnType::kInfered))
      return false;

    js << L"pfm_rt.get_val(" << step_txt << L")";
  } else {
    js << L"1";
  }
  js << L")\n";
  if (CXFA_IsTooBig(js))
    return false;

  if (!m_pList->ToJavaScript(js, type))
    return false;

  js << L"}\n";
  return !CXFA_IsTooBig(js);
}

CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    const WideStringView& wsIdentifier,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(),
      m_wsIdentifier(wsIdentifier),
      m_pAccessors(std::move(pAccessors)),
      m_pList(std::move(pList)) {}

CXFA_FMForeachExpression::~CXFA_FMForeachExpression() {}

bool CXFA_FMForeachExpression::ToJavaScript(CFX_WideTextBuf& js,
                                            ReturnType type) {
  CXFA_FMToJavaScriptDepth depthManager;
  if (CXFA_IsTooBig(js) || !depthManager.IsWithinMaxDepth())
    return false;

  if (type == ReturnType::kImplied)
    js << L"pfm_ret = 0;\n";

  js << L"{\n";

  WideString tmpName;
  if (m_wsIdentifier[0] == L'!') {
    tmpName =
        L"pfm__excl__" + m_wsIdentifier.Right(m_wsIdentifier.GetLength() - 1);
  } else {
    tmpName = m_wsIdentifier;
  }

  js << L"var " << tmpName << L" = null;\n";
  js << L"var pfm_ary = pfm_rt.concat_obj(";
  for (const auto& expr : m_pAccessors) {
    if (!expr->ToJavaScript(js, ReturnType::kInfered))
      return false;
    if (expr != m_pAccessors.back())
      js << L", ";
  }
  js << L");\n";

  js << L"var pfm_ary_idx = 0;\n";
  js << L"while(pfm_ary_idx < pfm_ary.length)\n{\n";
  js << tmpName << L" = pfm_ary[pfm_ary_idx++];\n";
  if (!m_pList->ToJavaScript(js, type))
    return false;
  js << L"}\n";  // while

  js << L"}\n";  // block
  return !CXFA_IsTooBig(js);
}
