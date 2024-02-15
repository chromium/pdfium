// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/byteorder.h"

#include "core/fxcrt/fx_system.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

constexpr uint32_t kTestValues32[] = {
    0x0,      0x1,        0x2,        0x3,        0x4,       0xfe,
    0xff,     0x100,      0x101,      0xffff,     0x10000,   0x123456,
    0x345167, 0x2f3e4a5b, 0xff000000, 0xfffffffe, 0xffffffff};

}  // namespace

namespace fxcrt {

TEST(ByteOrder, FromLE16) {
  // Since there are so few values, test them all.
  for (uint32_t v = 0; v < 0x10000; ++v) {
    const uint16_t v16 = v;
    uint16_t expected = GetUInt16LSBFirst(pdfium::byte_span_from_ref(v16));
    EXPECT_EQ(expected, FromLE16(v16)) << v;
  }
}

TEST(ByteOrder, FromLE32) {
  for (uint32_t v : kTestValues32) {
    uint32_t expected = GetUInt32LSBFirst(pdfium::byte_span_from_ref(v));
    EXPECT_EQ(expected, FromLE32(v)) << v;
  }
}

TEST(ByteOrder, FromBE16) {
  // Since there are so few values, test them all.
  for (uint32_t v = 0; v < 0x10000; ++v) {
    const uint16_t v16 = v;
    uint16_t expected = GetUInt16MSBFirst(pdfium::byte_span_from_ref(v16));
    EXPECT_EQ(expected, FromBE16(v16)) << v;
  }
}

TEST(ByteOrder, FromBE32) {
  for (uint32_t v : kTestValues32) {
    uint32_t expected = GetUInt32MSBFirst(pdfium::byte_span_from_ref(v));
    EXPECT_EQ(expected, FromBE32(v)) << v;
  }
}

TEST(ByteOrder, GetUInt16LSBFirst) {
  const uint8_t kBuf[2] = {0xff, 0xfe};
  EXPECT_EQ(0xfeff, GetUInt16LSBFirst(kBuf));
}

TEST(ByteOrder, GetUInt16MSBFirst) {
  const uint8_t kBuf[2] = {0xff, 0xfe};
  EXPECT_EQ(0xfffe, GetUInt16MSBFirst(kBuf));
}

TEST(ByteOrder, GetUInt32LSBFirst) {
  const uint8_t kBuf[4] = {0xff, 0xfe, 0xfd, 0xfc};
  EXPECT_EQ(0xfcfdfeff, GetUInt32LSBFirst(kBuf));
}

TEST(ByteOrder, GetUInt32MSBFirst) {
  const uint8_t kBuf[4] = {0xff, 0xfe, 0xfd, 0xfc};
  EXPECT_EQ(0xfffefdfc, GetUInt32MSBFirst(kBuf));
}

TEST(ByteOrder, PutUInt16LSBFirst) {
  uint8_t buf[2];
  PutUInt16LSBFirst(0xfffe, buf);
  EXPECT_THAT(buf, testing::ElementsAre(0xfe, 0xff));
}

TEST(ByteOrder, PutUInt16MSBFirst) {
  uint8_t buf[2];
  PutUInt16MSBFirst(0xfffe, buf);
  EXPECT_THAT(buf, testing::ElementsAre(0xff, 0xfe));
}

TEST(ByteOrder, PutUInt32LSBFirst) {
  uint8_t buf[4];
  PutUInt32LSBFirst(0xfffefdfc, buf);
  EXPECT_THAT(buf, testing::ElementsAre(0xfc, 0xfd, 0xfe, 0xff));
}

TEST(ByteOrder, PutUInt32MSBFirst) {
  uint8_t buf[4];
  PutUInt32MSBFirst(0xfffefdfc, buf);
  EXPECT_THAT(buf, testing::ElementsAre(0xff, 0xfe, 0xfd, 0xfc));
}

}  // namespace fxcrt
