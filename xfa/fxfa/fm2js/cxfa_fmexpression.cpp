// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmexpression.h"

#include <utility>

#include "core/fxcrt/fx_basic.h"

namespace {

const wchar_t RUNTIMEBLOCKTEMPARRAY[] = L"pfm_ary";

const wchar_t RUNTIMEBLOCKTEMPARRAYINDEX[] = L"pfm_ary_idx";

const wchar_t kLessEqual[] = L" <= ";
const wchar_t kGreaterEqual[] = L" >= ";
const wchar_t kPlusEqual[] = L" += ";
const wchar_t kMinusEqual[] = L" -= ";

}  // namespace

CXFA_FMExpression::CXFA_FMExpression(uint32_t line)
    : m_type(XFA_FM_EXPTYPE_UNKNOWN), m_line(line) {}

CXFA_FMExpression::CXFA_FMExpression(uint32_t line, XFA_FM_EXPTYPE type)
    : m_type(type), m_line(line) {}

bool CXFA_FMExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return true;
}

bool CXFA_FMExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  return true;
}

CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    uint32_t line,
    bool isGlobal,
    const CFX_WideStringC& wsName,
    std::vector<CFX_WideStringC>&& arguments,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_FUNC),
      m_wsName(wsName),
      m_pArguments(std::move(arguments)),
      m_pExpressions(std::move(expressions)),
      m_isGlobal(isGlobal) {}

CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() {}

bool CXFA_FMFunctionDefinition::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (m_isGlobal && m_pExpressions.empty()) {
    javascript << L"// comments only";
    return true;
  }
  if (m_isGlobal) {
    javascript << L"(\n";
  }
  javascript << L"function ";
  if (m_wsName.GetAt(0) == L'!') {
    CFX_WideString tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
    javascript << tempName;
  } else {
    javascript << m_wsName;
  }
  javascript << L"(";
  bool bNeedComma = false;
  for (const auto& identifier : m_pArguments) {
    if (bNeedComma)
      javascript << L", ";
    if (identifier.GetAt(0) == L'!') {
      CFX_WideString tempIdentifier =
          EXCLAMATION_IN_IDENTIFIER + identifier.Mid(1);
      javascript << tempIdentifier;
    } else {
      javascript << identifier;
    }
    bNeedComma = true;
  }
  javascript << L")\n{\n";
  javascript << L"var ";
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = null;\n";
  for (const auto& expr : m_pExpressions) {
    bool ret;
    if (expr == m_pExpressions.back())
      ret = expr->ToImpliedReturnJS(javascript);
    else
      ret = expr->ToJavaScript(javascript);
    if (!ret || CFXA_IsTooBig(javascript))
      return false;
  }
  javascript << L"return ";
  if (m_isGlobal) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
    javascript << L")";
  } else {
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
  }
  javascript << L";\n}\n";
  if (m_isGlobal) {
    javascript << L").call(this);\n";
  }
  return true;
}

bool CXFA_FMFunctionDefinition::ToImpliedReturnJS(CFX_WideTextBuf&) {
  return true;
}

CXFA_FMVarExpression::CXFA_FMVarExpression(
    uint32_t line,
    const CFX_WideStringC& wsName,
    std::unique_ptr<CXFA_FMExpression> pInit)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_VAR),
      m_wsName(wsName),
      m_pInit(std::move(pInit)) {}

CXFA_FMVarExpression::~CXFA_FMVarExpression() {}

bool CXFA_FMVarExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"var ";
  CFX_WideString tempName(m_wsName);
  if (m_wsName.GetAt(0) == L'!') {
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  }
  javascript << tempName;
  javascript << L" = ";
  if (m_pInit) {
    if (!m_pInit->ToJavaScript(javascript))
      return false;
    javascript << tempName;
    javascript << L" = ";
    javascript << XFA_FM_EXPTypeToString(VARFILTER);
    javascript << L"(";
    javascript << tempName;
    javascript << L");\n";
  } else {
    javascript << L"\"\";\n";
  }
  return true;
}

bool CXFA_FMVarExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << L"var ";
  CFX_WideString tempName(m_wsName);
  if (m_wsName.GetAt(0) == L'!') {
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  }
  javascript << tempName;
  javascript << L" = ";
  if (m_pInit) {
    if (!m_pInit->ToJavaScript(javascript))
      return false;
    javascript << tempName;
    javascript << L" = ";
    javascript << XFA_FM_EXPTypeToString(VARFILTER);
    javascript << L"(";
    javascript << tempName;
    javascript << L");\n";
  } else {
    javascript << L"\"\";\n";
  }
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = ";
  javascript << tempName;
  javascript << L";\n";
  return true;
}

CXFA_FMExpExpression::CXFA_FMExpExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_EXP),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMExpExpression::~CXFA_FMExpExpression() {}

bool CXFA_FMExpExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  bool ret = m_pExpression->ToJavaScript(javascript);
  if (m_pExpression->GetOperatorToken() != TOKassign)
    javascript << L";\n";
  return ret;
}

bool CXFA_FMExpExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (m_pExpression->GetOperatorToken() == TOKassign)
    return m_pExpression->ToImpliedReturnJS(javascript);

  if (m_pExpression->GetOperatorToken() == TOKstar ||
      m_pExpression->GetOperatorToken() == TOKdotstar ||
      m_pExpression->GetOperatorToken() == TOKdotscream ||
      m_pExpression->GetOperatorToken() == TOKdotdot ||
      m_pExpression->GetOperatorToken() == TOKdot) {
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
    javascript << L" = ";
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    if (!m_pExpression->ToJavaScript(javascript))
      return false;
    javascript << L");\n";
    return true;
  }

  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = ";
  if (!m_pExpression->ToJavaScript(javascript))
    return false;
  javascript << L";\n";
  return true;
}

CXFA_FMBlockExpression::CXFA_FMBlockExpression(
    uint32_t line,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BLOCK),
      m_ExpressionList(std::move(pExpressionList)) {}

CXFA_FMBlockExpression::~CXFA_FMBlockExpression() {}

bool CXFA_FMBlockExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"{\n";
  for (const auto& expr : m_ExpressionList) {
    if (!expr->ToJavaScript(javascript) || CFXA_IsTooBig(javascript))
      return false;
  }
  javascript << L"}\n";
  return true;
}

bool CXFA_FMBlockExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << L"{\n";
  for (const auto& expr : m_ExpressionList) {
    bool ret;
    if (expr == m_ExpressionList.back())
      ret = expr->ToImpliedReturnJS(javascript);
    else
      ret = expr->ToJavaScript(javascript);
    if (!ret || CFXA_IsTooBig(javascript))
      return false;
  }
  javascript << L"}\n";
  return true;
}

CXFA_FMDoExpression::CXFA_FMDoExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(line), m_pList(std::move(pList)) {}

CXFA_FMDoExpression::~CXFA_FMDoExpression() {}

bool CXFA_FMDoExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return m_pList->ToJavaScript(javascript);
}

bool CXFA_FMDoExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  return m_pList->ToImpliedReturnJS(javascript);
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

bool CXFA_FMIfExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"if (";
  if (m_pExpression) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    if (!m_pExpression->ToJavaScript(javascript))
      return false;
    javascript << L")";
  }
  javascript << L")\n";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pIfExpression) {
    if (!m_pIfExpression->ToJavaScript(javascript))
      return false;
    if (CFXA_IsTooBig(javascript))
      return false;
  }

  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      javascript << L"else\n";
      javascript << L"{\n";
      if (!m_pElseExpression->ToJavaScript(javascript))
        return false;
      javascript << L"}\n";
    } else {
      javascript << L"else\n";
      if (!m_pElseExpression->ToJavaScript(javascript))
        return false;
    }
  }
  return !CFXA_IsTooBig(javascript);
}

bool CXFA_FMIfExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"if (";
  if (m_pExpression) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    if (!m_pExpression->ToJavaScript(javascript))
      return false;
    javascript << L")";
  }
  javascript << L")\n";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pIfExpression) {
    if (!m_pIfExpression->ToImpliedReturnJS(javascript))
      return false;
    if (CFXA_IsTooBig(javascript))
      return false;
  }
  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      javascript << L"else\n";
      javascript << L"{\n";
      if (!m_pElseExpression->ToImpliedReturnJS(javascript))
        return false;
      javascript << L"}\n";
    } else {
      javascript << L"else\n";
      if (!m_pElseExpression->ToImpliedReturnJS(javascript))
        return false;
    }
  }
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMLoopExpression::~CXFA_FMLoopExpression() {}

bool CXFA_FMLoopExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return true;
}

bool CXFA_FMLoopExpression::ToImpliedReturnJS(CFX_WideTextBuf&) {
  return true;
}

CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pCondition,
    std::unique_ptr<CXFA_FMExpression> pExpression)
    : CXFA_FMLoopExpression(line),
      m_pCondition(std::move(pCondition)),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMWhileExpression::~CXFA_FMWhileExpression() {}

bool CXFA_FMWhileExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"while (";
  if (!m_pCondition->ToJavaScript(javascript))
    return false;
  javascript << L")\n";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (!m_pExpression->ToJavaScript(javascript))
    return false;
  return !CFXA_IsTooBig(javascript);
}

bool CXFA_FMWhileExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"while (";
  if (!m_pCondition->ToJavaScript(javascript))
    return false;
  javascript << L")\n";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (!m_pExpression->ToImpliedReturnJS(javascript))
    return false;
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMBreakExpression::CXFA_FMBreakExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BREAK) {}

CXFA_FMBreakExpression::~CXFA_FMBreakExpression() {}

bool CXFA_FMBreakExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"break;\n";
  return true;
}

bool CXFA_FMBreakExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"break;\n";
  return true;
}

CXFA_FMContinueExpression::CXFA_FMContinueExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_CONTINUE) {}

CXFA_FMContinueExpression::~CXFA_FMContinueExpression() {}

bool CXFA_FMContinueExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"continue;\n";
  return true;
}

bool CXFA_FMContinueExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"continue;\n";
  return true;
}

CXFA_FMForExpression::CXFA_FMForExpression(
    uint32_t line,
    const CFX_WideStringC& wsVariant,
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

bool CXFA_FMForExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"{\nvar ";
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    javascript << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    javascript << m_wsVariant;
  }
  javascript << L" = null;\n";
  javascript << L"for (";
  javascript << tempVariant;
  javascript << L" = ";
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << L"(";
  if (!m_pAssignment->ToJavaScript(javascript))
    return false;
  javascript << L"); ";
  javascript << tempVariant;
  if (CFXA_IsTooBig(javascript))
    return false;

  javascript << (m_bDirection ? kLessEqual : kGreaterEqual);
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << L"(";
  if (!m_pAccessor->ToJavaScript(javascript))
    return false;
  javascript << L"); ";
  javascript << tempVariant;
  javascript << (m_bDirection ? kPlusEqual : kMinusEqual);
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pStep) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    if (!m_pStep->ToJavaScript(javascript))
      return false;
    javascript << L")";
    if (CFXA_IsTooBig(javascript))
      return false;
  } else {
    javascript << L"1";
  }
  javascript << L")\n";
  if (!m_pList->ToJavaScript(javascript))
    return false;
  javascript << L"}\n";
  return !CFXA_IsTooBig(javascript);
}

bool CXFA_FMForExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"{\nvar ";
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    javascript << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    javascript << m_wsVariant;
  }
  javascript << L" = null;\n";
  javascript << L"for (";
  javascript << tempVariant;
  javascript << L" = ";
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << L"(";
  if (!m_pAssignment->ToJavaScript(javascript))
    return false;
  javascript << L"); ";
  javascript << tempVariant;
  if (CFXA_IsTooBig(javascript))
    return false;

  javascript << (m_bDirection ? kLessEqual : kGreaterEqual);
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << L"(";
  if (!m_pAccessor->ToJavaScript(javascript))
    return false;
  javascript << L"); ";
  javascript << tempVariant;
  javascript << L" += ";
  javascript << (m_bDirection ? kPlusEqual : kMinusEqual);
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pStep) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << L"(";
    if (!m_pStep->ToJavaScript(javascript))
      return false;
    javascript << L")";
    if (CFXA_IsTooBig(javascript))
      return false;
  } else {
    javascript << L"1";
  }
  javascript << L")\n";
  if (!m_pList->ToImpliedReturnJS(javascript))
    return false;
  javascript << L"}\n";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    uint32_t line,
    const CFX_WideStringC& wsIdentifier,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMLoopExpression(line),
      m_wsIdentifier(wsIdentifier),
      m_pAccessors(std::move(pAccessors)),
      m_pList(std::move(pList)) {}

CXFA_FMForeachExpression::~CXFA_FMForeachExpression() {}

bool CXFA_FMForeachExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"{\n";
  javascript << L"var ";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = null;\n";
  javascript << L"var ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L" = ";
  javascript << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  javascript << L"(";

  for (const auto& expr : m_pAccessors) {
    if (!expr->ToJavaScript(javascript))
      return false;
    if (expr != m_pAccessors.back())
      javascript << L", ";
    if (CFXA_IsTooBig(javascript))
      return false;
  }
  javascript << L");\n";
  javascript << L"var ";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << (L" = 0;\n");
  javascript << L"while(";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << L" < ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L".length)\n{\n";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L"[";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << L"++];\n";
  if (!m_pList->ToJavaScript(javascript))
    return false;
  javascript << L"}\n";
  javascript << L"}\n";
  return !CFXA_IsTooBig(javascript);
}

bool CXFA_FMForeachExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"{\n";
  javascript << L"var ";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = null;\n";
  javascript << L"var ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L" = ";
  javascript << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  javascript << L"(";
  for (const auto& expr : m_pAccessors) {
    if (!expr->ToJavaScript(javascript))
      return false;
    if (expr != m_pAccessors.back())
      javascript << L", ";
    if (CFXA_IsTooBig(javascript))
      return false;
  }
  javascript << L");\n";
  javascript << L"var ";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << L" = 0;\n";
  javascript << L"while(";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << L" < ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L".length)\n{\n";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << L" = ";
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << L"[";
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << L"++];\n";
  if (!m_pList->ToImpliedReturnJS(javascript))
    return false;
  javascript << L"}\n";
  javascript << L"}\n";
  return !CFXA_IsTooBig(javascript);
}
