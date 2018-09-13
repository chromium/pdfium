// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxstring, FX_atonum) {
  int i;
  EXPECT_TRUE(FX_atonum("10", &i));
  EXPECT_EQ(10, i);

  EXPECT_TRUE(FX_atonum("-10", &i));
  EXPECT_EQ(-10, i);

  EXPECT_TRUE(FX_atonum("+10", &i));
  EXPECT_EQ(10, i);

  EXPECT_TRUE(FX_atonum("-2147483648", &i));
  EXPECT_EQ(std::numeric_limits<int>::min(), i);

  EXPECT_TRUE(FX_atonum("2147483647", &i));
  EXPECT_EQ(2147483647, i);

  // Value overflows.
  EXPECT_TRUE(FX_atonum("-2147483649", &i));
  EXPECT_EQ(0, i);

  // Value overflows.
  EXPECT_TRUE(FX_atonum("+2147483648", &i));
  EXPECT_EQ(0, i);

  // Value overflows.
  EXPECT_TRUE(FX_atonum("4223423494965252", &i));
  EXPECT_EQ(0, i);

  // No explicit sign will allow the number to go negative. This is for things
  // like the encryption Permissions flag (Table 3.20 PDF 1.7 spec)
  EXPECT_TRUE(FX_atonum("4294965252", &i));
  EXPECT_EQ(-2044, i);

  EXPECT_TRUE(FX_atonum("-4294965252", &i));
  EXPECT_EQ(0, i);

  EXPECT_TRUE(FX_atonum("+4294965252", &i));
  EXPECT_EQ(0, i);

  float f;
  EXPECT_FALSE(FX_atonum("3.24", &f));
  EXPECT_FLOAT_EQ(3.24f, f);
}

TEST(fxstring, FX_UTF8Encode) {
  EXPECT_EQ("", FX_UTF8Encode(WideStringView()));
  EXPECT_EQ(
      "x"
      "\xc2\x80"
      "\xc3\xbf"
      "\xef\xbc\xac"
      "y",
      FX_UTF8Encode(L"x"
                    L"\u0080"
                    L"\u00ff"
                    L"\uff2c"
                    L"y"));
}

TEST(fxstring, FX_UTF8Decode) {
  EXPECT_EQ(L"", FX_UTF8Decode(ByteStringView()));
  EXPECT_EQ(
      L"x"
      L"\u0080"
      L"\u00ff"
      L"\uff2c"
      L"y",
      FX_UTF8Decode("x"
                    "\xc2\x80"
                    "\xc3\xbf"
                    "\xef\xbc\xac"
                    "y"));
  EXPECT_EQ(L"a(A) b() c() d() e().",
            FX_UTF8Decode("a(\xc2\x41) "      // Invalid continuation.
                          "b(\xc2\xc2) "      // Invalid continuation.
                          "c(\xc2\xff\x80) "  // Invalid continuation.
                          "d(\x80\x80) "      // Invalid leading.
                          "e(\xff\x80\x80)"   // Invalid leading.
                          "."));
}

TEST(fxstring, FX_UTF8EncodeDecodeConsistency) {
  WideString wstr;
  wstr.Reserve(0x10000);
  for (int w = 0; w < 0x10000; ++w)
    wstr += static_cast<wchar_t>(w);

  ByteString bstr = FX_UTF8Encode(wstr.AsStringView());
  WideString wstr2 = FX_UTF8Decode(bstr.AsStringView());
  EXPECT_EQ(0x10000u, wstr2.GetLength());
  EXPECT_EQ(wstr, wstr2);
}
