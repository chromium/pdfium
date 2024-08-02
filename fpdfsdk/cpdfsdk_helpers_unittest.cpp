// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cpdfsdk_helpers.h"

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(CPDFSDKHelpersTest, NulTerminateMaybeCopyAndReturnLength) {
  {
    const ByteString to_be_copied("toBeCopied");
    constexpr size_t kExpectedToBeCopiedLen = 10;
    ASSERT_EQ(kExpectedToBeCopiedLen, to_be_copied.GetLength());
    EXPECT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(to_be_copied,
                                                   pdfium::span<char>()));

    // Buffer should not change if declared length is too short.
    char buf[kExpectedToBeCopiedLen + 1];
    fxcrt::Fill(buf, 0x42);
    ASSERT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(
                  to_be_copied,
                  pdfium::make_span(buf).first(kExpectedToBeCopiedLen)));
    for (char c : buf)
      EXPECT_EQ(0x42, c);

    // Buffer should copy over if long enough.
    ASSERT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(to_be_copied,
                                                   pdfium::make_span(buf)));
    EXPECT_EQ(to_be_copied, ByteString(buf));
  }
  {
    // Empty ByteString should still copy NUL terminator.
    const ByteString empty;
    char buf[1];
    ASSERT_EQ(1u, NulTerminateMaybeCopyAndReturnLength(empty,
                                                       pdfium::make_span(buf)));
    EXPECT_EQ(empty, ByteString(buf));
  }
}

TEST(CPDFSDKHelpersTest, ParsePageRangeString) {
  EXPECT_THAT(ParsePageRangeString("", 1), IsEmpty());
  EXPECT_THAT(ParsePageRangeString(" ", 1), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("clams", 1), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("0", 0), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1", 0), IsEmpty());
  EXPECT_THAT(ParsePageRangeString(",1", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1,", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1,clams", 1), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("clams,1", 1), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("0-1", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-0", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-5", 4), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-11,", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString(",1-1", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-,", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("-2,", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-clams", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("clams-1,", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-2clams", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("0,1", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1,0", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-2,,,,3-4", 10), IsEmpty());
  EXPECT_THAT(ParsePageRangeString("1-2-", 10), IsEmpty());

  EXPECT_THAT(ParsePageRangeString("1-1", 10), ElementsAre(0));
  EXPECT_THAT(ParsePageRangeString("1", 1), ElementsAre(0));
  EXPECT_THAT(ParsePageRangeString("1-4", 4), ElementsAre(0, 1, 2, 3));
  EXPECT_THAT(ParsePageRangeString("1- 4", 4), ElementsAre(0, 1, 2, 3));
  EXPECT_THAT(ParsePageRangeString("1 -4", 4), ElementsAre(0, 1, 2, 3));
  EXPECT_THAT(ParsePageRangeString("1,2", 10), ElementsAre(0, 1));
  EXPECT_THAT(ParsePageRangeString("2,1", 10), ElementsAre(1, 0));
  EXPECT_THAT(ParsePageRangeString("1,50,2", 100), ElementsAre(0, 49, 1));
  EXPECT_THAT(ParsePageRangeString("1-4,50", 100), ElementsAre(0, 1, 2, 3, 49));
  EXPECT_THAT(ParsePageRangeString("50,1-2", 100), ElementsAre(49, 0, 1));
  EXPECT_THAT(ParsePageRangeString("5  0, 1-2 ", 100),
              ElementsAre(49, 0, 1));  // ???
  EXPECT_THAT(ParsePageRangeString("1-3,4-6", 10),
              ElementsAre(0, 1, 2, 3, 4, 5));
  EXPECT_THAT(ParsePageRangeString("1-4,3-6", 10),
              ElementsAre(0, 1, 2, 3, 2, 3, 4, 5));
}
