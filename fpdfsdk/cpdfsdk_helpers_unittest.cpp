// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cpdfsdk_helpers.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDFSDK_HelpersTest, NulTerminateMaybeCopyAndReturnLength) {
  {
    const ByteString to_be_copied("toBeCopied");
    constexpr size_t kExpectedToBeCopiedLen = 10;
    ASSERT_EQ(kExpectedToBeCopiedLen, to_be_copied.GetLength());

    EXPECT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(to_be_copied, nullptr, 0));

    // Buffer should not change if declared length is too short.
    char buf[kExpectedToBeCopiedLen + 1];
    memset(buf, 0x42, kExpectedToBeCopiedLen + 1);
    ASSERT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(to_be_copied, buf,
                                                   kExpectedToBeCopiedLen));
    for (char c : buf)
      EXPECT_EQ(0x42, c);

    // Buffer should copy over if long enough.
    ASSERT_EQ(kExpectedToBeCopiedLen + 1,
              NulTerminateMaybeCopyAndReturnLength(to_be_copied, buf,
                                                   kExpectedToBeCopiedLen + 1));
    EXPECT_EQ(to_be_copied, ByteString(buf));
  }
  {
    // Empty ByteString should still copy NUL terminator.
    const ByteString empty;
    char buf[1];
    ASSERT_EQ(1u, NulTerminateMaybeCopyAndReturnLength(empty, buf, 1));
    EXPECT_EQ(empty, ByteString(buf));
  }
}
