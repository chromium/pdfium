// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "../../../testing/fx_string_testhelpers.h"
#include "../../include/fxcrt/fx_basic.h"

TEST(fxcrt, WideStringOperatorSubscript) {
    // CFX_WideString includes the NUL terminator for non-empty strings.
    CFX_WideString abc(L"abc");
    EXPECT_EQ(L'a', abc[0]);
    EXPECT_EQ(L'b', abc[1]);
    EXPECT_EQ(L'c', abc[2]);
    EXPECT_EQ(L'\0', abc[3]);
}

TEST(fxcrt, WideStringOperatorLT) {
    CFX_WideString empty;
    CFX_WideString a(L"a");
    CFX_WideString abc(L"\x0110qq");  // Comes before despite endianness.
    CFX_WideString def(L"\x1001qq");  // Comes after despite endianness.

    EXPECT_FALSE(empty < empty);
    EXPECT_FALSE(a < a);
    EXPECT_FALSE(abc < abc);
    EXPECT_FALSE(def < def);

    EXPECT_TRUE(empty < a);
    EXPECT_FALSE(a < empty);

    EXPECT_TRUE(empty < abc);
    EXPECT_FALSE(abc < empty);

    EXPECT_TRUE(empty < def);
    EXPECT_FALSE(def < empty);

    EXPECT_TRUE(a < abc);
    EXPECT_FALSE(abc < a);

    EXPECT_TRUE(a < def);
    EXPECT_FALSE(def < a);

    EXPECT_TRUE(abc < def);
    EXPECT_FALSE(def < abc);
}

TEST(fxcrt, WideStringOperatorEQ) {
    CFX_WideString wide_string(L"hello");
    ASSERT_TRUE(wide_string == wide_string);

    CFX_WideString wide_string_same1(L"hello");
    ASSERT_TRUE(wide_string == wide_string_same1);
    ASSERT_TRUE(wide_string_same1 == wide_string);

    CFX_WideString wide_string_same2(wide_string);
    ASSERT_TRUE(wide_string == wide_string_same2);
    ASSERT_TRUE(wide_string_same2 == wide_string);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    ASSERT_FALSE(wide_string == wide_string1);
    ASSERT_FALSE(wide_string == wide_string2);
    ASSERT_FALSE(wide_string == wide_string3);
    ASSERT_FALSE(wide_string1 == wide_string);
    ASSERT_FALSE(wide_string2 == wide_string);
    ASSERT_FALSE(wide_string3 == wide_string);

    CFX_WideStringC wide_string_c_same1(L"hello");
    ASSERT_TRUE(wide_string == wide_string_c_same1);
    ASSERT_TRUE(wide_string_c_same1 == wide_string);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    ASSERT_FALSE(wide_string == wide_string_c1);
    ASSERT_FALSE(wide_string == wide_string_c2);
    ASSERT_FALSE(wide_string == wide_string_c3);
    ASSERT_FALSE(wide_string_c1 == wide_string);
    ASSERT_FALSE(wide_string_c2 == wide_string);
    ASSERT_FALSE(wide_string_c3 == wide_string);

    const wchar_t* c_string_same1 = L"hello";
    ASSERT_TRUE(wide_string == c_string_same1);
#if 0
    // TODO(tsepez): Missing operator - there's a prototype but no actual
    // implementation (at least we already removed implicit c_str() casting).
    ASSERT_TRUE(c_string_same1 == wide_string);
#endif

    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    ASSERT_FALSE(wide_string == c_string1);
    ASSERT_FALSE(wide_string == c_string2);
    ASSERT_FALSE(wide_string == c_string3);
#if 0
    // See above TODO.
    ASSERT_FALSE(c_string1 == wide_string);
    ASSERT_FALSE(c_string2 == wide_string);
    ASSERT_FALSE(c_string3 == wide_string);
#endif
}

TEST(fxcrt, WideStringOperatorNE) {
    CFX_WideString wide_string(L"hello");
    ASSERT_FALSE(wide_string != wide_string);

    CFX_WideString wide_string_same1(L"hello");
    ASSERT_FALSE(wide_string != wide_string_same1);
    ASSERT_FALSE(wide_string_same1 != wide_string);

    CFX_WideString wide_string_same2(wide_string);
    ASSERT_FALSE(wide_string != wide_string_same2);
    ASSERT_FALSE(wide_string_same2 != wide_string);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    ASSERT_TRUE(wide_string != wide_string1);
    ASSERT_TRUE(wide_string != wide_string2);
    ASSERT_TRUE(wide_string != wide_string3);
    ASSERT_TRUE(wide_string1 != wide_string);
    ASSERT_TRUE(wide_string2 != wide_string);
    ASSERT_TRUE(wide_string3 != wide_string);

    CFX_WideStringC wide_string_c_same1(L"hello");
    ASSERT_FALSE(wide_string != wide_string_c_same1);
    ASSERT_FALSE(wide_string_c_same1 != wide_string);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    ASSERT_TRUE(wide_string != wide_string_c1);
    ASSERT_TRUE(wide_string != wide_string_c2);
    ASSERT_TRUE(wide_string != wide_string_c3);
    ASSERT_TRUE(wide_string_c1 != wide_string);
    ASSERT_TRUE(wide_string_c2 != wide_string);
    ASSERT_TRUE(wide_string_c3 != wide_string);

    const wchar_t* c_string_same1 = L"hello";
    ASSERT_FALSE(wide_string != c_string_same1);
#if 0
    // See above TODO.
    ASSERT_FALSE(c_string_same1 != wide_string);
#endif
    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    ASSERT_TRUE(wide_string != c_string1);
    ASSERT_TRUE(wide_string != c_string2);
    ASSERT_TRUE(wide_string != c_string3);
#if 0
    // See above TODO.
    ASSERT_TRUE(c_string1 != wide_string);
    ASSERT_TRUE(c_string2 != wide_string);
    ASSERT_TRUE(c_string3 != wide_string);
#endif
}

#define ByteStringLiteral(str) CFX_ByteString(FX_BSTRC(str))

TEST(fxcrt, WideStringUTF16LE_Encode) {
  struct UTF16LEEncodeCase {
    CFX_WideString ws;
    CFX_ByteString bs;
  } utf16le_encode_cases[] = {
    { L"", ByteStringLiteral("\0\0") },
    { L"abc", ByteStringLiteral("a\0b\0c\0\0\0") },
    { L"abcdef", ByteStringLiteral("a\0b\0c\0d\0e\0f\0\0\0") },
    { L"abc\0def", ByteStringLiteral("a\0b\0c\0\0\0") },
    { L"\xaabb\xccdd", ByteStringLiteral("\xbb\xaa\xdd\xcc\0\0") },
    { L"\x3132\x6162", ByteStringLiteral("\x32\x31\x62\x61\0\0") },
  };

  for (size_t i = 0; i < FX_ArraySize(utf16le_encode_cases); ++i) {
    EXPECT_EQ(utf16le_encode_cases[i].bs,
        utf16le_encode_cases[i].ws.UTF16LE_Encode())
        << " for case number " << i;
  }
}

TEST(fxcrt, WideStringCOperatorSubscript) {
    // CFX_WideStringC includes the NUL terminator for non-empty strings.
    CFX_WideStringC abc(L"abc");
    EXPECT_EQ(L'a', abc[0]);
    EXPECT_EQ(L'b', abc[1]);
    EXPECT_EQ(L'c', abc[2]);
    EXPECT_EQ(L'\0', abc[3]);
}

TEST(fxcrt, WideStringCOperatorLT) {
    CFX_WideStringC empty;
    CFX_WideStringC a(L"a");
    CFX_WideStringC abc(L"\x0110qq");  // Comes before despite endianness.
    CFX_WideStringC def(L"\x1001qq");  // Comes after despite endianness.

    EXPECT_FALSE(empty < empty);
    EXPECT_FALSE(a < a);
    EXPECT_FALSE(abc < abc);
    EXPECT_FALSE(def < def);

    EXPECT_TRUE(empty < a);
    EXPECT_FALSE(a < empty);

    EXPECT_TRUE(empty < abc);
    EXPECT_FALSE(abc < empty);

    EXPECT_TRUE(empty < def);
    EXPECT_FALSE(def < empty);

    EXPECT_TRUE(a < abc);
    EXPECT_FALSE(abc < a);

    EXPECT_TRUE(a < def);
    EXPECT_FALSE(def < a);

    EXPECT_TRUE(abc < def);
    EXPECT_FALSE(def < abc);
}

