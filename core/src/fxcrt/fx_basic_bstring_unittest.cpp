// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "../../../testing/fx_string_testhelpers.h"
#include "../../include/fxcrt/fx_basic.h"

TEST(fxcrt, ByteStringCNull) {
  CFX_ByteStringC null_string;
  EXPECT_EQ(null_string.GetPtr(), nullptr);
  EXPECT_EQ(null_string.GetLength(), 0);
  EXPECT_TRUE(null_string.IsEmpty());

  CFX_ByteStringC another_null_string;
  EXPECT_EQ(null_string, another_null_string);

  CFX_ByteStringC copied_null_string(null_string);
  EXPECT_EQ(copied_null_string.GetPtr(), nullptr);
  EXPECT_EQ(copied_null_string.GetLength(), 0);
  EXPECT_TRUE(copied_null_string.IsEmpty());
  EXPECT_EQ(null_string, copied_null_string);

  CFX_ByteStringC empty_string("");  // Pointer to NUL, not NULL pointer.
  EXPECT_NE(empty_string.GetPtr(), nullptr);
  EXPECT_EQ(empty_string.GetLength(), 0);
  EXPECT_TRUE(empty_string.IsEmpty());
  EXPECT_EQ(null_string, empty_string);

  CFX_ByteStringC assigned_null_string("initially not NULL");
  assigned_null_string = null_string;
  EXPECT_EQ(assigned_null_string.GetPtr(), nullptr);
  EXPECT_EQ(assigned_null_string.GetLength(), 0);
  EXPECT_TRUE(assigned_null_string.IsEmpty());
  EXPECT_EQ(null_string, assigned_null_string);

  CFX_ByteStringC assigned_nullptr_string("initially not NULL");
  assigned_nullptr_string = (FX_LPCSTR)nullptr;
  EXPECT_EQ(assigned_nullptr_string.GetPtr(), nullptr);
  EXPECT_EQ(assigned_nullptr_string.GetLength(), 0);
  EXPECT_TRUE(assigned_nullptr_string.IsEmpty());
  EXPECT_EQ(null_string, assigned_nullptr_string);

  CFX_ByteStringC non_null_string("a");
  EXPECT_NE(null_string, non_null_string);
}

TEST(fxcrt, ByteStringCNotNull) {
  CFX_ByteStringC string3("abc");
  CFX_ByteStringC string6("abcdef");
  CFX_ByteStringC alternate_string3("abcdef", 3);
  CFX_ByteStringC embedded_nul_string7("abc\0def", 7);
  CFX_ByteStringC illegal_string7("abcdef", 7);

  EXPECT_EQ(3, string3.GetLength());
  EXPECT_EQ(6, string6.GetLength());
  EXPECT_EQ(3, alternate_string3.GetLength());
  EXPECT_EQ(7, embedded_nul_string7.GetLength());
  EXPECT_EQ(7, illegal_string7.GetLength());

  EXPECT_NE(string3, string6);
  EXPECT_EQ(string3, alternate_string3);
  EXPECT_NE(string3, embedded_nul_string7);
  EXPECT_NE(string3, illegal_string7);
  EXPECT_NE(string6, alternate_string3);
  EXPECT_NE(string6, embedded_nul_string7);
  EXPECT_NE(string6, illegal_string7);
  EXPECT_NE(alternate_string3, embedded_nul_string7);
  EXPECT_NE(alternate_string3, illegal_string7);
  EXPECT_NE(embedded_nul_string7, illegal_string7);

  CFX_ByteStringC copied_string3(string3);
  CFX_ByteStringC copied_alternate_string3(alternate_string3);
  CFX_ByteStringC copied_embedded_nul_string7(embedded_nul_string7);

  EXPECT_EQ(string3, copied_string3);
  EXPECT_EQ(alternate_string3, copied_alternate_string3);
  EXPECT_EQ(embedded_nul_string7, copied_embedded_nul_string7);

  CFX_ByteStringC assigned_string3("intially something else");
  CFX_ByteStringC assigned_alternate_string3("initally something else");
  CFX_ByteStringC assigned_ptr_string3("initially something else");
  CFX_ByteStringC assigned_embedded_nul_string7("initially something else");

  assigned_string3 = string3;
  assigned_alternate_string3 = alternate_string3;
  assigned_ptr_string3 = "abc";
  assigned_embedded_nul_string7 = embedded_nul_string7;
  EXPECT_EQ(string3, assigned_string3);
  EXPECT_EQ(alternate_string3, assigned_alternate_string3);
  EXPECT_EQ(alternate_string3, assigned_ptr_string3);
  EXPECT_EQ(embedded_nul_string7, assigned_embedded_nul_string7);
}

TEST(fxcrt, ByteStringCFromChar) {
  CFX_ByteStringC null_string;
  CFX_ByteStringC lower_a_string("a");

  // Must have lvalues that outlive the corresponding ByteStringC.
  char nul = '\0';
  char lower_a = 'a';
  CFX_ByteStringC nul_string_from_char(nul);
  CFX_ByteStringC lower_a_string_from_char(lower_a);

  // Pointer to nul, not NULL ptr, hence length 1 ...
  EXPECT_EQ(1, nul_string_from_char.GetLength());
  EXPECT_NE(null_string, nul_string_from_char);

  EXPECT_EQ(1, lower_a_string_from_char.GetLength());
  EXPECT_EQ(lower_a_string, lower_a_string_from_char);
  EXPECT_NE(nul_string_from_char, lower_a_string_from_char);

  CFX_ByteStringC longer_string("ab");
  EXPECT_NE(longer_string, lower_a_string_from_char);
}

TEST(fxcrt, ByteStringCGetID) {
  CFX_ByteStringC null_string;
  EXPECT_EQ(0u, null_string.GetID());
  EXPECT_EQ(0u, null_string.GetID(1));
  EXPECT_EQ(0u, null_string.GetID(-1));
  EXPECT_EQ(0u, null_string.GetID(-1000000));

  CFX_ByteStringC empty_string("");
  EXPECT_EQ(0u, empty_string.GetID());
  EXPECT_EQ(0u, empty_string.GetID(1));
  EXPECT_EQ(0u, empty_string.GetID(-1));
  EXPECT_EQ(0u, empty_string.GetID(-1000000));

  CFX_ByteStringC short_string("ab");
  EXPECT_EQ(FXBSTR_ID('a', 'b', 0, 0), short_string.GetID());
  EXPECT_EQ(FXBSTR_ID('b', 0, 0, 0), short_string.GetID(1));
  EXPECT_EQ(0u, short_string.GetID(2));
  EXPECT_EQ(0u, short_string.GetID(-1));
  EXPECT_EQ(0u, short_string.GetID(-1000000));

  CFX_ByteStringC longer_string("abcdef");
  EXPECT_EQ(FXBSTR_ID('a', 'b', 'c', 'd'), longer_string.GetID());
  EXPECT_EQ(FXBSTR_ID('b', 'c', 'd', 'e'), longer_string.GetID(1));
  EXPECT_EQ(FXBSTR_ID('c', 'd', 'e', 'f'), longer_string.GetID(2));
  EXPECT_EQ(FXBSTR_ID('d', 'e', 'f', 0), longer_string.GetID(3));
  EXPECT_EQ(FXBSTR_ID('e', 'f', 0 , 0), longer_string.GetID(4));
  EXPECT_EQ(FXBSTR_ID('f', 0 , 0, 0), longer_string.GetID(5));
  EXPECT_EQ(0u, longer_string.GetID(6));
  EXPECT_EQ(0u, longer_string.GetID(-1));
  EXPECT_EQ(0u, longer_string.GetID(-1000000));
}

TEST(fxcrt, ByteStringCMid) {
  CFX_ByteStringC null_string;
  EXPECT_EQ(null_string, null_string.Mid(0, 1));
  EXPECT_EQ(null_string, null_string.Mid(1, 1));

  CFX_ByteStringC empty_string("");
  EXPECT_EQ(empty_string, empty_string.Mid(0, 1));
  EXPECT_EQ(empty_string, empty_string.Mid(1, 1));

  CFX_ByteStringC single_character("a");
  EXPECT_EQ(empty_string, single_character.Mid(0, 0));
  EXPECT_EQ(single_character, single_character.Mid(0, 1));
  EXPECT_EQ(empty_string, single_character.Mid(1, 0));
  EXPECT_EQ(empty_string, single_character.Mid(1, 1));

  CFX_ByteStringC longer_string("abcdef");
  EXPECT_EQ(longer_string, longer_string.Mid(0, 6));
  EXPECT_EQ(longer_string, longer_string.Mid(0, 187));
  EXPECT_EQ(longer_string, longer_string.Mid(-42, 6));
  EXPECT_EQ(longer_string, longer_string.Mid(-42, 187));

  CFX_ByteStringC leading_substring("ab");
  EXPECT_EQ(leading_substring, longer_string.Mid(0, 2));
  EXPECT_EQ(leading_substring, longer_string.Mid(-1, 2));

  CFX_ByteStringC middle_substring("bcde");
  EXPECT_EQ(middle_substring, longer_string.Mid(1, 4));

  CFX_ByteStringC trailing_substring("ef");
  EXPECT_EQ(trailing_substring, longer_string.Mid(4, 2));
  EXPECT_EQ(trailing_substring, longer_string.Mid(4, 3));
}

TEST(fxcrt, ByteStringCGetAt) {
  CFX_ByteString short_string("a");
  CFX_ByteString longer_string("abc");
  CFX_ByteString embedded_nul_string("ab\0c", 4);

  EXPECT_EQ('a', short_string.GetAt(0));
  EXPECT_EQ('c', longer_string.GetAt(2));
  EXPECT_EQ('b', embedded_nul_string.GetAt(1));
  EXPECT_EQ('\0', embedded_nul_string.GetAt(2));
  EXPECT_EQ('c', embedded_nul_string.GetAt(3));
}
