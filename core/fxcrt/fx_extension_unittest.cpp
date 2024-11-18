// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_extension.h"

#include <math.h>
#include <stdint.h>

#include <iterator>
#include <limits>

#include "core/fxcrt/compiler_specific.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ElementsAre;

TEST(fxcrt, FXSYSIsLowerASCII) {
  EXPECT_TRUE(FXSYS_IsLowerASCII('a'));
  EXPECT_TRUE(FXSYS_IsLowerASCII(L'a'));
  EXPECT_TRUE(FXSYS_IsLowerASCII('b'));
  EXPECT_TRUE(FXSYS_IsLowerASCII(L'b'));
  EXPECT_TRUE(FXSYS_IsLowerASCII('y'));
  EXPECT_TRUE(FXSYS_IsLowerASCII(L'y'));
  EXPECT_TRUE(FXSYS_IsLowerASCII('z'));
  EXPECT_TRUE(FXSYS_IsLowerASCII(L'z'));
  EXPECT_FALSE(FXSYS_IsLowerASCII('`'));
  EXPECT_FALSE(FXSYS_IsLowerASCII(L'`'));
  EXPECT_FALSE(FXSYS_IsLowerASCII('{'));
  EXPECT_FALSE(FXSYS_IsLowerASCII(L'{'));
  EXPECT_FALSE(FXSYS_IsLowerASCII('Z'));
  EXPECT_FALSE(FXSYS_IsLowerASCII(L'Z'));
  EXPECT_FALSE(FXSYS_IsLowerASCII('7'));
  EXPECT_FALSE(FXSYS_IsLowerASCII(L'7'));
  EXPECT_FALSE(FXSYS_IsLowerASCII(static_cast<char>(-78)));
  EXPECT_FALSE(FXSYS_IsLowerASCII(static_cast<wchar_t>(0xb2)));
}

TEST(fxcrt, FXSYSIsUpperASCII) {
  EXPECT_TRUE(FXSYS_IsUpperASCII('A'));
  EXPECT_TRUE(FXSYS_IsUpperASCII(L'A'));
  EXPECT_TRUE(FXSYS_IsUpperASCII('B'));
  EXPECT_TRUE(FXSYS_IsUpperASCII(L'B'));
  EXPECT_TRUE(FXSYS_IsUpperASCII('Y'));
  EXPECT_TRUE(FXSYS_IsUpperASCII(L'Y'));
  EXPECT_TRUE(FXSYS_IsUpperASCII('Z'));
  EXPECT_TRUE(FXSYS_IsUpperASCII(L'Z'));
  EXPECT_FALSE(FXSYS_IsUpperASCII('@'));
  EXPECT_FALSE(FXSYS_IsUpperASCII(L'@'));
  EXPECT_FALSE(FXSYS_IsUpperASCII('['));
  EXPECT_FALSE(FXSYS_IsUpperASCII(L'['));
  EXPECT_FALSE(FXSYS_IsUpperASCII('z'));
  EXPECT_FALSE(FXSYS_IsUpperASCII(L'z'));
  EXPECT_FALSE(FXSYS_IsUpperASCII('7'));
  EXPECT_FALSE(FXSYS_IsUpperASCII(L'7'));
  EXPECT_FALSE(FXSYS_IsUpperASCII(static_cast<char>(-78)));
  EXPECT_FALSE(FXSYS_IsUpperASCII(static_cast<wchar_t>(0xb2)));
}

TEST(fxcrt, FXSYSHexCharToInt) {
  EXPECT_EQ(10, FXSYS_HexCharToInt('a'));
  EXPECT_EQ(10, FXSYS_HexCharToInt('A'));
  EXPECT_EQ(7, FXSYS_HexCharToInt('7'));
  EXPECT_EQ(0, FXSYS_HexCharToInt('i'));
}

TEST(fxcrt, FXSYSDecimalCharToInt) {
  EXPECT_EQ(7, FXSYS_DecimalCharToInt('7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt('a'));
  EXPECT_EQ(7, FXSYS_DecimalCharToInt(L'7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(L'a'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(static_cast<char>(-78)));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(static_cast<wchar_t>(0xb2)));
}

TEST(fxcrt, FXSYSIsDecimalDigit) {
  EXPECT_TRUE(FXSYS_IsDecimalDigit('7'));
  EXPECT_TRUE(FXSYS_IsDecimalDigit(L'7'));
  EXPECT_FALSE(FXSYS_IsDecimalDigit('a'));
  EXPECT_FALSE(FXSYS_IsDecimalDigit(L'a'));
  EXPECT_FALSE(FXSYS_IsDecimalDigit(static_cast<char>(-78)));
  EXPECT_FALSE(FXSYS_IsDecimalDigit(static_cast<wchar_t>(0xb2)));
}

TEST(fxcrt, FXSYSIntToTwoHexChars) {
  char buf[2] = {0};
  FXSYS_IntToTwoHexChars(0x0, buf);
  EXPECT_THAT(buf, ElementsAre('0', '0'));
  FXSYS_IntToTwoHexChars(0x9, buf);
  EXPECT_THAT(buf, ElementsAre('0', '9'));
  FXSYS_IntToTwoHexChars(0xA, buf);
  EXPECT_THAT(buf, ElementsAre('0', 'A'));
  FXSYS_IntToTwoHexChars(0x8C, buf);
  EXPECT_THAT(buf, ElementsAre('8', 'C'));
  FXSYS_IntToTwoHexChars(0xBE, buf);
  EXPECT_THAT(buf, ElementsAre('B', 'E'));
  FXSYS_IntToTwoHexChars(0xD0, buf);
  EXPECT_THAT(buf, ElementsAre('D', '0'));
  FXSYS_IntToTwoHexChars(0xFF, buf);
  EXPECT_THAT(buf, ElementsAre('F', 'F'));
}

TEST(fxcrt, FXSYSIntToFourHexChars) {
  char buf[4] = {0};
  FXSYS_IntToFourHexChars(0x0, buf);
  EXPECT_THAT(buf, ElementsAre('0', '0', '0', '0'));
  FXSYS_IntToFourHexChars(0xA23, buf);
  EXPECT_THAT(buf, ElementsAre('0', 'A', '2', '3'));
  FXSYS_IntToFourHexChars(0xB701, buf);
  EXPECT_THAT(buf, ElementsAre('B', '7', '0', '1'));
  FXSYS_IntToFourHexChars(0xFFFF, buf);
  EXPECT_THAT(buf, ElementsAre('F', 'F', 'F', 'F'));
}

TEST(fxcrt, FXSYSToUTF16BE) {
  char buf[8] = {0};
  // Test U+0000 to U+D7FF and U+E000 to U+FFFF
  pdfium::span<const char> result = FXSYS_ToUTF16BE(0x0, buf);
  EXPECT_THAT(result, ElementsAre('0', '0', '0', '0'));
  result = FXSYS_ToUTF16BE(0xD7FF, buf);
  EXPECT_THAT(result, ElementsAre('D', '7', 'F', 'F'));
  result = FXSYS_ToUTF16BE(0xE000, buf);
  EXPECT_THAT(result, ElementsAre('E', '0', '0', '0'));
  result = FXSYS_ToUTF16BE(0xFFFF, buf);
  EXPECT_THAT(result, ElementsAre('F', 'F', 'F', 'F'));
  // Test U+10000 to U+10FFFF
  result = FXSYS_ToUTF16BE(0x10000, buf);
  EXPECT_THAT(result, ElementsAre('D', '8', '0', '0', 'D', 'C', '0', '0'));
  result = FXSYS_ToUTF16BE(0x10FFFF, buf);
  EXPECT_THAT(result, ElementsAre('D', 'B', 'F', 'F', 'D', 'F', 'F', 'F'));
  result = FXSYS_ToUTF16BE(0x2003E, buf);
  EXPECT_THAT(result, ElementsAre('D', '8', '4', '0', 'D', 'C', '3', 'E'));
}

TEST(fxcrt, FXSYSwcstof) {
  size_t used_len = 0;
  EXPECT_FLOAT_EQ(-12.0f, FXSYS_wcstof(L"-12", &used_len));
  EXPECT_EQ(3u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(12.0f, FXSYS_wcstof(L"+12", &used_len));
  EXPECT_EQ(3u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(123.0f, FXSYS_wcstof(L" 123", &used_len));
  EXPECT_EQ(4u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(123.0f, FXSYS_wcstof(L" 123 ", &used_len));
  EXPECT_EQ(4u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.0f, FXSYS_wcstof(L" 1 2 3 ", &used_len));
  EXPECT_EQ(2u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.5362f, FXSYS_wcstof(L"1.5362", &used_len));
  EXPECT_EQ(6u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.0f, FXSYS_wcstof(L"1 .5362", &used_len));
  EXPECT_EQ(1u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.0f, FXSYS_wcstof(L"1. 5362", &used_len));
  EXPECT_EQ(2u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.5f, FXSYS_wcstof(L"1.5.3.6.2", &used_len));
  EXPECT_EQ(3u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(0.875f, FXSYS_wcstof(L"0.875", &used_len));
  EXPECT_EQ(5u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(5.56e-2f, FXSYS_wcstof(L"5.56e-2", &used_len));
  EXPECT_EQ(7u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(1.234e10f, FXSYS_wcstof(L"1.234E10", &used_len));
  EXPECT_EQ(8u, used_len);

  used_len = 0;
  EXPECT_TRUE(isinf(FXSYS_wcstof(L"1.234E100000000000000", &used_len)));
  EXPECT_EQ(21u, used_len);

  used_len = 0;
  EXPECT_FLOAT_EQ(0.0f, FXSYS_wcstof(L"1.234E-128", &used_len));
  EXPECT_EQ(10u, used_len);

  // TODO(dsinclair): This should round as per IEEE 64-bit values.
  // EXPECT_EQ(L"123456789.01234567", FXSYS_wcstof(L"123456789.012345678"));
  used_len = 0;
  EXPECT_FLOAT_EQ(123456789.012345678f,
                  FXSYS_wcstof(L"123456789.012345678", &used_len));
  EXPECT_EQ(19u, used_len);

  // TODO(dsinclair): This is spec'd as rounding when > 16 significant digits
  // prior to the exponent.
  // EXPECT_EQ(100000000000000000, FXSYS_wcstof(L"99999999999999999"));
  used_len = 0;
  EXPECT_FLOAT_EQ(99999999999999999.0f,
                  FXSYS_wcstof(L"99999999999999999", &used_len));
  EXPECT_EQ(17u, used_len);

  // For https://crbug.com/pdfium/1217
  EXPECT_FLOAT_EQ(0.0f, FXSYS_wcstof(L"e76", nullptr));

  // Overflow to infinity.
  used_len = 0;
  EXPECT_TRUE(isinf(FXSYS_wcstof(
      L"88888888888888888888888888888888888888888888888888888888888888888888888"
      L"88888888888888888888888888888888888888888888888888888888888",
      &used_len)));
  EXPECT_EQ(130u, used_len);

  used_len = 0;
  EXPECT_TRUE(isinf(FXSYS_wcstof(
      L"-8888888888888888888888888888888888888888888888888888888888888888888888"
      L"888888888888888888888888888888888888888888888888888888888888",
      &used_len)));
  EXPECT_EQ(131u, used_len);
}

TEST(fxcrt, FXSYSSafeOps) {
  const float fMin = std::numeric_limits<float>::min();
  const float fMax = std::numeric_limits<float>::max();
  const float fInf = std::numeric_limits<float>::infinity();
  const float fNan = std::numeric_limits<float>::quiet_NaN();
  const float ascending[] = {fMin, 1.0f, 2.0f, fMax, fInf, fNan};

  UNSAFE_TODO({
    for (size_t i = 0; i < std::size(ascending); ++i) {
      for (size_t j = 0; j < std::size(ascending); ++j) {
        if (i == j) {
          EXPECT_TRUE(FXSYS_SafeEQ(ascending[i], ascending[j]))
              << " at " << i << " " << j;
        } else {
          EXPECT_FALSE(FXSYS_SafeEQ(ascending[i], ascending[j]))
              << " at " << i << " " << j;
        }
        if (i < j) {
          EXPECT_TRUE(FXSYS_SafeLT(ascending[i], ascending[j]))
              << " at " << i << " " << j;
        } else {
          EXPECT_FALSE(FXSYS_SafeLT(ascending[i], ascending[j]))
              << " at " << i << " " << j;
        }
      }
    }
  });
}
