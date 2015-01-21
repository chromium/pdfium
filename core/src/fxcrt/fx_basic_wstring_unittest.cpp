// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "../../../testing/fx_string_testhelpers.h"
#include "../../include/fxcrt/fx_basic.h"

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
