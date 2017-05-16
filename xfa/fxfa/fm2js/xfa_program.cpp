// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_program.h"

#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"

CXFA_FMProgram::CXFA_FMProgram(const CFX_WideStringC& wsFormcalc)
    : m_parse(wsFormcalc, &m_pErrorInfo) {}

CXFA_FMProgram::~CXFA_FMProgram() {}

bool CXFA_FMProgram::ParseProgram() {
  m_parse.NextToken();
  if (!m_pErrorInfo.message.IsEmpty())
    return false;

  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions =
      m_parse.ParseTopExpression();
  if (!m_pErrorInfo.message.IsEmpty())
    return false;

  std::vector<CFX_WideStringC> arguments;
  m_globalFunction = pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      1, true, L"", std::move(arguments), std::move(expressions));
  return true;
}

bool CXFA_FMProgram::TranslateProgram(CFX_WideTextBuf& wsJavaScript) {
  if (!m_globalFunction->ToJavaScript(wsJavaScript))
    return false;
  wsJavaScript.AppendChar(0);
  return true;
}
