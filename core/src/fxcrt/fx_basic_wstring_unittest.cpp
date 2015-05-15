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
    CFX_WideString null_string;
    EXPECT_TRUE(null_string == null_string);

    CFX_WideString empty_string(L"");
    EXPECT_TRUE(empty_string == empty_string);
    EXPECT_TRUE(empty_string == null_string);
    EXPECT_TRUE(null_string == empty_string);

    CFX_WideString deleted_string(L"hello");
    deleted_string.Delete(0, 5);
    EXPECT_TRUE(deleted_string == deleted_string);
    EXPECT_TRUE(deleted_string == null_string);
    EXPECT_TRUE(deleted_string == empty_string);
    EXPECT_TRUE(null_string == deleted_string);
    EXPECT_TRUE(null_string == empty_string);

    CFX_WideString wide_string(L"hello");
    EXPECT_TRUE(wide_string == wide_string);
    EXPECT_FALSE(wide_string == null_string);
    EXPECT_FALSE(wide_string == empty_string);
    EXPECT_FALSE(wide_string == deleted_string);
    EXPECT_FALSE(null_string == wide_string);
    EXPECT_FALSE(empty_string == wide_string);
    EXPECT_FALSE(deleted_string == wide_string);

    CFX_WideString wide_string_same1(L"hello");
    EXPECT_TRUE(wide_string == wide_string_same1);
    EXPECT_TRUE(wide_string_same1 == wide_string);

    CFX_WideString wide_string_same2(wide_string);
    EXPECT_TRUE(wide_string == wide_string_same2);
    EXPECT_TRUE(wide_string_same2 == wide_string);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    EXPECT_FALSE(wide_string == wide_string1);
    EXPECT_FALSE(wide_string == wide_string2);
    EXPECT_FALSE(wide_string == wide_string3);
    EXPECT_FALSE(wide_string1 == wide_string);
    EXPECT_FALSE(wide_string2 == wide_string);
    EXPECT_FALSE(wide_string3 == wide_string);

    CFX_WideStringC null_string_c;
    CFX_WideStringC empty_string_c(L"");
    EXPECT_TRUE(null_string == null_string_c);
    EXPECT_TRUE(null_string == empty_string_c);
    EXPECT_TRUE(empty_string == null_string_c);
    EXPECT_TRUE(empty_string == empty_string_c);
    EXPECT_TRUE(deleted_string == null_string_c);
    EXPECT_TRUE(deleted_string == empty_string_c);
    EXPECT_TRUE(null_string_c == null_string);
    EXPECT_TRUE(empty_string_c == null_string);
    EXPECT_TRUE(null_string_c == empty_string);
    EXPECT_TRUE(empty_string_c == empty_string);
    EXPECT_TRUE(null_string_c == deleted_string);
    EXPECT_TRUE(empty_string_c == deleted_string);

    CFX_WideStringC wide_string_c_same1(L"hello");
    EXPECT_TRUE(wide_string == wide_string_c_same1);
    EXPECT_TRUE(wide_string_c_same1 == wide_string);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    EXPECT_FALSE(wide_string == wide_string_c1);
    EXPECT_FALSE(wide_string == wide_string_c2);
    EXPECT_FALSE(wide_string == wide_string_c3);
    EXPECT_FALSE(wide_string_c1 == wide_string);
    EXPECT_FALSE(wide_string_c2 == wide_string);
    EXPECT_FALSE(wide_string_c3 == wide_string);

    const wchar_t* c_null_string = nullptr;
    const wchar_t* c_empty_string = L"";
    EXPECT_TRUE(null_string == c_null_string);
    EXPECT_TRUE(null_string == c_empty_string);
    EXPECT_TRUE(empty_string == c_null_string);
    EXPECT_TRUE(empty_string == c_empty_string);
    EXPECT_TRUE(deleted_string == c_null_string);
    EXPECT_TRUE(deleted_string == c_empty_string);
    EXPECT_TRUE(c_null_string == null_string);
    EXPECT_TRUE(c_empty_string == null_string);
    EXPECT_TRUE(c_null_string == empty_string);
    EXPECT_TRUE(c_empty_string == empty_string);
    EXPECT_TRUE(c_null_string == deleted_string);
    EXPECT_TRUE(c_empty_string == deleted_string);

    const wchar_t* c_string_same1 = L"hello";
    EXPECT_TRUE(wide_string == c_string_same1);
    EXPECT_TRUE(c_string_same1 == wide_string);

    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    EXPECT_FALSE(wide_string == c_string1);
    EXPECT_FALSE(wide_string == c_string2);
    EXPECT_FALSE(wide_string == c_string3);
    EXPECT_FALSE(c_string1 == wide_string);
    EXPECT_FALSE(c_string2 == wide_string);
    EXPECT_FALSE(c_string3 == wide_string);
}

TEST(fxcrt, WideStringOperatorNE) {
    CFX_WideString null_string;
    EXPECT_FALSE(null_string != null_string);

    CFX_WideString empty_string(L"");
    EXPECT_FALSE(empty_string != empty_string);
    EXPECT_FALSE(empty_string != null_string);
    EXPECT_FALSE(null_string != empty_string);

    CFX_WideString deleted_string(L"hello");
    deleted_string.Delete(0, 5);
    EXPECT_FALSE(deleted_string != deleted_string);
    EXPECT_FALSE(deleted_string != null_string);
    EXPECT_FALSE(deleted_string != empty_string);
    EXPECT_FALSE(null_string != deleted_string);
    EXPECT_FALSE(null_string != empty_string);

    CFX_WideString wide_string(L"hello");
    EXPECT_FALSE(wide_string != wide_string);
    EXPECT_TRUE(wide_string != null_string);
    EXPECT_TRUE(wide_string != empty_string);
    EXPECT_TRUE(wide_string != deleted_string);
    EXPECT_TRUE(null_string != wide_string);
    EXPECT_TRUE(empty_string != wide_string);
    EXPECT_TRUE(deleted_string != wide_string);

    CFX_WideString wide_string_same1(L"hello");
    EXPECT_FALSE(wide_string != wide_string_same1);
    EXPECT_FALSE(wide_string_same1 != wide_string);

    CFX_WideString wide_string_same2(wide_string);
    EXPECT_FALSE(wide_string != wide_string_same2);
    EXPECT_FALSE(wide_string_same2 != wide_string);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    EXPECT_TRUE(wide_string != wide_string1);
    EXPECT_TRUE(wide_string != wide_string2);
    EXPECT_TRUE(wide_string != wide_string3);
    EXPECT_TRUE(wide_string1 != wide_string);
    EXPECT_TRUE(wide_string2 != wide_string);
    EXPECT_TRUE(wide_string3 != wide_string);

    CFX_WideStringC null_string_c;
    CFX_WideStringC empty_string_c(L"");
    EXPECT_FALSE(null_string != null_string_c);
    EXPECT_FALSE(null_string != empty_string_c);
    EXPECT_FALSE(empty_string != null_string_c);
    EXPECT_FALSE(empty_string != empty_string_c);
    EXPECT_FALSE(deleted_string != null_string_c);
    EXPECT_FALSE(deleted_string != empty_string_c);
    EXPECT_FALSE(null_string_c != null_string);
    EXPECT_FALSE(empty_string_c != null_string);
    EXPECT_FALSE(null_string_c != empty_string);
    EXPECT_FALSE(empty_string_c != empty_string);

    CFX_WideStringC wide_string_c_same1(L"hello");
    EXPECT_FALSE(wide_string != wide_string_c_same1);
    EXPECT_FALSE(wide_string_c_same1 != wide_string);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    EXPECT_TRUE(wide_string != wide_string_c1);
    EXPECT_TRUE(wide_string != wide_string_c2);
    EXPECT_TRUE(wide_string != wide_string_c3);
    EXPECT_TRUE(wide_string_c1 != wide_string);
    EXPECT_TRUE(wide_string_c2 != wide_string);
    EXPECT_TRUE(wide_string_c3 != wide_string);

    const wchar_t* c_null_string = nullptr;
    const wchar_t* c_empty_string = L"";
    EXPECT_FALSE(null_string != c_null_string);
    EXPECT_FALSE(null_string != c_empty_string);
    EXPECT_FALSE(empty_string != c_null_string);
    EXPECT_FALSE(empty_string != c_empty_string);
    EXPECT_FALSE(deleted_string != c_null_string);
    EXPECT_FALSE(deleted_string != c_empty_string);
    EXPECT_FALSE(c_null_string != null_string);
    EXPECT_FALSE(c_empty_string != null_string);
    EXPECT_FALSE(c_null_string != empty_string);
    EXPECT_FALSE(c_empty_string != empty_string);
    EXPECT_FALSE(c_null_string != deleted_string);
    EXPECT_FALSE(c_empty_string != deleted_string);

    const wchar_t* c_string_same1 = L"hello";
    EXPECT_FALSE(wide_string != c_string_same1);
    EXPECT_FALSE(c_string_same1 != wide_string);

    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    EXPECT_TRUE(wide_string != c_string1);
    EXPECT_TRUE(wide_string != c_string2);
    EXPECT_TRUE(wide_string != c_string3);
    EXPECT_TRUE(c_string1 != wide_string);
    EXPECT_TRUE(c_string2 != wide_string);
    EXPECT_TRUE(c_string3 != wide_string);
}

TEST(fxcrt, WideStringConcatInPlace) {
    CFX_WideString fred;
    fred.ConcatInPlace(4, L"FRED");
    EXPECT_EQ(L"FRED", fred);

    fred.ConcatInPlace(2, L"DY");
    EXPECT_EQ(L"FREDDY", fred);

    fred.Delete(3, 3);
    EXPECT_EQ(L"FRE", fred);

    fred.ConcatInPlace(1, L"D");
    EXPECT_EQ(L"FRED", fred);

    CFX_WideString copy = fred;
    fred.ConcatInPlace(2, L"DY");
    EXPECT_EQ(L"FREDDY", fred);
    EXPECT_EQ(L"FRED", copy);

    // Test invalid arguments.
    copy = fred;
    fred.ConcatInPlace(-6, L"freddy");
    CFX_WideString not_aliased(L"xxxxxx");
    EXPECT_EQ(L"FREDDY", fred);
    EXPECT_EQ(L"xxxxxx", not_aliased);
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

TEST(fxcrt, WideStringCOperatorEQ) {
    CFX_WideStringC wide_string_c(L"hello");
    EXPECT_TRUE(wide_string_c == wide_string_c);

    CFX_WideStringC wide_string_c_same1(L"hello");
    EXPECT_TRUE(wide_string_c == wide_string_c_same1);
    EXPECT_TRUE(wide_string_c_same1 == wide_string_c);

    CFX_WideStringC wide_string_c_same2(wide_string_c);
    EXPECT_TRUE(wide_string_c == wide_string_c_same2);
    EXPECT_TRUE(wide_string_c_same2 == wide_string_c);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    EXPECT_FALSE(wide_string_c == wide_string_c1);
    EXPECT_FALSE(wide_string_c == wide_string_c2);
    EXPECT_FALSE(wide_string_c == wide_string_c3);
    EXPECT_FALSE(wide_string_c1 == wide_string_c);
    EXPECT_FALSE(wide_string_c2 == wide_string_c);
    EXPECT_FALSE(wide_string_c3 == wide_string_c);

    CFX_WideString wide_string_same1(L"hello");
    EXPECT_TRUE(wide_string_c == wide_string_same1);
    EXPECT_TRUE(wide_string_same1 == wide_string_c);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    EXPECT_FALSE(wide_string_c == wide_string1);
    EXPECT_FALSE(wide_string_c == wide_string2);
    EXPECT_FALSE(wide_string_c == wide_string3);
    EXPECT_FALSE(wide_string1 == wide_string_c);
    EXPECT_FALSE(wide_string2 == wide_string_c);
    EXPECT_FALSE(wide_string3 == wide_string_c);

    const wchar_t* c_string_same1 = L"hello";
    EXPECT_TRUE(wide_string_c == c_string_same1);
    EXPECT_TRUE(c_string_same1 == wide_string_c);

    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    EXPECT_FALSE(wide_string_c == c_string1);
    EXPECT_FALSE(wide_string_c == c_string2);
    EXPECT_FALSE(wide_string_c == c_string3);

    EXPECT_FALSE(c_string1 == wide_string_c);
    EXPECT_FALSE(c_string2 == wide_string_c);
    EXPECT_FALSE(c_string3 == wide_string_c);
}

TEST(fxcrt, WideStringCOperatorNE) {
    CFX_WideStringC wide_string_c(L"hello");
    EXPECT_FALSE(wide_string_c != wide_string_c);

    CFX_WideStringC wide_string_c_same1(L"hello");
    EXPECT_FALSE(wide_string_c != wide_string_c_same1);
    EXPECT_FALSE(wide_string_c_same1 != wide_string_c);

    CFX_WideStringC wide_string_c_same2(wide_string_c);
    EXPECT_FALSE(wide_string_c != wide_string_c_same2);
    EXPECT_FALSE(wide_string_c_same2 != wide_string_c);

    CFX_WideStringC wide_string_c1(L"he");
    CFX_WideStringC wide_string_c2(L"hellp");
    CFX_WideStringC wide_string_c3(L"hellod");
    EXPECT_TRUE(wide_string_c != wide_string_c1);
    EXPECT_TRUE(wide_string_c != wide_string_c2);
    EXPECT_TRUE(wide_string_c != wide_string_c3);
    EXPECT_TRUE(wide_string_c1 != wide_string_c);
    EXPECT_TRUE(wide_string_c2 != wide_string_c);
    EXPECT_TRUE(wide_string_c3 != wide_string_c);

    CFX_WideString wide_string_same1(L"hello");
    EXPECT_FALSE(wide_string_c != wide_string_same1);
    EXPECT_FALSE(wide_string_same1 != wide_string_c);

    CFX_WideString wide_string1(L"he");
    CFX_WideString wide_string2(L"hellp");
    CFX_WideString wide_string3(L"hellod");
    EXPECT_TRUE(wide_string_c != wide_string1);
    EXPECT_TRUE(wide_string_c != wide_string2);
    EXPECT_TRUE(wide_string_c != wide_string3);
    EXPECT_TRUE(wide_string1 != wide_string_c);
    EXPECT_TRUE(wide_string2 != wide_string_c);
    EXPECT_TRUE(wide_string3 != wide_string_c);

    const wchar_t* c_string_same1 = L"hello";
    EXPECT_FALSE(wide_string_c != c_string_same1);
    EXPECT_FALSE(c_string_same1 != wide_string_c);

    const wchar_t* c_string1 = L"he";
    const wchar_t* c_string2 = L"hellp";
    const wchar_t* c_string3 = L"hellod";
    EXPECT_TRUE(wide_string_c != c_string1);
    EXPECT_TRUE(wide_string_c != c_string2);
    EXPECT_TRUE(wide_string_c != c_string3);

    EXPECT_TRUE(c_string1 != wide_string_c);
    EXPECT_TRUE(c_string2 != wide_string_c);
    EXPECT_TRUE(c_string3 != wide_string_c);
}
