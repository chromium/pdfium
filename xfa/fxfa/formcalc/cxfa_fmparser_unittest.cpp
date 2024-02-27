// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/formcalc/cxfa_fmparser.h"

#include "core/fxcrt/widetext_buffer.h"
#include "testing/fxgc_unittest.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xfa/fxfa/formcalc/cxfa_fmtojavascriptdepth.h"

class CXFA_FMParserTest : public FXGCUnitTest {};

TEST_F(CXFA_FMParserTest, Empty) {
  CXFA_FMLexer lexer(L"");
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  // TODO(dsinclair): This is a little weird .....
  EXPECT_EQ(L"// comments only", buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, CommentOnlyIsError) {
  CXFA_FMLexer lexer(L"; Just comment");
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  // TODO(dsinclair): This isn't allowed per the spec.
  EXPECT_FALSE(parser.HasError());
  // EXPECT_TRUE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(L"// comments only", buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, CommentThenValue) {
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = 12;
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(L"; Just comment\n12");
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, Parse) {
  const wchar_t input[] =
      LR"***($ = Avg (-3, 5, -6, 12, -13);
$ = Avg (Table2..Row[*].Cell1);
if ($ ne -1)then
  border.fill.color.value = "255,64,64";
elseif ($ ne -2) then
  border.fill.color.value = "128,128,128";
else
  border.fill.color.value = "20,170,13";
endif
$)***";

  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
if (pfm_rt.is_obj(this))
{
pfm_rt.asgn_val_op(this, pfm_rt.Avg(pfm_rt.neg_op(3), 5, pfm_rt.neg_op(6), 12, pfm_rt.neg_op(13)));
}
if (pfm_rt.is_obj(this))
{
pfm_rt.asgn_val_op(this, pfm_rt.Avg(pfm_rt.dot_acc(pfm_rt.dotdot_acc(Table2, "Table2", "Row", 1), "", "Cell1", 0, 0)));
}
if (pfm_rt.get_val(pfm_rt.neq_op(this, pfm_rt.neg_op(1))))
{
if (pfm_rt.is_obj(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0)))
{
pfm_rt.asgn_val_op(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0), "255,64,64");
}
}
else if (pfm_rt.get_val(pfm_rt.neq_op(this, pfm_rt.neg_op(2))))
{
if (pfm_rt.is_obj(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0)))
{
pfm_rt.asgn_val_op(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0), "128,128,128");
}
}
else {
if (pfm_rt.is_obj(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0)))
{
pfm_rt.asgn_val_op(pfm_rt.dot_acc(pfm_rt.dot_acc(pfm_rt.dot_acc(border, "border", "fill", 0, 0), "", "color", 0, 0), "", "value", 0, 0), "20,170,13");
}
}
pfm_ret = this;
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().AsStringView());
}

TEST_F(CXFA_FMParserTest, MaxParseDepth) {
  CXFA_FMLexer lexer(L"foo(bar[baz(fizz[0])])");
  CXFA_FMParser parser(heap(), &lexer);
  parser.SetMaxParseDepthForTest(5);
  EXPECT_FALSE(parser.Parse());
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, chromium752201) {
  CXFA_FMLexer lexer(
      LR"***(fTep a
.#
fo@ =[=l)***");

  CXFA_FMParser parser(heap(), &lexer);
  EXPECT_FALSE(parser.Parse());
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, MultipleAssignmentIsNotAllowed) {
  CXFA_FMLexer lexer(L"(a=(b=t))=u");
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(!ast);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseFuncWithParams) {
  const wchar_t input[] =
      LR"***(func MyFunction(param1, param2) do
  param1 * param2
endfunc)***";

  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
function MyFunction(param1, param2) {
var pfm_ret = null;
pfm_ret = pfm_rt.mul_op(param1, param2);
return pfm_ret;
}
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFuncWithoutParams) {
  const wchar_t input[] =
      LR"***(func MyFunction() do
  42
endfunc)***";

  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
function MyFunction() {
var pfm_ret = null;
pfm_ret = 42;
return pfm_ret;
}
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast);
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFuncWithBadParamsList) {
  const wchar_t input[] =
      LR"***(func MyFunction(param1,) do
  param1 * param2
endfunc)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseBadIfExpression) {
  const wchar_t input[] = L"if ( then";
  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseBadElseIfExpression) {
  const wchar_t input[] =
      LR"***(if ($ ne -1) then"
elseif( then)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseDepthWithWideTree) {
  const wchar_t input[] = L"a <> b <> c <> d <> e <> f <> g <> h <> i <> j";

  {
    CXFA_FMLexer lexer(input);
    CXFA_FMParser parser(heap(), &lexer);
    CXFA_FMAST* ast = parser.Parse();
    ASSERT_TRUE(ast);
    EXPECT_TRUE(!parser.HasError());
  }

  {
    CXFA_FMLexer lexer(input);
    CXFA_FMParser parser(heap(), &lexer);
    parser.SetMaxParseDepthForTest(5);
    CXFA_FMAST* ast = parser.Parse();
    ASSERT_TRUE(ast == nullptr);
    EXPECT_TRUE(parser.HasError());
  }
}

TEST_F(CXFA_FMParserTest, ParseCallSmall) {
  const wchar_t input[] = L"i.f(O)";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = pfm_rt.get_val((function() {
  return pfm_method_runner(i, function(obj) {
    return obj.f(pfm_rt.get_val(O));
  });
}).call(this));
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseCallBig) {
  const wchar_t input[] = L"i.f(O.e(O.e(O)))";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = pfm_rt.get_val((function() {
  return pfm_method_runner(i, function(obj) {
    return obj.f(pfm_rt.get_val((function() {
  return pfm_method_runner(O, function(obj) {
    return obj.e(pfm_rt.get_val((function() {
  return pfm_method_runner(O, function(obj) {
    return obj.e(pfm_rt.get_val(O));
  });
}).call(this)));
  });
}).call(this)));
  });
}).call(this));
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseVar) {
  const wchar_t input[] = LR"(var s = "")";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
var s = "";
s = pfm_rt.var_filter(s);
pfm_ret = s;
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallNoArguments) {
  const wchar_t input[] = L"P.x()";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = pfm_rt.get_val((function() {
  return pfm_method_runner(P, function(obj) {
    return obj.x();
  });
}).call(this));
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallSingleArgument) {
  const wchar_t input[] = L"P.x(foo)";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = pfm_rt.get_val((function() {
  return pfm_method_runner(P, function(obj) {
    return obj.x(pfm_rt.get_jsobj(foo));
  });
}).call(this));
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallMultipleArguments) {
  const wchar_t input[] = L"P.x(foo, bar, baz)";
  const wchar_t ret[] =
      LR"***((function() {
let pfm_method_runner = function(obj, cb) {
  if (pfm_rt.is_ary(obj)) {
    let pfm_method_return = null;
    for (var idx = obj.length -1; idx > 1; idx--) {
      pfm_method_return = cb(obj[idx]);
    }
    return pfm_method_return;
  }
  return cb(obj);
};
var pfm_ret = null;
pfm_ret = pfm_rt.get_val((function() {
  return pfm_method_runner(P, function(obj) {
    return obj.x(pfm_rt.get_jsobj(foo), pfm_rt.get_val(bar), pfm_rt.get_val(baz));
  });
}).call(this));
return pfm_rt.get_val(pfm_ret);
}).call(this);)***";

  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  EXPECT_FALSE(parser.HasError());

  CXFA_FMToJavaScriptDepth::Reset();
  std::optional<WideTextBuffer> buf = ast->ToJavaScript();
  ASSERT_TRUE(buf.has_value());
  EXPECT_EQ(ret, buf.value().MakeString());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallMissingCommas) {
  const wchar_t input[] = L"P.x(!foo!bar!baz)";
  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallTrailingComma) {
  const wchar_t input[] = L"P.x(foo,bar,baz,)";
  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}

TEST_F(CXFA_FMParserTest, ParseFunctionCallExtraComma) {
  const wchar_t input[] = L"P.x(foo,bar,,baz)";
  CXFA_FMLexer lexer(input);
  CXFA_FMParser parser(heap(), &lexer);
  CXFA_FMAST* ast = parser.Parse();
  ASSERT_TRUE(ast == nullptr);
  EXPECT_TRUE(parser.HasError());
}
