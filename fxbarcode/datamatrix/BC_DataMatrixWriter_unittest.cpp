// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxbarcode/datamatrix/BC_DataMatrixWriter.h"

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ElementsAreArray;

class CBCDataMatrixWriterTest : public testing::Test {
 public:
  CBCDataMatrixWriterTest() = default;
  ~CBCDataMatrixWriterTest() override = default;

  // testing::Test:
  void SetUp() override { BC_Library_Init(); }
  void TearDown() override { BC_Library_Destroy(); }
};

TEST_F(CBCDataMatrixWriterTest, Encode) {
  CBC_DataMatrixWriter writer;
  int32_t width = -1;
  int32_t height = -1;

  {
    static constexpr int kExpectedDimension = 10;
    // clang-format off
    static constexpr uint8_t kExpectedData[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
        1, 1, 0, 1, 0, 0, 0, 0, 1, 0,
        1, 1, 1, 1, 0, 0, 0, 1, 0, 1,
        1, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 1, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    // clang-format on
    DataVector<uint8_t> data = writer.Encode(L"", &width, &height);
    ASSERT_EQ(std::size(kExpectedData), data.size());
    ASSERT_EQ(kExpectedDimension, width);
    ASSERT_EQ(kExpectedDimension, height);
    EXPECT_THAT(data, ElementsAreArray(kExpectedData));
  }
  {
    static constexpr int kExpectedDimension = 14;
    // clang-format off
    static constexpr uint8_t kExpectedData[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
        1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0,
        1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1,
        1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0,
        1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0,
        1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0,
        1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    // clang-format on
    DataVector<uint8_t> data = writer.Encode(L"helloworld", &width, &height);
    ASSERT_EQ(std::size(kExpectedData), data.size());
    ASSERT_EQ(kExpectedDimension, width);
    ASSERT_EQ(kExpectedDimension, height);
    EXPECT_THAT(data, ElementsAreArray(kExpectedData));
  }
  {
    static constexpr int kExpectedDimension = 10;
    // clang-format off
    static constexpr uint8_t kExpectedData[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 1, 0, 1, 1, 0, 0, 1, 1, 1,
        1, 1, 0, 0, 0, 1, 0, 1, 1, 0,
        1, 1, 0, 0, 1, 1, 0, 1, 0, 1,
        1, 1, 0, 0, 1, 1, 1, 0, 0, 0,
        1, 0, 0, 0, 0, 1, 1, 1, 1, 1,
        1, 1, 0, 1, 0, 1, 1, 1, 1, 0,
        1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 0, 1, 1, 0, 0, 1, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    // clang-format on
    DataVector<uint8_t> data = writer.Encode(L"12345", &width, &height);
    ASSERT_EQ(std::size(kExpectedData), data.size());
    ASSERT_EQ(kExpectedDimension, width);
    ASSERT_EQ(kExpectedDimension, height);
    EXPECT_THAT(data, ElementsAreArray(kExpectedData));
  }
  {
    static constexpr int kExpectedDimension = 18;
    // clang-format off
    static constexpr uint8_t kExpectedData[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1,
        1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0,
        1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1,
        1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0,
        1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0,
        1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1,
        1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,
        1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1,
        1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0,
        1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
        1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0,
        1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1,
        1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0,
        1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };
    // clang-format on
    DataVector<uint8_t> data =
        writer.Encode(L"abcdefghijklmnopqrst", &width, &height);
    ASSERT_EQ(std::size(kExpectedData), data.size());
    ASSERT_EQ(kExpectedDimension, width);
    ASSERT_EQ(kExpectedDimension, height);
    EXPECT_THAT(data, ElementsAreArray(kExpectedData));
  }
  {
    DataVector<uint8_t> data = writer.Encode(L"hello world", &width, &height);
    ASSERT_TRUE(data.empty());
  }
}

TEST_F(CBCDataMatrixWriterTest, EncodeLimitAlphaNumeric) {
  CBC_DataMatrixWriter writer;
  int32_t width = -1;
  int32_t height = -1;

  static constexpr int kMaxInputLength = 2335;  // Per spec.
  WideString input;
  for (size_t i = 0; i < kMaxInputLength; ++i)
    input.InsertAtBack(L'a');

  {
    static constexpr int kExpectedDimension = 144;
    DataVector<uint8_t> data = writer.Encode(input.c_str(), &width, &height);
    EXPECT_EQ(20736u, data.size());
    EXPECT_EQ(kExpectedDimension, width);
    EXPECT_EQ(kExpectedDimension, height);
  }

  // Go over the limit.
  input.InsertAtBack(L'a');
  {
    width = -1;
    height = -1;
    DataVector<uint8_t> data = writer.Encode(input.c_str(), &width, &height);
    EXPECT_EQ(0u, data.size());
    EXPECT_EQ(-1, width);
    EXPECT_EQ(-1, height);
  }
}

TEST_F(CBCDataMatrixWriterTest, EncodeLimitNumbers) {
  CBC_DataMatrixWriter writer;
  int32_t width = -1;
  int32_t height = -1;

  static constexpr int kMaxInputLength = 3116;  // Per spec.
  WideString input;
  for (size_t i = 0; i < kMaxInputLength; ++i)
    input.InsertAtBack(L'1');

  {
    static constexpr int kExpectedDimension = 144;
    DataVector<uint8_t> data = writer.Encode(input.c_str(), &width, &height);
    EXPECT_EQ(20736u, data.size());
    EXPECT_EQ(kExpectedDimension, width);
    EXPECT_EQ(kExpectedDimension, height);
  }

  // Go over the limit.
  input.InsertAtBack(L'1');
  {
    width = -1;
    height = -1;
    DataVector<uint8_t> data = writer.Encode(input.c_str(), &width, &height);
    EXPECT_EQ(0u, data.size());
    EXPECT_EQ(-1, width);
    EXPECT_EQ(-1, height);
  }
}
