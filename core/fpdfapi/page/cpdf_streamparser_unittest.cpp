// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_streamparser.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ElementsAre;
using testing::IsEmpty;

TEST(cpdf_streamparser, ReadHexString) {
  {
    // Position out of bounds.
    uint8_t data[] = "12ab>";
    CPDF_StreamParser parser(data);
    parser.SetPos(6);
    EXPECT_THAT(parser.ReadHexString(), IsEmpty());
  }

  {
    // Regular conversion.
    uint8_t data[] = "1A2b>abcd";
    CPDF_StreamParser parser(data);
    EXPECT_THAT(parser.ReadHexString(), ElementsAre(0x1a, 0x2b));
    EXPECT_EQ(5u, parser.GetPos());
  }

  {
    // Missing ending >
    uint8_t data[] = "1A2b";
    CPDF_StreamParser parser(data);
    EXPECT_THAT(parser.ReadHexString(), ElementsAre(0x1a, 0x2b));
    EXPECT_EQ(5u, parser.GetPos());
  }

  {
    // Uneven number of bytes.
    uint8_t data[] = "1A2>asdf";
    CPDF_StreamParser parser(data);
    EXPECT_THAT(parser.ReadHexString(), ElementsAre(0x1a, 0x20));
    EXPECT_EQ(4u, parser.GetPos());
  }

  {
    uint8_t data[] = ">";
    CPDF_StreamParser parser(data);
    EXPECT_THAT(parser.ReadHexString(), IsEmpty());
    EXPECT_EQ(1u, parser.GetPos());
  }
}
