// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fpdfapi/fpdf_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fpdf_parser_decode, HexDecode) {
  {
    // Empty src string.
    uint8_t* dest = nullptr;
    FX_DWORD dest_size;
    uint8_t src[] = "";
    EXPECT_EQ(0, HexDecode(src, 0, dest, dest_size));
    EXPECT_EQ(0, dest_size);
    EXPECT_EQ('\0', dest[0]);
    FX_Free(dest);
  }

  {
    // Regular conversion.
    uint8_t* dest = nullptr;
    FX_DWORD dest_size;
    uint8_t src[] = "12Ac>zzz";
    EXPECT_EQ(5, HexDecode(src, 8, dest, dest_size));
    EXPECT_EQ(2, dest_size);
    EXPECT_EQ(18, dest[0]);
    EXPECT_EQ(172, dest[1]);
    FX_Free(dest);
  }

  {
    // Non-multiple length.
    uint8_t* dest = nullptr;
    FX_DWORD dest_size;
    uint8_t src[] = "12A>zzz";
    EXPECT_EQ(4, HexDecode(src, 8, dest, dest_size));
    EXPECT_EQ(2, dest_size);
    EXPECT_EQ(18, dest[0]);
    EXPECT_EQ(160, dest[1]);
    FX_Free(dest);
  }

  {
    // Skips unknown characters.
    uint8_t* dest = nullptr;
    FX_DWORD dest_size;
    uint8_t src[] = "12tk  \tAc>zzz";
    EXPECT_EQ(10, HexDecode(src, 13, dest, dest_size));
    EXPECT_EQ(2, dest_size);
    EXPECT_EQ(18, dest[0]);
    EXPECT_EQ(172, dest[1]);
    FX_Free(dest);
  }
}
