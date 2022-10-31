// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <iterator>
#include <limits>
#include <memory>

#include "core/fxcodec/basic/basicmodule.h"
#include "core/fxcrt/data_vector.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(fxcodec, A85EmptyInput) {
  EXPECT_TRUE(BasicModule::A85Encode({}).empty());
}

// No leftover bytes, just translate 2 sets of symbols.
TEST(fxcodec, A85Basic) {
  // Make sure really big values don't break.
  const uint8_t src_buf[] = {1, 2, 3, 4, 255, 255, 255, 255};
  DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf);

  // Should have 5 chars for each set of 4 and 2 terminators.
  const uint8_t expected_out[] = {33, 60, 78, 63, 43,  115,
                                  56, 87, 45, 33, 126, 62};
  ASSERT_EQ(std::size(expected_out), dest_buf.size());

  // Check the output
  for (uint32_t i = 0; i < dest_buf.size(); i++)
    EXPECT_EQ(expected_out[i], dest_buf[i]) << " at " << i;
}

// Leftover bytes.
TEST(fxcodec, A85LeftoverBytes) {
  {
    // 1 Leftover Byte:
    const uint8_t src_buf_1leftover[] = {1, 2, 3, 4, 255};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf_1leftover);

    // 5 chars for first symbol + 2 + 2 terminators.
    uint8_t expected_out_1leftover[] = {33, 60, 78, 63, 43, 114, 114, 126, 62};
    ASSERT_EQ(std::size(expected_out_1leftover), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out_1leftover[i], dest_buf[i]) << " at " << i;
  }

  {
    // 2 Leftover bytes:
    const uint8_t src_buf_2leftover[] = {1, 2, 3, 4, 255, 254};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf_2leftover);
    // 5 chars for first symbol + 3 + 2 terminators.
    const uint8_t expected_out_2leftover[] = {33,  60, 78, 63,  43,
                                              115, 56, 68, 126, 62};
    ASSERT_EQ(std::size(expected_out_2leftover), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out_2leftover[i], dest_buf[i]) << " at " << i;
  }

  {
    // 3 Leftover bytes:
    const uint8_t src_buf_3leftover[] = {1, 2, 3, 4, 255, 254, 253};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf_3leftover);
    // 5 chars for first symbol + 4 + 2 terminators.
    const uint8_t expected_out_3leftover[] = {33, 60, 78,  63,  43, 115,
                                              56, 77, 114, 126, 62};
    ASSERT_EQ(std::size(expected_out_3leftover), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out_3leftover[i], dest_buf[i]) << " at " << i;
  }
}

// Test all zeros comes through as "z".
TEST(fxcodec, A85Zeros) {
  {
    // Make sure really big values don't break.
    const uint8_t src_buf[] = {1, 2, 3, 4, 0, 0, 0, 0};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf);

    // Should have 5 chars for first set of 4 + 1 for z + 2 terminators.
    const uint8_t expected_out[] = {33, 60, 78, 63, 43, 122, 126, 62};
    ASSERT_EQ(std::size(expected_out), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out[i], dest_buf[i]) << " at " << i;
  }

  {
    // Should also work if it is at the start:
    const uint8_t src_buf_2[] = {0, 0, 0, 0, 1, 2, 3, 4};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf_2);

    // Should have 5 chars for set of 4 + 1 for z + 2 terminators.
    const uint8_t expected_out_2[] = {122, 33, 60, 78, 63, 43, 126, 62};
    ASSERT_EQ(std::size(expected_out_2), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out_2[i], dest_buf[i]) << " at " << i;
  }

  {
    // Try with 2 leftover zero bytes. Make sure we don't get a "z".
    const uint8_t src_buf_3[] = {1, 2, 3, 4, 0, 0};
    DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf_3);

    // Should have 5 chars for set of 4 + 3 for last 2 + 2 terminators.
    const uint8_t expected_out_leftover[] = {33, 60, 78, 63,  43,
                                             33, 33, 33, 126, 62};
    ASSERT_EQ(std::size(expected_out_leftover), dest_buf.size());

    // Check the output
    for (uint32_t i = 0; i < dest_buf.size(); i++)
      EXPECT_EQ(expected_out_leftover[i], dest_buf[i]) << " at " << i;
  }
}

// Make sure we get returns in the expected locations.
TEST(fxcodec, A85LineBreaks) {
  // Make sure really big values don't break.
  uint8_t src_buf[131] = {0};
  // 1 full line + most of a line of normal symbols.
  for (int k = 0; k < 116; k += 4) {
    src_buf[k] = 1;
    src_buf[k + 1] = 2;
    src_buf[k + 2] = 3;
    src_buf[k + 3] = 4;
  }
  // Fill in the end, leaving an all zero gap + 3 extra zeros at the end.
  for (int k = 120; k < 128; k++) {
    src_buf[k] = 1;
    src_buf[k + 1] = 2;
    src_buf[k + 2] = 3;
    src_buf[k + 3] = 4;
  }

  // Should succeed.
  DataVector<uint8_t> dest_buf = BasicModule::A85Encode(src_buf);

  // Should have 75 chars in the first row plus 2 char return,
  // 76 chars in the second row plus 2 char return,
  // and 9 chars in the last row with 2 terminators.
  ASSERT_EQ(166u, dest_buf.size());

  // Check for the returns.
  EXPECT_EQ(13, dest_buf[75]);
  EXPECT_EQ(10, dest_buf[76]);
  EXPECT_EQ(13, dest_buf[153]);
  EXPECT_EQ(10, dest_buf[154]);
}
