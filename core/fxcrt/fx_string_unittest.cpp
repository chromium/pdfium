// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "build/build_config.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/utf16.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/containers/span.h"

char* TerminatedFloatToString(float value, pdfium::span<char> buf) {
  size_t buflen = FloatToString(value, buf);
  buf[buflen] = '\0';
  return buf.data();
}

char* TerminatedDoubleToString(double value, pdfium::span<char> buf) {
  size_t buflen = DoubleToString(value, buf);
  buf[buflen] = '\0';
  return buf.data();
}

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

#if defined(WCHAR_T_IS_UTF16)
TEST(fxstring, FXUTF8EncodeSurrogateErrorRecovery) {
  EXPECT_EQ("(\xed\xa0\x80)", FX_UTF8Encode(L"(\xd800)")) << "High";
  EXPECT_EQ("(\xed\xb0\x80)", FX_UTF8Encode(L"(\xdc00)")) << "Low";
  EXPECT_EQ("(\xed\xa0\x80🎨)", FX_UTF8Encode(L"(\xd800\xd83c\xdfa8)"))
      << "High-high";
  EXPECT_EQ("(🎨\xed\xb0\x80)", FX_UTF8Encode(L"(\xd83c\xdfa8\xdc00)"))
      << "Low-low";
}
#endif  // defined(WCHAR_T_IS_UTF16)

TEST(fxstring, FXUTF8Decode) {
  EXPECT_EQ(L"", FX_UTF8Decode(ByteStringView()));
  EXPECT_EQ(
      L"x"
      L"\u0080"
      L"\u00ff"
      L"\ud7ff"
      L"\ue000"
      L"\uff2c"
      L"\uffff"
      L"y",
      FX_UTF8Decode("x"
                    "\u0080"
                    "\u00ff"
                    "\ud7ff"
                    "\ue000"
                    "\uff2c"
                    "\uffff"
                    "y"));
}

TEST(fxstring, FXUTF8DecodeSupplementary) {
  EXPECT_EQ(
      L"\U00010000"
      L"\U0001f3a8"
      L"\U0010ffff",
      FX_UTF8Decode("\U00010000"
                    "🎨"
                    "\U0010ffff"));
}

TEST(fxstring, FXUTF8DecodeErrorRecovery) {
  EXPECT_EQ(L"(A)", FX_UTF8Decode("(\xc2\x41)")) << "Invalid continuation";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xc2\xc2)")) << "Invalid continuation";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xc2\xff\x80)")) << "Invalid continuation";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\x80\x80)")) << "Invalid leading";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xff\x80\x80)")) << "Invalid leading";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xf8\x80\x80\x80\x80)"))
      << "Invalid leading";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xf8\x88\x80\x80\x80)"))
      << "Invalid leading";
  EXPECT_EQ(L"()", FX_UTF8Decode("(\xf4\x90\x80\x80)"))
      << "Code point greater than U+10FFFF";
}

TEST(fxstring, FXUTF8EncodeDecodeConsistency) {
  WideString wstr;
  wstr.Reserve(0x10000);
  for (char32_t w = 0; w < pdfium::kMinimumSupplementaryCodePoint; ++w) {
    if (pdfium::IsHighSurrogate(w) || pdfium::IsLowSurrogate(w)) {
      // Skip UTF-16 surrogates.
      continue;
    }
    wstr += static_cast<wchar_t>(w);
  }
  ASSERT_EQ(0xf800u, wstr.GetLength());

  ByteString bstr = FX_UTF8Encode(wstr.AsStringView());
  WideString wstr2 = FX_UTF8Decode(bstr.AsStringView());
  EXPECT_EQ(wstr, wstr2);
}

TEST(fxstring, FXUTF8EncodeDecodeConsistencyUnpairedHighSurrogates) {
  WideString wstr;
  wstr.Reserve(0x400);
  for (wchar_t w = pdfium::kMinimumHighSurrogateCodeUnit;
       w <= pdfium::kMaximumHighSurrogateCodeUnit; ++w) {
    wstr += w;
  }
  ASSERT_EQ(0x400u, wstr.GetLength());

  ByteString bstr = FX_UTF8Encode(wstr.AsStringView());
  WideString wstr2 = FX_UTF8Decode(bstr.AsStringView());
  EXPECT_EQ(wstr, wstr2);
}

TEST(fxstring, FXUTF8EncodeDecodeConsistencyUnpairedLowSurrogates) {
  WideString wstr;
  wstr.Reserve(0x400);
  for (wchar_t w = pdfium::kMinimumLowSurrogateCodeUnit;
       w <= pdfium::kMaximumLowSurrogateCodeUnit; ++w) {
    wstr += w;
  }
  ASSERT_EQ(0x400u, wstr.GetLength());

  ByteString bstr = FX_UTF8Encode(wstr.AsStringView());
  WideString wstr2 = FX_UTF8Decode(bstr.AsStringView());
  EXPECT_EQ(wstr, wstr2);
}

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
}

TEST(fxstring, FloatToString) {
  char buf[32];

  EXPECT_STREQ("0", TerminatedFloatToString(0.0f, buf));
  EXPECT_STREQ("0", TerminatedFloatToString(-0.0f, buf));
  EXPECT_STREQ("0",
               TerminatedFloatToString(std::numeric_limits<float>::min(), buf));
  EXPECT_STREQ(
      "0", TerminatedFloatToString(-std::numeric_limits<float>::min(), buf));

  EXPECT_STREQ("0.25", TerminatedFloatToString(0.25f, buf));
  EXPECT_STREQ("-0.25", TerminatedFloatToString(-0.25f, buf));

  EXPECT_STREQ("100", TerminatedFloatToString(100.0f, buf));
  EXPECT_STREQ("-100", TerminatedFloatToString(-100.0f, buf));

  // FloatToString won't convert beyond the maximum integer, and values
  // larger than that get converted to a string representing that.
  EXPECT_STREQ("2147483647", TerminatedFloatToString(2147483647.0f, buf));
  EXPECT_STREQ("2147483647", TerminatedFloatToString(2147483647.5f, buf));
  EXPECT_STREQ("2147483647",
               TerminatedFloatToString(std::numeric_limits<float>::max(), buf));

  // FloatToString won't convert beyond the minimum integer, and values
  // smaller than that get converted to a string representing that.
  EXPECT_STREQ("-2147483647", TerminatedFloatToString(-2147483647.0f, buf));
  EXPECT_STREQ("-2147483647", TerminatedFloatToString(-2147483647.5f, buf));
  EXPECT_STREQ("-2147483647", TerminatedFloatToString(
                                  -std::numeric_limits<float>::max(), buf));

  // Conversion only acknowledges precision to 5 digit past decimal, and
  // rounds beyond that.
  EXPECT_STREQ("1", TerminatedFloatToString(1.000001119f, buf));
  EXPECT_STREQ("1.00001", TerminatedFloatToString(1.000011119f, buf));
  EXPECT_STREQ("1.99999", TerminatedFloatToString(1.999988881f, buf));
  EXPECT_STREQ("2", TerminatedFloatToString(1.999999881f, buf));
}

TEST(fxstring, ByteStringToDouble) {
  EXPECT_FLOAT_EQ(0.0, StringToDouble(""));
  EXPECT_FLOAT_EQ(0.0, StringToDouble("0"));
  EXPECT_FLOAT_EQ(0.0, StringToDouble("0.0"));
  EXPECT_FLOAT_EQ(0.0, StringToDouble("-0.0"));

  EXPECT_FLOAT_EQ(0.25, StringToDouble("0.25"));
  EXPECT_FLOAT_EQ(-0.25, StringToDouble("-0.25"));

  EXPECT_FLOAT_EQ(100.0, StringToDouble("100"));
  EXPECT_FLOAT_EQ(100.0, StringToDouble("100.0"));
  EXPECT_FLOAT_EQ(100.0, StringToDouble("    100.0"));
  EXPECT_FLOAT_EQ(-100.0, StringToDouble("-100.0000"));

  EXPECT_FLOAT_EQ(3.402823e+38,
                  StringToDouble("340282300000000000000000000000000000000"));
  EXPECT_FLOAT_EQ(-3.402823e+38,
                  StringToDouble("-340282300000000000000000000000000000000"));

  EXPECT_FLOAT_EQ(1.000000119, StringToDouble("1.000000119"));
  EXPECT_FLOAT_EQ(1.999999881, StringToDouble("1.999999881"));
}

TEST(fxstring, WideStringToDouble) {
  EXPECT_FLOAT_EQ(0.0, StringToDouble(L""));
  EXPECT_FLOAT_EQ(0.0, StringToDouble(L"0"));
  EXPECT_FLOAT_EQ(0.0, StringToDouble(L"0.0"));
  EXPECT_FLOAT_EQ(0.0, StringToDouble(L"-0.0"));

  EXPECT_FLOAT_EQ(0.25, StringToDouble(L"0.25"));
  EXPECT_FLOAT_EQ(-0.25, StringToDouble(L"-0.25"));

  EXPECT_FLOAT_EQ(100.0, StringToDouble(L"100"));
  EXPECT_FLOAT_EQ(100.0, StringToDouble(L"100.0"));
  EXPECT_FLOAT_EQ(100.0, StringToDouble(L"    100.0"));
  EXPECT_FLOAT_EQ(-100.0, StringToDouble(L"-100.0000"));

  EXPECT_FLOAT_EQ(3.402823e+38,
                  StringToDouble(L"340282300000000000000000000000000000000"));
  EXPECT_FLOAT_EQ(-3.402823e+38,
                  StringToDouble(L"-340282300000000000000000000000000000000"));

  EXPECT_FLOAT_EQ(1.000000119, StringToDouble(L"1.000000119"));
  EXPECT_FLOAT_EQ(1.999999881, StringToDouble(L"1.999999881"));
}

TEST(fxstring, DoubleToString) {
  char buf[32];

  EXPECT_STREQ("0", TerminatedDoubleToString(0.0f, buf));
  EXPECT_STREQ("0", TerminatedDoubleToString(-0.0f, buf));
  EXPECT_STREQ(
      "0", TerminatedDoubleToString(std::numeric_limits<double>::min(), buf));
  EXPECT_STREQ(
      "0", TerminatedDoubleToString(-std::numeric_limits<double>::min(), buf));

  EXPECT_STREQ("0.25", TerminatedDoubleToString(0.25f, buf));
  EXPECT_STREQ("-0.25", TerminatedDoubleToString(-0.25f, buf));

  EXPECT_STREQ("100", TerminatedDoubleToString(100.0f, buf));
  EXPECT_STREQ("-100", TerminatedDoubleToString(-100.0f, buf));

  // DoubleToString won't convert beyond the maximum integer, and values
  // larger than that get converted to a string representing that.
  EXPECT_STREQ("2147483647", TerminatedDoubleToString(2147483647.0f, buf));
  EXPECT_STREQ("2147483647", TerminatedDoubleToString(2147483647.5f, buf));
  EXPECT_STREQ("2147483647", TerminatedDoubleToString(
                                 std::numeric_limits<double>::max(), buf));

  // DoubleToString won't convert beyond the minimum integer, and values
  // smaller than that get converted to a string representing that.
  EXPECT_STREQ("-2147483647", TerminatedDoubleToString(-2147483647.0f, buf));
  EXPECT_STREQ("-2147483647", TerminatedDoubleToString(-2147483647.5f, buf));
  EXPECT_STREQ("-2147483647", TerminatedDoubleToString(
                                  -std::numeric_limits<double>::max(), buf));

  // Conversion only acknowledges precision to 5 digit past decimal, and
  // rounds beyond that.
  EXPECT_STREQ("1", TerminatedDoubleToString(1.000001119f, buf));
  EXPECT_STREQ("1.00001", TerminatedDoubleToString(1.000011119f, buf));
  EXPECT_STREQ("1.99999", TerminatedDoubleToString(1.999988881f, buf));
  EXPECT_STREQ("2", TerminatedDoubleToString(1.999999881f, buf));
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
  ByteString input(commas.data(), commas.size());
  std::vector<ByteString> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ("", result.front());
  EXPECT_EQ("", result.back());
}

TEST(fxstring, ByteStringViewSplitEfficiency) {
  std::vector<char> commas(50000, ',');
  ByteStringView input(commas.data(), commas.size());
  std::vector<ByteStringView> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ("", result.front());
  EXPECT_EQ("", result.back());
}

TEST(fxstring, WideStringSplitEfficiency) {
  std::vector<wchar_t> commas(50000, L',');
  WideString input(commas.data(), commas.size());
  std::vector<WideString> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ(L"", result.front());
  EXPECT_EQ(L"", result.back());
}

TEST(fxstring, WideStringViewSplitEfficiency) {
  std::vector<wchar_t> commas(50000, L',');
  WideStringView input(commas.data(), commas.size());
  std::vector<WideStringView> result;
  result = fxcrt::Split(input, ',');
  ASSERT_EQ(commas.size() + 1, result.size());
  EXPECT_EQ(L"", result.front());
  EXPECT_EQ(L"", result.back());
}
