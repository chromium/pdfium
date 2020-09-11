// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/cxfa_fmexpression.h"

#include <memory>
#include <utility>

#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"

TEST(FMCallExpressionTest, more_than_32_arguments) {
  // Use sign as it has 3 object parameters at positions 0, 5, and 6.
  auto exp = std::make_unique<CXFA_FMIdentifierExpression>(L"sign");

  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> args;
  for (size_t i = 0; i < 50; i++)
    args.push_back(std::make_unique<CXFA_FMNullExpression>());

  CXFA_FMToJavaScriptDepth::Reset();
  CXFA_FMCallExpression callExp(std::move(exp), std::move(args), true);

  CFX_WideTextBuf js;
  callExp.ToJavaScript(&js, ReturnType::kInfered);

  // Generate the result javascript string.
  WideString result = L"sign(";
  for (size_t i = 0; i < 50; i++) {
    if (i > 0)
      result += L", ";

    result += L"pfm_rt.get_";
    // Object positions for sign() method.
    if (i == 0 || i == 5 || i == 6)
      result += L"jsobj(null)";
    else
      result += L"val(null)";
  }
  result += L")";

  EXPECT_EQ(result.AsStringView(), js.AsStringView());
}

TEST(FMStringExpressionTest, Empty) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(L"").ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_EQ(L"", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Short) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(L"a").ToJavaScript(&accumulator,
                                             ReturnType::kInfered);
  EXPECT_EQ(L"a", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Medium) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(L".abcd.").ToJavaScript(&accumulator,
                                                  ReturnType::kInfered);
  EXPECT_EQ(L"\"abcd\"", accumulator.AsStringView());
}

TEST(FMStringExpressionTest, Long) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  std::vector<WideStringView::UnsignedType> vec(140000, L'A');
  CXFA_FMStringExpression(WideStringView(vec))
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_EQ(140000u, accumulator.GetLength());
}

TEST(FMStringExpressionTest, Quoted) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;
  CXFA_FMStringExpression(L".Simon says \"\"run\"\".")
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_EQ(L"\"Simon says \\\"run\\\"\"", accumulator.AsStringView());
}

TEST(CXFA_FMExpressionTest, VarExpressionInitNull) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;

  CXFA_FMVarExpression(L"s", nullptr)
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_STREQ(
      LR"***(var s = "";
)***",
      accumulator.MakeString().c_str());
}

TEST(CXFA_FMExpressionTest, VarExpressionInitBlank) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;

  auto init = std::make_unique<CXFA_FMStringExpression>(LR"("")");
  CXFA_FMVarExpression(L"s", std::move(init))
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_STREQ(
      LR"***(var s = "";
s = pfm_rt.var_filter(s);
)***",
      accumulator.MakeString().c_str());
}

TEST(CXFA_FMExpressionTest, VarExpressionInitString) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;

  auto init = std::make_unique<CXFA_FMStringExpression>(LR"("foo")");
  CXFA_FMVarExpression(L"s", std::move(init))
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_STREQ(
      LR"***(var s = "foo";
s = pfm_rt.var_filter(s);
)***",
      accumulator.MakeString().c_str());
}

TEST(CXFA_FMExpressionTest, VarExpressionInitNumeric) {
  CXFA_FMToJavaScriptDepth::Reset();
  CFX_WideTextBuf accumulator;

  auto init = std::make_unique<CXFA_FMNumberExpression>(L"112");
  CXFA_FMVarExpression(L"s", std::move(init))
      .ToJavaScript(&accumulator, ReturnType::kInfered);
  EXPECT_STREQ(
      LR"***(var s = 112;
s = pfm_rt.var_filter(s);
)***",
      accumulator.MakeString().c_str());
}
