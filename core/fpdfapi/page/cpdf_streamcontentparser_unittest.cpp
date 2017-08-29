// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_streamcontentparser.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_streamcontentparser, PDF_FindKeyAbbreviation) {
  EXPECT_EQ(CFX_ByteStringC("BitsPerComponent"),
            CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
                CFX_ByteStringC("BPC")));
  EXPECT_EQ(CFX_ByteStringC("Width"),
            CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
                CFX_ByteStringC("W")));
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
                CFX_ByteStringC("")));
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
                CFX_ByteStringC("NoInList")));
  // Prefix should not match.
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
                CFX_ByteStringC("WW")));
}

TEST(cpdf_streamcontentparser, PDF_FindValueAbbreviation) {
  EXPECT_EQ(CFX_ByteStringC("DeviceGray"),
            CPDF_StreamContentParser::FindValueAbbreviationForTesting(
                CFX_ByteStringC("G")));
  EXPECT_EQ(CFX_ByteStringC("DCTDecode"),
            CPDF_StreamContentParser::FindValueAbbreviationForTesting(
                CFX_ByteStringC("DCT")));
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindValueAbbreviationForTesting(
                CFX_ByteStringC("")));
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindValueAbbreviationForTesting(
                CFX_ByteStringC("NoInList")));
  // Prefix should not match.
  EXPECT_EQ(CFX_ByteStringC(""),
            CPDF_StreamContentParser::FindValueAbbreviationForTesting(
                CFX_ByteStringC("II")));
}
