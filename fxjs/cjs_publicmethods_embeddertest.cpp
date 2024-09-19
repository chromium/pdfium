// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>

#include "core/fxcrt/fx_string.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fxjs/cjs_event_context.h"
#include "fxjs/cjs_publicmethods.h"
#include "testing/external_engine_embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/v8-container.h"
#include "v8/include/v8-context.h"
#include "v8/include/v8-isolate.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-value.h"

namespace {

double RoundDownDate(double date) {
  return date - fmod(date, 86400000);
}

}  // namespace

class CJSPublicMethodsEmbedderTest : public ExternalEngineEmbedderTest {};

TEST_F(CJSPublicMethodsEmbedderTest, ParseDateUsingFormat) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());
  bool bWrongFormat;
  double date;

  // 1968
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"06/25/1968",
                                                 L"mm/dd/yyyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(-47865600000, date);
  EXPECT_FALSE(bWrongFormat);

  // 1968
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"25061968",
                                                 L"ddmmyyyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(-47865600000, date);
  EXPECT_FALSE(bWrongFormat);

  // 1968
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"19680625",
                                                 L"yyyymmdd", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(-47865600000, date);
  EXPECT_FALSE(bWrongFormat);

  // 1985
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"31121985",
                                                 L"ddmmyyyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(504835200000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 2085, the other '85.
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"311285",
                                                 L"ddmmyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(3660595200000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 1995
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"01021995",
                                                 L"ddmmyyyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(791596800000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 2095, the other '95.
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"010295",
                                                 L"ddmmyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(3947356800000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 2005
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"01022005",
                                                 L"ddmmyyyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(1107216000000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 2005
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"010205",
                                                 L"ddmmyy", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(1107216000000.0, date);
  EXPECT_FALSE(bWrongFormat);

  // 2005 in a different format. https://crbug.com/436572
  bWrongFormat = false;
  date = CJS_PublicMethods::ParseDateUsingFormat(isolate(), L"050201",
                                                 L"yymmdd", &bWrongFormat);
  date = RoundDownDate(date);
  EXPECT_DOUBLE_EQ(1107216000000.0, date);
  EXPECT_FALSE(bWrongFormat);
}

TEST_F(CJSPublicMethodsEmbedderTest, PrintDateUsingFormat) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());
  WideString formatted_date;

  // 1968-06-25
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(-47952000000, L"ddmmyy");
  EXPECT_EQ(L"250668", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(-47952000000, L"yy/mm/dd");
  EXPECT_EQ(L"68/06/25", formatted_date);

  // 1969-12-31
  formatted_date = CJS_PublicMethods::PrintDateUsingFormat(-0.0001, L"ddmmyy");
  EXPECT_EQ(L"311269", formatted_date);
  formatted_date = CJS_PublicMethods::PrintDateUsingFormat(-0.0001, L"yy!mmdd");
  EXPECT_EQ(L"69!1231", formatted_date);

  // 1970-01-01
  formatted_date = CJS_PublicMethods::PrintDateUsingFormat(0, L"ddmmyy");
  EXPECT_EQ(L"010170", formatted_date);
  formatted_date = CJS_PublicMethods::PrintDateUsingFormat(0, L"mm-yyyy-dd");
  EXPECT_EQ(L"01-1970-01", formatted_date);

  // 1985-12-31
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(504835200000.0, L"ddmmyy");
  EXPECT_EQ(L"311285", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(504835200000.0, L"yymmdd");
  EXPECT_EQ(L"851231", formatted_date);

  // 1995-02-01
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(791596800000.0, L"ddmmyy");
  EXPECT_EQ(L"010295", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(791596800000.0, L"yyyymmdd");
  EXPECT_EQ(L"19950201", formatted_date);

  // 2005-02-01
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(1107216000000.0, L"ddmmyy");
  EXPECT_EQ(L"010205", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(1107216000000.0, L"yyyyddmm");
  EXPECT_EQ(L"20050102", formatted_date);

  // 2085-12-31
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(3660595200000.0, L"ddmmyy");
  EXPECT_EQ(L"311285", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(3660595200000.0, L"yyyydd");
  EXPECT_EQ(L"208531", formatted_date);

  // 2095-02-01
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(3947356800000.0, L"ddmmyy");
  EXPECT_EQ(L"010295", formatted_date);
  formatted_date =
      CJS_PublicMethods::PrintDateUsingFormat(3947356800000.0, L"mmddyyyy");
  EXPECT_EQ(L"02012095", formatted_date);
}

TEST_F(CJSPublicMethodsEmbedderTest, AFSimpleCalculateSum) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());

  ASSERT_TRUE(OpenDocument("calculate.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  CJS_Runtime runtime(
      CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle()));
  runtime.NewEventContext();

  WideString result;
  runtime.GetCurrentEventContext()->SetValueForTest(&result);

  auto ary = runtime.NewArray();
  runtime.PutArrayElement(ary, 0, runtime.NewString("Calc1_A"));
  runtime.PutArrayElement(ary, 1, runtime.NewString("Calc1_B"));

  v8::LocalVector<v8::Value> params(runtime.GetIsolate());
  params.push_back(runtime.NewString("SUM"));
  params.push_back(ary);

  CJS_Result ret = CJS_PublicMethods::AFSimple_Calculate(&runtime, params);

  runtime.GetCurrentEventContext()->SetValueForTest(nullptr);

  ASSERT_TRUE(!ret.HasError());
  ASSERT_TRUE(!ret.HasReturn());
  ASSERT_EQ(L"7", result);
}

TEST_F(CJSPublicMethodsEmbedderTest, AFNumberKeystroke) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());

  ASSERT_TRUE(OpenDocument("calculate.pdf"));
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);

  CJS_Runtime runtime(
      CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle()));
  runtime.NewEventContext();

  auto* handler = runtime.GetCurrentEventContext();

  bool valid = true;
  WideString result = L"-10";
  WideString change = L"";

  handler->SetValueForTest(&result);
  handler->SetRCForTest(&valid);
  handler->SetStrChangeForTest(&change);

  handler->ResetWillCommitForTest();
  handler->SetSelStart(0);
  handler->SetSelEnd(0);

  v8::LocalVector<v8::Value> params(runtime.GetIsolate());
  params.push_back(runtime.NewString("-10"));
  params.push_back(runtime.NewString(""));

  CJS_Result ret = CJS_PublicMethods::AFNumber_Keystroke(&runtime, params);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(!ret.HasError());
  EXPECT_TRUE(!ret.HasReturn());


  // Keep the *SAN bots happy. One of these is an UnownedPtr, another seems to
  // used during destruction. Clear them all to be safe and consistent.
  handler->SetValueForTest(nullptr);
  handler->SetRCForTest(nullptr);
  handler->SetStrChangeForTest(nullptr);
}
