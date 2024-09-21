// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "build/build_config.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/utf16.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxstring, FXUTF8Encode) {
  EXPECT_EQ("", FX_UTF8Encode(WideStringView()));
  EXPECT_EQ(
      "x"
      "\u0080"
      "\u00ff"
      "\ud7ff"
      "\ue000"
      "\uff2c"
      "\uffff"
      "y",
      FX_UTF8Encode(L"x"
                    L"\u0080"
                    L"\u00ff"
                    L"\ud7ff"
                    L"\ue000"
                    L"\uff2c"
                    L"\uffff"
                    L"y"));
}

TEST(fxstring, FXUTF8EncodeSupplementary) {
  EXPECT_EQ(
      "\U00010000"
      "🎨"
      "\U0010ffff",
      FX_UTF8Encode(L"\U00010000"
                    L"\U0001f3a8"
                    L"\U0010ffff"));
}

#if defined(WCHAR_T_IS_16_BIT)
TEST(fxstring, FXUTF8EncodeSurrogateErrorRecovery) {
  EXPECT_EQ("(\xed\xa0\x80)", FX_UTF8Encode(L"(\xd800)")) << "High";
  EXPECT_EQ("(\xed\xb0\x80)", FX_UTF8Encode(L"(\xdc00)")) << "Low";
  EXPECT_EQ("(\xed\xa0\x80🎨)", FX_UTF8Encode(L"(\xd800\xd83c\xdfa8)"))
      << "High-high";
  EXPECT_EQ("(🎨\xed\xb0\x80)", FX_UTF8Encode(L"(\xd83c\xdfa8\xdc00)"))
      << "Low-low";
}
#endif  // defined(WCHAR_T_IS_16_BIT)

TEST(fxstring, ByteStringToFloat) {
  EXPECT_FLOAT_EQ(0.0f, StringToFloat(""));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat("0"));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat("0.0"));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat("-0.0"));

  EXPECT_FLOAT_EQ(0.25f, StringToFloat("0.25"));
  EXPECT_FLOAT_EQ(-0.25f, StringToFloat("-0.25"));

  EXPECT_FLOAT_EQ(100.0f, StringToFloat("100"));
  EXPECT_FLOAT_EQ(100.0f, StringToFloat("100.0"));
  EXPECT_FLOAT_EQ(100.0f, StringToFloat("    100.0"));
  EXPECT_FLOAT_EQ(-100.0f, StringToFloat("-100.0000"));

  EXPECT_FLOAT_EQ(3.402823e+38f,
                  StringToFloat("340282300000000000000000000000000000000"));
  EXPECT_FLOAT_EQ(-3.402823e+38f,
                  StringToFloat("-340282300000000000000000000000000000000"));

  EXPECT_FLOAT_EQ(1.000000119f, StringToFloat("1.000000119"));
  EXPECT_FLOAT_EQ(1.999999881f, StringToFloat("1.999999881"));

  EXPECT_FLOAT_EQ(0.25f, StringToFloat(L"+0.25"));

  EXPECT_FLOAT_EQ(1.2e34f, StringToFloat("1.2e34"));

  EXPECT_FLOAT_EQ(std::numeric_limits<float>::infinity(),
                  StringToFloat("999999999999999999999999999999999999999"));
  EXPECT_FLOAT_EQ(-std::numeric_limits<float>::infinity(),
                  StringToFloat("-999999999999999999999999999999999999999"));

  EXPECT_FLOAT_EQ(0.0f, StringToFloat("invalid"));

  // Test the exact float value. Use EXPECT_EQ, which does an exact comparison,
  // instead of EXPECT_FLOAT_EQ, which allows slight precision error.
  EXPECT_EQ(38.89528656005859375f, StringToFloat("38.895285"));
}

TEST(fxstring, WideStringToFloat) {
  EXPECT_FLOAT_EQ(0.0f, StringToFloat(L""));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat(L"0"));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat(L"0.0"));
  EXPECT_FLOAT_EQ(0.0f, StringToFloat(L"-0.0"));

  EXPECT_FLOAT_EQ(0.25f, StringToFloat(L"0.25"));
  EXPECT_FLOAT_EQ(-0.25f, StringToFloat(L"-0.25"));

  EXPECT_FLOAT_EQ(100.0f, StringToFloat(L"100"));
  EXPECT_FLOAT_EQ(100.0f, StringToFloat(L"100.0"));
  EXPECT_FLOAT_EQ(100.0f, StringToFloat(L"    100.0"));
  EXPECT_FLOAT_EQ(-100.0f, StringToFloat(L"-100.0000"));

  EXPECT_FLOAT_EQ(3.402823e+38f,
                  StringToFloat(L"340282300000000000000000000000000000000"));
  EXPECT_FLOAT_EQ(-3.402823e+38f,
                  StringToFloat(L"-340282300000000000000000000000000000000"));

  EXPECT_FLOAT_EQ(1.000000119f, StringToFloat(L"1.000000119"));
  EXPECT_FLOAT_EQ(1.999999881f, StringToFloat(L"1.999999881"));

  EXPECT_FLOAT_EQ(0.25f, StringToFloat(L"+0.25"));

  EXPECT_FLOAT_EQ(1.2e34f, StringToFloat(L"1.2e34"));

  EXPECT_FLOAT_EQ(std::numeric_limits<float>::infinity(),
                  StringToFloat(L"999999999999999999999999999999999999999"));
  EXPECT_FLOAT_EQ(-std::numeric_limits<float>::infinity(),
                  StringToFloat(L"-999999999999999999999999999999999999999"));

  EXPECT_FLOAT_EQ(0.0f, StringToFloat(L"invalid"));

  // Test the exact float value. Use EXPECT_EQ, which does an exact comparison,
  // instead of EXPECT_FLOAT_EQ, which allows slight precision error.
  EXPECT_EQ(38.89528656005859375f, StringToFloat(L"38.895285"));
}

TEST(fxstring, ByteStringToDouble) {
  EXPECT_DOUBLE_EQ(0.0, StringToDouble(""));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble("0"));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble("0.0"));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble("-0.0"));

  EXPECT_DOUBLE_EQ(0.25, StringToDouble("0.25"));
  EXPECT_DOUBLE_EQ(-0.25, StringToDouble("-0.25"));

  EXPECT_DOUBLE_EQ(100.0, StringToDouble("100"));
  EXPECT_DOUBLE_EQ(100.0, StringToDouble("100.0"));
  EXPECT_DOUBLE_EQ(100.0, StringToDouble("    100.0"));
  EXPECT_DOUBLE_EQ(-100.0, StringToDouble("-100.0000"));

  EXPECT_DOUBLE_EQ(3.402823e+38,
                   StringToDouble("340282300000000000000000000000000000000"));
  EXPECT_DOUBLE_EQ(-3.402823e+38,
                   StringToDouble("-340282300000000000000000000000000000000"));

  EXPECT_DOUBLE_EQ(1.000000119, StringToDouble("1.000000119"));
  EXPECT_DOUBLE_EQ(1.999999881, StringToDouble("1.999999881"));

  EXPECT_DOUBLE_EQ(0.25, StringToDouble("+0.25"));

  EXPECT_DOUBLE_EQ(1.2e34, StringToDouble("1.2e34"));

  EXPECT_DOUBLE_EQ(
      std::numeric_limits<double>::infinity(),
      StringToDouble(
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999"));
  EXPECT_DOUBLE_EQ(
      -std::numeric_limits<double>::infinity(),
      StringToDouble(
          "-9999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999999999999999999999999999999999"
          "99999999999999999999999999999999999999"));

  EXPECT_DOUBLE_EQ(0.0, StringToDouble("invalid"));

  // Test the exact double value. Use EXPECT_EQ, which does an exact comparison,
  // instead of EXPECT_DOUBLE_EQ, which allows slight precision error.
  EXPECT_EQ(1.9998779296892903, StringToDouble("1.99987792968929034"));
}

TEST(fxstring, WideStringToDouble) {
  EXPECT_DOUBLE_EQ(0.0, StringToDouble(L""));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble(L"0"));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble(L"0.0"));
  EXPECT_DOUBLE_EQ(0.0, StringToDouble(L"-0.0"));

  EXPECT_DOUBLE_EQ(0.25, StringToDouble(L"0.25"));
  EXPECT_DOUBLE_EQ(-0.25, StringToDouble(L"-0.25"));

  EXPECT_DOUBLE_EQ(100.0, StringToDouble(L"100"));
  EXPECT_DOUBLE_EQ(100.0, StringToDouble(L"100.0"));
  EXPECT_DOUBLE_EQ(100.0, StringToDouble(L"    100.0"));
  EXPECT_DOUBLE_EQ(-100.0, StringToDouble(L"-100.0000"));

  EXPECT_DOUBLE_EQ(3.402823e+38,
                   StringToDouble(L"340282300000000000000000000000000000000"));
  EXPECT_DOUBLE_EQ(-3.402823e+38,
                   StringToDouble(L"-340282300000000000000000000000000000000"));

  EXPECT_DOUBLE_EQ(1.000000119, StringToDouble(L"1.000000119"));
  EXPECT_DOUBLE_EQ(1.999999881, StringToDouble(L"1.999999881"));

  EXPECT_DOUBLE_EQ(0.25, StringToDouble(L"+0.25"));

  EXPECT_DOUBLE_EQ(1.2e34, StringToDouble(L"1.2e34"));

  EXPECT_DOUBLE_EQ(
      std::numeric_limits<double>::infinity(),
      StringToDouble(
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"999999999999999999999999999999999999999999"));
  EXPECT_DOUBLE_EQ(
      -std::numeric_limits<double>::infinity(),
      StringToDouble(
          L"-999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"9999999999999999999999999999999999999999999999999999999999999999999"
          L"999999999999999999999999999999999999999999"));

  EXPECT_DOUBLE_EQ(0.0, StringToDouble(L"invalid"));

  // Test the exact double value. Use EXPECT_EQ, which does an exact comparison,
  // instead of EXPECT_DOUBLE_EQ, which allows slight precision error.
  EXPECT_EQ(1.9998779296892903, StringToDouble(L"1.99987792968929034"));
}

TEST(fxstring, SplitByteString) {
  std::vector<ByteString> result;
  result = fxcrt::Split(ByteString(""), ',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("", result[0]);

  result = fxcrt::Split(ByteString("a"), ',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("a", result[0]);

  result = fxcrt::Split(ByteString(","), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("", result[1]);

  result = fxcrt::Split(ByteString("a,"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("", result[1]);

  result = fxcrt::Split(ByteString(",b"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("b", result[1]);

  result = fxcrt::Split(ByteString("a,b"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("b", result[1]);

  result = fxcrt::Split(ByteString("a,b,"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("b", result[1]);
  EXPECT_EQ("", result[2]);

  result = fxcrt::Split(ByteString("a,,"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("", result[1]);
  EXPECT_EQ("", result[2]);

  result = fxcrt::Split(ByteString(",,a"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("", result[1]);
  EXPECT_EQ("a", result[2]);
}

TEST(fxstring, SplitByteStringView) {
  std::vector<ByteStringView> result;
  result = fxcrt::Split(ByteStringView(""), ',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("", result[0]);

  result = fxcrt::Split(ByteStringView("a"), ',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("a", result[0]);

  result = fxcrt::Split(ByteStringView(","), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("", result[1]);

  result = fxcrt::Split(ByteStringView("a,"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("", result[1]);

  result = fxcrt::Split(ByteStringView(",b"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("b", result[1]);

  result = fxcrt::Split(ByteStringView("a,b"), ',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("b", result[1]);

  result = fxcrt::Split(ByteStringView("a,b,"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("b", result[1]);
  EXPECT_EQ("", result[2]);

  result = fxcrt::Split(ByteStringView("a,,"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("a", result[0]);
  EXPECT_EQ("", result[1]);
  EXPECT_EQ("", result[2]);

  result = fxcrt::Split(ByteStringView(",,a"), ',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ("", result[0]);
  EXPECT_EQ("", result[1]);
  EXPECT_EQ("a", result[2]);
}

TEST(fxstring, SplitWideString) {
  std::vector<WideString> result;
  result = fxcrt::Split(WideString(L""), L',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(L"", result[0]);

  result = fxcrt::Split(WideString(L"a"), L',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(L"a", result[0]);

  result = fxcrt::Split(WideString(L","), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"", result[1]);

  result = fxcrt::Split(WideString(L"a,"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"", result[1]);

  result = fxcrt::Split(WideString(L",b"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"b", result[1]);

  result = fxcrt::Split(WideString(L"a,b"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"b", result[1]);

  result = fxcrt::Split(WideString(L"a,b,"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"b", result[1]);
  EXPECT_EQ(L"", result[2]);

  result = fxcrt::Split(WideString(L"a,,"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"", result[1]);
  EXPECT_EQ(L"", result[2]);

  result = fxcrt::Split(WideString(L",,a"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"", result[1]);
  EXPECT_EQ(L"a", result[2]);
}

TEST(fxstring, SplitWideStringView) {
  std::vector<WideStringView> result;
  result = fxcrt::Split(WideStringView(L""), L',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(L"", result[0]);

  result = fxcrt::Split(WideStringView(L"a"), L',');
  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(L"a", result[0]);

  result = fxcrt::Split(WideStringView(L","), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"", result[1]);

  result = fxcrt::Split(WideStringView(L"a,"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"", result[1]);

  result = fxcrt::Split(WideStringView(L",b"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"b", result[1]);

  result = fxcrt::Split(WideStringView(L"a,b"), L',');
  ASSERT_EQ(2u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"b", result[1]);

  result = fxcrt::Split(WideStringView(L"a,b,"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"b", result[1]);
  EXPECT_EQ(L"", result[2]);

  result = fxcrt::Split(WideStringView(L"a,,"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"a", result[0]);
  EXPECT_EQ(L"", result[1]);
  EXPECT_EQ(L"", result[2]);

  result = fxcrt::Split(WideStringView(L",,a"), L',');
  ASSERT_EQ(3u, result.size());
  EXPECT_EQ(L"", result[0]);
  EXPECT_EQ(L"", result[1]);
  EXPECT_EQ(L"a", result[2]);
}

TEST(fxstring, ByteStringSplitEfficiency) {
  std::vector<char> commas(50000, ',');
  auto input = ByteString(ByteStringView(commas));
  std::vector<ByteString> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ("", result.front());
  EXPECT_EQ("", result.back());
}

TEST(fxstring, ByteStringViewSplitEfficiency) {
  std::vector<char> commas(50000, ',');
  ByteStringView input(commas);
  std::vector<ByteStringView> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ("", result.front());
  EXPECT_EQ("", result.back());
}

TEST(fxstring, WideStringSplitEfficiency) {
  std::vector<wchar_t> commas(50000, L',');
  auto input = WideString(WideStringView(commas));
  std::vector<WideString> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ(L"", result.front());
  EXPECT_EQ(L"", result.back());
}

TEST(fxstring, WideStringViewSplitEfficiency) {
  std::vector<wchar_t> commas(50000, L',');
  WideStringView input(commas);
  std::vector<WideStringView> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ(L"", result.front());
  EXPECT_EQ(L"", result.back());
}
