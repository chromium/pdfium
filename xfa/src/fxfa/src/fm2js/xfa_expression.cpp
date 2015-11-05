// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
static CFX_WideStringC RUNTIMEBLOCKTEMPARRAY =
    FX_WSTRC(L"foxit_xfa_formcalc_runtime_block_temp_array");
static CFX_WideStringC RUNTIMEBLOCKTEMPARRAYINDEX =
    FX_WSTRC(L"foxit_xfa_formcalc_runtime_block_temp_array_index");
CXFA_FMExpression::CXFA_FMExpression(FX_DWORD line)
    : m_type(XFA_FM_EXPTYPE_UNKNOWN), m_line(line) {
}
CXFA_FMExpression::CXFA_FMExpression(FX_DWORD line, XFA_FM_EXPTYPE type)
    : m_type(type), m_line(line) {
}
void CXFA_FMExpression::ToJavaScript(CFX_WideTextBuf& javascript) {}
void CXFA_FMExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {}
CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    FX_DWORD line,
    FX_BOOL isGlobal,
    const CFX_WideStringC& wsName,
    CFX_WideStringCArray* pArguments,
    CFX_PtrArray* pExpressions)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_FUNC),
      m_wsName(wsName),
      m_pArguments(pArguments),
      m_pExpressions(pExpressions),
      m_isGlobal(isGlobal) {
}
CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() {
  if (m_pArguments) {
    m_pArguments->RemoveAll();
    delete m_pArguments;
    m_pArguments = 0;
  }
  if (m_pExpressions) {
    int32_t expc = m_pExpressions->GetSize();
    int32_t index = 0;
    CXFA_FMExpression* e = 0;
    while (index < expc) {
      e = (CXFA_FMExpression*)m_pExpressions->GetAt(index);
      delete e;
      index++;
    }
    m_pExpressions->RemoveAll();
    delete m_pExpressions;
    m_pExpressions = 0;
  }
}
void CXFA_FMFunctionDefinition::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (m_isGlobal && (!m_pExpressions || m_pExpressions->GetSize() == 0)) {
    javascript << FX_WSTRC(L"// comments only");
    return;
  }
  if (m_isGlobal) {
    javascript << FX_WSTRC(L"(\n");
  }
  javascript << FX_WSTRC(L"function ");
  if (m_wsName.GetAt(0) == L'!') {
    CFX_WideString tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
    javascript << tempName;
  } else {
    javascript << m_wsName;
  }
  javascript << FX_WSTRC(L"(");
  if (m_pArguments != 0) {
    int32_t argc = m_pArguments->GetSize();
    int32_t index = 0;
    CFX_WideStringC identifier = 0;
    while (index < argc) {
      identifier = m_pArguments->GetAt(index);
      if (identifier.GetAt(0) == L'!') {
        CFX_WideString tempIdentifier =
            EXCLAMATION_IN_IDENTIFIER + identifier.Mid(1);
        javascript << tempIdentifier;
      } else {
        javascript << identifier;
      }
      if (index + 1 < argc) {
        javascript << FX_WSTRC(L", ");
      }
      index++;
    }
  }
  javascript << FX_WSTRC(L")\n{\n");
  javascript << FX_WSTRC(L"var ");
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = null;\n");
  if (m_pExpressions) {
    int32_t expc = m_pExpressions->GetSize();
    int32_t index = 0;
    CXFA_FMExpression* e = 0;
    while (index < expc) {
      e = (CXFA_FMExpression*)m_pExpressions->GetAt(index);
      if (index + 1 < expc) {
        e->ToJavaScript(javascript);
      } else {
        e->ToImpliedReturnJS(javascript);
      }
      index++;
    }
  }
  javascript << FX_WSTRC(L"return ");
  if (m_isGlobal) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
    javascript << FX_WSTRC(L")");
  } else {
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
  }
  javascript << FX_WSTRC(L";\n}\n");
  if (m_isGlobal) {
    javascript << FX_WSTRC(L").call(this);\n");
  }
}
void CXFA_FMFunctionDefinition::ToImpliedReturnJS(CFX_WideTextBuf&) {}
CXFA_FMVarExpression::CXFA_FMVarExpression(FX_DWORD line,
                                           const CFX_WideStringC& wsName,
                                           CXFA_FMExpression* pInit)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_VAR),
      m_wsName(wsName),
      m_pInit(pInit) {}
CXFA_FMVarExpression::~CXFA_FMVarExpression() {
  if (m_pInit) {
    delete m_pInit;
    m_pInit = 0;
  }
}
void CXFA_FMVarExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"var ");
  CFX_WideString tempName = m_wsName;
  if (m_wsName.GetAt(0) == L'!') {
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  }
  javascript << tempName;
  javascript << FX_WSTRC(L" = ");
  if (m_pInit) {
    m_pInit->ToJavaScript(javascript);
    javascript << tempName;
    javascript << FX_WSTRC(L" = ");
    javascript << XFA_FM_EXPTypeToString(VARFILTER);
    javascript << FX_WSTRC(L"(");
    javascript << tempName;
    javascript << FX_WSTRC(L");\n");
  } else {
    javascript << FX_WSTRC(L"\"\";\n");
  }
}
void CXFA_FMVarExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"var ");
  CFX_WideString tempName = m_wsName;
  if (m_wsName.GetAt(0) == L'!') {
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  }
  javascript << tempName;
  javascript << FX_WSTRC(L" = ");
  if (m_pInit) {
    m_pInit->ToJavaScript(javascript);
    javascript << tempName;
    javascript << FX_WSTRC(L" = ");
    javascript << XFA_FM_EXPTypeToString(VARFILTER);
    javascript << FX_WSTRC(L"(");
    javascript << tempName;
    javascript << FX_WSTRC(L");\n");
  } else {
    javascript << FX_WSTRC(L"\"\";\n");
  }
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = ");
  javascript << tempName;
  javascript << FX_WSTRC(L";\n");
}
CXFA_FMExpExpression::CXFA_FMExpExpression(FX_DWORD line,
                                           CXFA_FMSimpleExpression* pExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_EXP), m_pExpression(pExpression) {}
CXFA_FMExpExpression::~CXFA_FMExpExpression() {
  if (m_pExpression) {
    delete m_pExpression;
    m_pExpression = 0;
  }
}
void CXFA_FMExpExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (m_pExpression->GetOperatorToken() == TOKassign) {
    m_pExpression->ToJavaScript(javascript);
  } else {
    m_pExpression->ToJavaScript(javascript);
    javascript << FX_WSTRC(L";\n");
  }
}
void CXFA_FMExpExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (m_pExpression->GetOperatorToken() == TOKassign) {
    m_pExpression->ToImpliedReturnJS(javascript);
  } else {
    if (m_pExpression->GetOperatorToken() == TOKstar ||
        m_pExpression->GetOperatorToken() == TOKdotstar ||
        m_pExpression->GetOperatorToken() == TOKdotscream ||
        m_pExpression->GetOperatorToken() == TOKdotdot ||
        m_pExpression->GetOperatorToken() == TOKdot) {
      javascript << RUNTIMEFUNCTIONRETURNVALUE;
      javascript << FX_WSTRC(L" = ");
      javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
      javascript << FX_WSTRC(L"(");
      m_pExpression->ToJavaScript(javascript);
      javascript << FX_WSTRC(L");\n");
    } else {
      javascript << RUNTIMEFUNCTIONRETURNVALUE;
      javascript << FX_WSTRC(L" = ");
      m_pExpression->ToJavaScript(javascript);
      javascript << FX_WSTRC(L";\n");
    }
  }
}
CXFA_FMBlockExpression::CXFA_FMBlockExpression(FX_DWORD line,
                                               CFX_PtrArray* pExpressionList)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BLOCK),
      m_pExpressionList(pExpressionList) {}
CXFA_FMBlockExpression::~CXFA_FMBlockExpression() {
  if (m_pExpressionList) {
    int32_t expc = m_pExpressionList->GetSize();
    int32_t index = 0;
    CXFA_FMExpression* e = 0;
    while (index < expc) {
      e = (CXFA_FMExpression*)m_pExpressionList->GetAt(index);
      delete e;
      index++;
    }
    m_pExpressionList->RemoveAll();
    delete m_pExpressionList;
    m_pExpressionList = 0;
  }
}
void CXFA_FMBlockExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"{\n");
  if (m_pExpressionList) {
    int32_t expc = m_pExpressionList->GetSize();
    int32_t index = 0;
    CXFA_FMExpression* e = 0;
    while (index < expc) {
      e = (CXFA_FMExpression*)m_pExpressionList->GetAt(index);
      e->ToJavaScript(javascript);
      index++;
    }
  }
  javascript << FX_WSTRC(L"}\n");
}
void CXFA_FMBlockExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"{\n");
  if (m_pExpressionList) {
    int32_t expc = m_pExpressionList->GetSize();
    int32_t index = 0;
    CXFA_FMExpression* e = 0;
    while (index < expc) {
      e = (CXFA_FMExpression*)m_pExpressionList->GetAt(index);
      if (index + 1 == expc) {
        e->ToImpliedReturnJS(javascript);
      } else {
        e->ToJavaScript(javascript);
      }
      index++;
    }
  }
  javascript << FX_WSTRC(L"}\n");
}
CXFA_FMDoExpression::CXFA_FMDoExpression(FX_DWORD line,
                                         CXFA_FMExpression* pList)
    : CXFA_FMExpression(line), m_pList(pList) {}
CXFA_FMDoExpression::~CXFA_FMDoExpression() {
  if (m_pList) {
    delete m_pList;
    m_pList = 0;
  }
}
void CXFA_FMDoExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  m_pList->ToJavaScript(javascript);
}
void CXFA_FMDoExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  m_pList->ToImpliedReturnJS(javascript);
}
CXFA_FMIfExpression::CXFA_FMIfExpression(FX_DWORD line,
                                         CXFA_FMSimpleExpression* pExpression,
                                         CXFA_FMExpression* pIfExpression,
                                         CXFA_FMExpression* pElseExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_IF),
      m_pExpression(pExpression),
      m_pIfExpression(pIfExpression),
      m_pElseExpression(pElseExpression) {}
CXFA_FMIfExpression::~CXFA_FMIfExpression() {
  if (m_pExpression) {
    delete m_pExpression;
    m_pExpression = 0;
  }
  if (m_pIfExpression) {
    delete m_pIfExpression;
    m_pIfExpression = 0;
  }
  if (m_pElseExpression) {
    delete m_pElseExpression;
    m_pElseExpression = 0;
  }
}
void CXFA_FMIfExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"if (");
  if (m_pExpression) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pExpression->ToJavaScript(javascript);
    javascript << FX_WSTRC(L")");
  }
  javascript << FX_WSTRC(L")\n");
  if (m_pIfExpression) {
    m_pIfExpression->ToJavaScript(javascript);
  }
  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      javascript << FX_WSTRC(L"else\n");
      javascript << FX_WSTRC(L"{\n");
      m_pElseExpression->ToJavaScript(javascript);
      javascript << FX_WSTRC(L"}\n");
    } else {
      javascript << FX_WSTRC(L"else\n");
      m_pElseExpression->ToJavaScript(javascript);
    }
  }
}
void CXFA_FMIfExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"if (");
  if (m_pExpression) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pExpression->ToJavaScript(javascript);
    javascript << FX_WSTRC(L")");
  }
  javascript << FX_WSTRC(L")\n");
  if (m_pIfExpression) {
    m_pIfExpression->ToImpliedReturnJS(javascript);
  }
  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      javascript << FX_WSTRC(L"else\n");
      javascript << FX_WSTRC(L"{\n");
      m_pElseExpression->ToImpliedReturnJS(javascript);
      javascript << FX_WSTRC(L"}\n");
    } else {
      javascript << FX_WSTRC(L"else\n");
      m_pElseExpression->ToImpliedReturnJS(javascript);
    }
  }
}
CXFA_FMLoopExpression::~CXFA_FMLoopExpression() {}
void CXFA_FMLoopExpression::ToJavaScript(CFX_WideTextBuf& javascript) {}
void CXFA_FMLoopExpression::ToImpliedReturnJS(CFX_WideTextBuf&) {}
CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    FX_DWORD line,
    CXFA_FMSimpleExpression* pCondition,
    CXFA_FMExpression* pExpression)
    : CXFA_FMLoopExpression(line),
      m_pCondition(pCondition),
      m_pExpression(pExpression) {}
CXFA_FMWhileExpression::~CXFA_FMWhileExpression() {
  if (m_pCondition) {
    delete m_pCondition;
    m_pCondition = 0;
  }
  if (m_pExpression) {
    delete m_pExpression;
    m_pExpression = 0;
  }
}
void CXFA_FMWhileExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"while (");
  m_pCondition->ToJavaScript(javascript);
  javascript << FX_WSTRC(L")\n");
  m_pExpression->ToJavaScript(javascript);
}
void CXFA_FMWhileExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"while (");
  m_pCondition->ToJavaScript(javascript);
  javascript << FX_WSTRC(L")\n");
  m_pExpression->ToImpliedReturnJS(javascript);
}
CXFA_FMBreakExpression::CXFA_FMBreakExpression(FX_DWORD line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BREAK) {
}
CXFA_FMBreakExpression::~CXFA_FMBreakExpression() {}
void CXFA_FMBreakExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"break;\n");
}
void CXFA_FMBreakExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"break;\n");
}
CXFA_FMContinueExpression::CXFA_FMContinueExpression(FX_DWORD line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_CONTINUE) {
}
CXFA_FMContinueExpression::~CXFA_FMContinueExpression() {}
void CXFA_FMContinueExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"continue;\n");
}
void CXFA_FMContinueExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"continue;\n");
}
CXFA_FMForExpression::CXFA_FMForExpression(FX_DWORD line,
                                           const CFX_WideStringC& wsVariant,
                                           CXFA_FMSimpleExpression* pAssignment,
                                           CXFA_FMSimpleExpression* pAccessor,
                                           int32_t iDirection,
                                           CXFA_FMSimpleExpression* pStep,
                                           CXFA_FMExpression* pList)
    : CXFA_FMLoopExpression(line),
      m_wsVariant(wsVariant),
      m_pAssignment(pAssignment),
      m_pAccessor(pAccessor),
      m_iDirection(iDirection),
      m_pStep(pStep),
      m_pList(pList) {}
CXFA_FMForExpression::~CXFA_FMForExpression() {
  if (m_pAssignment) {
    delete m_pAssignment;
    m_pAssignment = 0;
  }
  if (m_pAccessor) {
    delete m_pAccessor;
    m_pAccessor = 0;
  }
  if (m_pStep) {
    delete m_pStep;
    m_pStep = 0;
  }
  if (m_pList) {
    delete m_pList;
    m_pList = 0;
  }
}
void CXFA_FMForExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"{\nvar ");
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    javascript << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    javascript << m_wsVariant;
  }
  javascript << FX_WSTRC(L" = null;\n");
  javascript << FX_WSTRC(L"for (");
  javascript << tempVariant;
  javascript << FX_WSTRC(L" = ");
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << FX_WSTRC(L"(");
  m_pAssignment->ToJavaScript(javascript);
  javascript << FX_WSTRC(L"); ");
  javascript << tempVariant;
  if (m_iDirection == 1) {
    javascript << FX_WSTRC(L" <= ");
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pAccessor->ToJavaScript(javascript);
    javascript << FX_WSTRC(L"); ");
    javascript << tempVariant;
    javascript << FX_WSTRC(L" += ");
  } else {
    javascript << FX_WSTRC(L" >= ");
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pAccessor->ToJavaScript(javascript);
    javascript << FX_WSTRC(L"); ");
    javascript << tempVariant;
    javascript << FX_WSTRC(L" -= ");
  }
  if (m_pStep) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pStep->ToJavaScript(javascript);
    javascript << FX_WSTRC(L")");
  } else {
    javascript << FX_WSTRC(L"1");
  }
  javascript << FX_WSTRC(L")\n");
  m_pList->ToJavaScript(javascript);
  javascript << FX_WSTRC(L"}\n");
}
void CXFA_FMForExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"{\nvar ");
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    javascript << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    javascript << m_wsVariant;
  }
  javascript << FX_WSTRC(L" = null;\n");
  javascript << FX_WSTRC(L"for (");
  javascript << tempVariant;
  javascript << FX_WSTRC(L" = ");
  javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
  javascript << FX_WSTRC(L"(");
  m_pAssignment->ToJavaScript(javascript);
  javascript << FX_WSTRC(L"); ");
  javascript << tempVariant;
  if (m_iDirection == 1) {
    javascript << FX_WSTRC(L" <= ");
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pAccessor->ToJavaScript(javascript);
    javascript << FX_WSTRC(L"); ");
    javascript << tempVariant;
    javascript << FX_WSTRC(L" += ");
  } else {
    javascript << FX_WSTRC(L" >= ");
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pAccessor->ToJavaScript(javascript);
    javascript << FX_WSTRC(L"); ");
    javascript << tempVariant;
    javascript << FX_WSTRC(L" -= ");
  }
  if (m_pStep) {
    javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
    javascript << FX_WSTRC(L"(");
    m_pStep->ToJavaScript(javascript);
    javascript << FX_WSTRC(L")");
  } else {
    javascript << FX_WSTRC(L"1");
  }
  javascript << FX_WSTRC(L")\n");
  m_pList->ToImpliedReturnJS(javascript);
  javascript << FX_WSTRC(L"}\n");
}
CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    FX_DWORD line,
    const CFX_WideStringC& wsIdentifier,
    CFX_PtrArray* pAccessors,
    CXFA_FMExpression* pList)
    : CXFA_FMLoopExpression(line),
      m_wsIdentifier(wsIdentifier),
      m_pAccessors(pAccessors),
      m_pList(pList) {}
CXFA_FMForeachExpression::~CXFA_FMForeachExpression() {
  if (m_pList) {
    delete m_pList;
    m_pList = 0;
  }
  if (m_pAccessors) {
    int32_t size = m_pAccessors->GetSize();
    int32_t index = 0;
    CXFA_FMSimpleExpression* e = 0;
    while (index < size) {
      e = (CXFA_FMSimpleExpression*)m_pAccessors->GetAt(index);
      delete e;
      index++;
    }
    m_pAccessors->RemoveAll();
    delete m_pAccessors;
    m_pAccessors = 0;
  }
}
void CXFA_FMForeachExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << FX_WSTRC(L"{\n");
  javascript << FX_WSTRC(L"var ");
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << FX_WSTRC(L" = null;\n");
  javascript << FX_WSTRC(L"var ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L" = ");
  javascript << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  javascript << FX_WSTRC(L"(");
  int32_t iSize = m_pAccessors->GetSize();
  int32_t index = 0;
  CXFA_FMSimpleExpression* s = 0;
  while (index < iSize) {
    s = (CXFA_FMSimpleExpression*)m_pAccessors->GetAt(index);
    s->ToJavaScript(javascript);
    if (index + 1 < iSize) {
      javascript << FX_WSTRC(L", ");
    }
    index++;
  }
  s = 0;
  javascript << FX_WSTRC(L");\n");
  javascript << FX_WSTRC(L"var ");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"while(");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L" < ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L".length)\n{\n");
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << FX_WSTRC(L" = ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L"[");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L"++];\n");
  m_pList->ToJavaScript(javascript);
  javascript << FX_WSTRC(L"}\n");
  javascript << FX_WSTRC(L"}\n");
}
void CXFA_FMForeachExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"{\n");
  javascript << FX_WSTRC(L"var ");
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << FX_WSTRC(L" = null;\n");
  javascript << FX_WSTRC(L"var ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L" = ");
  javascript << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  javascript << FX_WSTRC(L"(");
  int32_t iSize = m_pAccessors->GetSize();
  int32_t index = 0;
  CXFA_FMSimpleExpression* s = 0;
  while (index < iSize) {
    s = (CXFA_FMSimpleExpression*)m_pAccessors->GetAt(index);
    s->ToJavaScript(javascript);
    if (index + 1 < iSize) {
      javascript << FX_WSTRC(L", ");
    }
    index++;
  }
  s = 0;
  javascript << FX_WSTRC(L");\n");
  javascript << FX_WSTRC(L"var ");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L" = 0;\n");
  javascript << FX_WSTRC(L"while(");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L" < ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L".length)\n{\n");
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    javascript << tempIdentifier;
  } else {
    javascript << m_wsIdentifier;
  }
  javascript << FX_WSTRC(L" = ");
  javascript << RUNTIMEBLOCKTEMPARRAY;
  javascript << FX_WSTRC(L"[");
  javascript << RUNTIMEBLOCKTEMPARRAYINDEX;
  javascript << FX_WSTRC(L"++];\n");
  m_pList->ToImpliedReturnJS(javascript);
  javascript << FX_WSTRC(L"}\n");
  javascript << FX_WSTRC(L"}\n");
}
