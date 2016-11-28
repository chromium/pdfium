// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_program.h"

#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"

CXFA_FMProgram::CXFA_FMProgram() {}

CXFA_FMProgram::~CXFA_FMProgram() {}

int32_t CXFA_FMProgram::Init(const CFX_WideStringC& wsFormcalc) {
  return m_parse.Init(wsFormcalc, &m_pErrorInfo);
}

int32_t CXFA_FMProgram::ParseProgram() {
  m_parse.NextToken();
  if (!m_pErrorInfo.message.IsEmpty())
    return -1;

  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions =
      m_parse.ParseTopExpression();
  if (!m_pErrorInfo.message.IsEmpty())
    return -1;

  m_globalFunction = pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      1, true, L"", nullptr, std::move(expressions));
  return 0;
}

int32_t CXFA_FMProgram::TranslateProgram(CFX_WideTextBuf& wsJavaScript) {
  m_globalFunction->ToJavaScript(wsJavaScript);
  wsJavaScript.AppendChar(0);
  return 0;
}
