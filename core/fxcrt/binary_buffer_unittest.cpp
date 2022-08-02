// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/binary_buffer.h"

#include <utility>
#include <vector>

#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace fxcrt {

TEST(BinaryBuffer, Empty) {
  BinaryBuffer buffer;
  EXPECT_TRUE(buffer.IsEmpty());
  EXPECT_EQ(0u, buffer.GetSize());
  EXPECT_EQ(0u, buffer.GetLength());
  EXPECT_TRUE(buffer.GetSpan().empty());
}

TEST(BinaryBuffer, MoveConstruct) {
  BinaryBuffer buffer;
  buffer.AppendByte(65u);

  BinaryBuffer buffer2(std::move(buffer));

  EXPECT_TRUE(buffer.IsEmpty());
  EXPECT_EQ(0u, buffer.GetSize());
  EXPECT_EQ(0u, buffer.GetLength());
  EXPECT_TRUE(buffer.GetSpan().empty());

  EXPECT_FALSE(buffer2.IsEmpty());
  EXPECT_EQ(1u, buffer2.GetSize());
  EXPECT_EQ(1u, buffer2.GetLength());
  EXPECT_EQ(65u, buffer2.GetSpan()[0]);
}

TEST(BinaryBuffer, MoveAssign) {
  BinaryBuffer buffer;
  BinaryBuffer buffer2;
  buffer.AppendByte(65u);
  buffer2 = std::move(buffer);

  EXPECT_TRUE(buffer.IsEmpty());
  EXPECT_EQ(0u, buffer.GetSize());
  EXPECT_EQ(0u, buffer.GetLength());
  EXPECT_TRUE(buffer.GetSpan().empty());

  EXPECT_FALSE(buffer2.IsEmpty());
  EXPECT_EQ(1u, buffer2.GetSize());
  ASSERT_EQ(1u, buffer2.GetLength());
  EXPECT_EQ(65u, buffer2.GetSpan()[0]);
}

TEST(BinaryBuffer, Clear) {
  BinaryBuffer buffer;
  buffer.AppendByte(65u);
  buffer.Clear();
  EXPECT_TRUE(buffer.IsEmpty());
  EXPECT_EQ(0u, buffer.GetSize());
  EXPECT_EQ(0u, buffer.GetLength());
  EXPECT_TRUE(buffer.GetSpan().empty());
}

TEST(BinaryBuffer, AppendSpans) {
  BinaryBuffer buffer;
  std::vector<uint8_t> aaa(3, 65u);
  std::vector<uint8_t> bbb(3, 66u);
  buffer.AppendSpan(aaa);
  buffer.AppendSpan(bbb);
  EXPECT_FALSE(buffer.IsEmpty());
  EXPECT_EQ(6u, buffer.GetSize());
  EXPECT_EQ(6u, buffer.GetLength());
  EXPECT_EQ(65u, buffer.GetSpan()[0]);
  EXPECT_EQ(65u, buffer.GetSpan()[1]);
  EXPECT_EQ(65u, buffer.GetSpan()[2]);
  EXPECT_EQ(66u, buffer.GetSpan()[3]);
  EXPECT_EQ(66u, buffer.GetSpan()[4]);
  EXPECT_EQ(66u, buffer.GetSpan()[5]);
}

TEST(BinaryBuffer, AppendBlocks) {
  BinaryBuffer buffer;
  std::vector<uint8_t> aaa(3, 65u);
  std::vector<uint8_t> bbb(3, 66u);
  buffer.AppendBlock(aaa.data(), aaa.size());
  buffer.AppendBlock(bbb.data(), bbb.size());
  EXPECT_EQ(6u, buffer.GetSize());
  EXPECT_EQ(6u, buffer.GetLength());
  EXPECT_EQ(65u, buffer.GetSpan()[0]);
  EXPECT_EQ(65u, buffer.GetSpan()[1]);
  EXPECT_EQ(65u, buffer.GetSpan()[2]);
  EXPECT_EQ(66u, buffer.GetSpan()[3]);
  EXPECT_EQ(66u, buffer.GetSpan()[4]);
  EXPECT_EQ(66u, buffer.GetSpan()[5]);
}

TEST(BinaryBuffer, AppendStrings) {
  BinaryBuffer buffer;
  buffer.AppendString("AA");
  buffer.AppendString("BB");
  EXPECT_EQ(4u, buffer.GetSize());
  EXPECT_EQ(4u, buffer.GetLength());
  EXPECT_EQ(65u, buffer.GetSpan()[0]);
  EXPECT_EQ(65u, buffer.GetSpan()[1]);
  EXPECT_EQ(66u, buffer.GetSpan()[2]);
  EXPECT_EQ(66u, buffer.GetSpan()[3]);
}

TEST(BinaryBuffer, AppendBytes) {
  BinaryBuffer buffer;
  buffer.AppendByte(65u);
  buffer.AppendByte(66u);
  EXPECT_EQ(2u, buffer.GetSize());
  EXPECT_EQ(2u, buffer.GetLength());
  EXPECT_EQ(65u, buffer.GetSpan()[0]);
  EXPECT_EQ(66u, buffer.GetSpan()[1]);
}

}  // namespace fxcrt
