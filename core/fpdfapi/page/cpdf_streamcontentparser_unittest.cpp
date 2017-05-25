// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_streamcontentparser.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_streamcontentparser, PDF_FindKeyAbbreviation) {
  CPDF_StreamContentParser parser(nullptr, nullptr, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, nullptr, 0);

  EXPECT_EQ(CFX_ByteStringC("BitsPerComponent"),
            parser.FindKeyAbbreviationForTesting(CFX_ByteStringC("BPC")));
  EXPECT_EQ(CFX_ByteStringC("Width"),
            parser.FindKeyAbbreviationForTesting(CFX_ByteStringC("W")));
  EXPECT_EQ(CFX_ByteStringC(""),
            parser.FindKeyAbbreviationForTesting(CFX_ByteStringC("")));
  EXPECT_EQ(CFX_ByteStringC(""),
            parser.FindKeyAbbreviationForTesting(CFX_ByteStringC("NoInList")));
  // Prefix should not match.
  EXPECT_EQ(CFX_ByteStringC(""),
            parser.FindKeyAbbreviationForTesting(CFX_ByteStringC("WW")));
}

TEST(cpdf_streamcontentparser, PDF_FindValueAbbreviation) {
  CPDF_StreamContentParser parser(nullptr, nullptr, nullptr, nullptr, nullptr,
                                  nullptr, nullptr, nullptr, 0);

  EXPECT_EQ(CFX_ByteStringC("DeviceGray"),
            parser.FindValueAbbreviationForTesting(CFX_ByteStringC("G")));
  EXPECT_EQ(CFX_ByteStringC("DCTDecode"),
            parser.FindValueAbbreviationForTesting(CFX_ByteStringC("DCT")));
  EXPECT_EQ(CFX_ByteStringC(""),
            parser.FindValueAbbreviationForTesting(CFX_ByteStringC("")));
  EXPECT_EQ(CFX_ByteStringC(""), parser.FindValueAbbreviationForTesting(
                                     CFX_ByteStringC("NoInList")));
  // Prefix should not match.
  EXPECT_EQ(CFX_ByteStringC(""),
            parser.FindValueAbbreviationForTesting(CFX_ByteStringC("II")));
}
