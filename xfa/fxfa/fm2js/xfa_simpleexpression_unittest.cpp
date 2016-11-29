// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/xfa_simpleexpression.h"

#include <memory>
#include <utility>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fm2js/xfa_lexer.h"

TEST(FMCallExpression, more_than_32_arguments) {
  // Use sign as it has 3 object parameters at positions 0, 5, and 6.
  auto exp = pdfium::MakeUnique<CXFA_FMIdentifierExpression>(0, L"sign");

  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> args;
  for (size_t i = 0; i < 50; i++)
    args.push_back(pdfium::MakeUnique<CXFA_FMSimpleExpression>(0, TOKnan));

  CXFA_FMCallExpression callExp(0, std::move(exp), std::move(args), true);
  CFX_WideTextBuf js;
  callExp.ToJavaScript(js);

  // Generate the result javascript string.
  CFX_WideString result = L"sign(";
  for (size_t i = 0; i < 50; i++) {
    if (i > 0)
      result += L", ";

    result += L"foxit_xfa_formcalc_runtime.get_fm_";
    // Object positions for sign() method.
    if (i == 0 || i == 5 || i == 6)
      result += L"jsobj()";
    else
      result += L"value()";
  }
  result += L")";

  EXPECT_EQ(result.AsStringC(), js.AsStringC());
}
