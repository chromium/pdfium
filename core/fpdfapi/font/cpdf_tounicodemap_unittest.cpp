// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_tounicodemap.h"

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDFToUnicodeMapTest, StringToCode) {
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<0001>"), testing::Optional(1u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<c2>"), testing::Optional(194u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<A2>"), testing::Optional(162u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<Af2>"),
              testing::Optional(2802u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<FFFFFFFF>"),
              testing::Optional(4294967295u));

  // Whitespaces within the string are ignored.
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<00\n0\r1>"),
              testing::Optional(1u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<c 2>"),
              testing::Optional(194u));
  EXPECT_THAT(CPDF_ToUnicodeMap::StringToCode("<A2\r\n>"),
              testing::Optional(162u));

  // Integer overflow
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<100000000>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1abcdFFFF>").has_value());

  // Invalid string
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<12").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("12>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<1-7>").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("00AB").has_value());
  EXPECT_FALSE(CPDF_ToUnicodeMap::StringToCode("<00NN>").has_value());
}

TEST(CPDFToUnicodeMapTest, StringToWideString) {
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString(""));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("1234"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("<c2D2"));
  EXPECT_EQ(L"", CPDF_ToUnicodeMap::StringToWideString("c2ab>"));

  WideString res = L"\xc2ab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abab>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2ab 1234>"));

  res += L"\xfaab";
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb>"));
  EXPECT_EQ(res, CPDF_ToUnicodeMap::StringToWideString("<c2abFaAb12>"));
}

TEST(CPDFToUnicodeMapTest, HandleBeginBFRangeAvoidIntegerOverflow) {
  // Make sure there won't be infinite loops due to integer overflows in
  // HandleBeginBFRange().
  {
    static constexpr uint8_t kInput1[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF>[<0041>]endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput1);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(L"A", map.Lookup(0xffffffff));
  }
  {
    static constexpr uint8_t kInput2[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF><0042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput2);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(L"B", map.Lookup(0xffffffff));
  }
  {
    static constexpr uint8_t kInput3[] =
        "beginbfrange<FFFFFFFF><FFFFFFFF><00410042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput3);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(L"AB", map.Lookup(0xffffffff));
  }
}

TEST(CPDFToUnicodeMapTest, InsertIntoMultimap) {
  {
    // Both the CIDs and the unicodes are different.
    static constexpr uint8_t kInput1[] =
        "beginbfchar<1><0041><2><0042>endbfchar";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput1);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(1u, map.ReverseLookup(0x0041));
    EXPECT_EQ(2u, map.ReverseLookup(0x0042));
    EXPECT_EQ(1u, map.GetUnicodeCountByCharcodeForTesting(1u));
    EXPECT_EQ(1u, map.GetUnicodeCountByCharcodeForTesting(2u));
  }
  {
    // The same CID with different unicodes.
    static constexpr uint8_t kInput2[] =
        "beginbfrange<0><0><0041><0><0><0042>endbfrange";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput2);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(0u, map.ReverseLookup(0x0041));
    EXPECT_EQ(0u, map.ReverseLookup(0x0042));
    EXPECT_EQ(2u, map.GetUnicodeCountByCharcodeForTesting(0u));
  }
  {
    // Duplicate mappings of CID 0 to unicode "A". There should be only 1 entry
    // in `m_Multimap`.
    static constexpr uint8_t kInput3[] =
        "beginbfrange<0><0>[<0041>]endbfrange\n"
        "beginbfchar<0><0041>endbfchar";
    auto stream = pdfium::MakeRetain<CPDF_Stream>(kInput3);
    CPDF_ToUnicodeMap map(stream);
    EXPECT_EQ(0u, map.ReverseLookup(0x0041));
    EXPECT_EQ(1u, map.GetUnicodeCountByCharcodeForTesting(0u));
  }
}

TEST(CPDFToUnicodeMapTest, NonBmpUnicodeLookup) {
  static constexpr uint8_t kInput[] = "beginbfchar<01><d841de76>endbfchar";
  CPDF_ToUnicodeMap map(pdfium::MakeRetain<CPDF_Stream>(kInput));
  EXPECT_EQ(L"\xd841\xde76", map.Lookup(0x01));
#if defined(WCHAR_T_IS_32_BIT)
  // TODO(crbug.com/374947848): Should work if wchar_t is 16-bit.
  // TODO(crbug.com/374947848): Should return 1u.
  EXPECT_EQ(0u, map.ReverseLookup(0x20676));
#endif
}
