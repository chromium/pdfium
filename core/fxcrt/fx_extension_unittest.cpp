// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/fx_extension.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

uint32_t ReferenceGetBits32(const uint8_t* pData, int bitpos, int nbits) {
  int result = 0;
  for (int i = 0; i < nbits; i++) {
    if (pData[(bitpos + i) / 8] & (1 << (7 - (bitpos + i) % 8)))
      result |= 1 << (nbits - i - 1);
  }
  return result;
}

}  // namespace

TEST(fxcrt, FXSYS_HexCharToInt) {
  EXPECT_EQ(10, FXSYS_HexCharToInt('a'));
  EXPECT_EQ(10, FXSYS_HexCharToInt('A'));
  EXPECT_EQ(7, FXSYS_HexCharToInt('7'));
  EXPECT_EQ(0, FXSYS_HexCharToInt('i'));
}

TEST(fxcrt, FXSYS_DecimalCharToInt) {
  EXPECT_EQ(7, FXSYS_DecimalCharToInt('7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt('a'));
  EXPECT_EQ(7, FXSYS_DecimalCharToInt(L'7'));
  EXPECT_EQ(0, FXSYS_DecimalCharToInt(L'a'));
}

TEST(fxcrt, FXSYS_isDecimalDigit) {
  EXPECT_TRUE(FXSYS_isDecimalDigit('7'));
  EXPECT_TRUE(FXSYS_isDecimalDigit(L'7'));
  EXPECT_FALSE(FXSYS_isDecimalDigit('a'));
  EXPECT_FALSE(FXSYS_isDecimalDigit(L'a'));
}

TEST(fxcrt, FX_HashCode_Ascii) {
  EXPECT_EQ(0u, FX_HashCode_GetA("", false));
  EXPECT_EQ(65u, FX_HashCode_GetA("A", false));
  EXPECT_EQ(97u, FX_HashCode_GetA("A", true));
  EXPECT_EQ(31 * 65u + 66u, FX_HashCode_GetA("AB", false));
}

TEST(fxcrt, FX_HashCode_Wide) {
  EXPECT_EQ(0u, FX_HashCode_GetW(L"", false));
  EXPECT_EQ(65u, FX_HashCode_GetW(L"A", false));
  EXPECT_EQ(97u, FX_HashCode_GetW(L"A", true));
  EXPECT_EQ(1313 * 65u + 66u, FX_HashCode_GetW(L"AB", false));
}

TEST(fxcrt, FXSYS_IntToTwoHexChars) {
  char buf[3] = {0};
  FXSYS_IntToTwoHexChars(0x0, buf);
  EXPECT_STREQ("00", buf);
  FXSYS_IntToTwoHexChars(0x9, buf);
  EXPECT_STREQ("09", buf);
  FXSYS_IntToTwoHexChars(0xA, buf);
  EXPECT_STREQ("0A", buf);
  FXSYS_IntToTwoHexChars(0x8C, buf);
  EXPECT_STREQ("8C", buf);
  FXSYS_IntToTwoHexChars(0xBE, buf);
  EXPECT_STREQ("BE", buf);
  FXSYS_IntToTwoHexChars(0xD0, buf);
  EXPECT_STREQ("D0", buf);
  FXSYS_IntToTwoHexChars(0xFF, buf);
  EXPECT_STREQ("FF", buf);
}

TEST(fxcrt, FXSYS_IntToFourHexChars) {
  char buf[5] = {0};
  FXSYS_IntToFourHexChars(0x0, buf);
  EXPECT_STREQ("0000", buf);
  FXSYS_IntToFourHexChars(0xA23, buf);
  EXPECT_STREQ("0A23", buf);
  FXSYS_IntToFourHexChars(0xB701, buf);
  EXPECT_STREQ("B701", buf);
  FXSYS_IntToFourHexChars(0xFFFF, buf);
  EXPECT_STREQ("FFFF", buf);
}

TEST(fxcrt, FXSYS_ToUTF16BE) {
  char buf[9] = {0};
  // Test U+0000 to U+D7FF and U+E000 to U+FFFF
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0x0, buf));
  EXPECT_STREQ("0000", buf);
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xD7FF, buf));
  EXPECT_STREQ("D7FF", buf);
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xE000, buf));
  EXPECT_STREQ("E000", buf);
  EXPECT_EQ(4U, FXSYS_ToUTF16BE(0xFFFF, buf));
  EXPECT_STREQ("FFFF", buf);
  // Test U+10000 to U+10FFFF
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x10000, buf));
  EXPECT_STREQ("D800DC00", buf);
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x10FFFF, buf));
  EXPECT_STREQ("DBFFDFFF", buf);
  EXPECT_EQ(8U, FXSYS_ToUTF16BE(0x2003E, buf));
  EXPECT_STREQ("D840DC3E", buf);
}

TEST(fxcrt, GetBits32) {
  unsigned char data[] = {0xDE, 0x3F, 0xB1, 0x7C, 0x12, 0x9A, 0x04, 0x56};
  for (int nbits = 1; nbits <= 32; ++nbits) {
    for (int bitpos = 0; bitpos < (int)sizeof(data) * 8 - nbits; ++bitpos) {
      EXPECT_EQ(ReferenceGetBits32(data, bitpos, nbits),
                GetBits32(data, bitpos, nbits));
    }
  }
}
