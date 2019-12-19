// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/byteorder.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace {

// Original code to use as a reference implementation.

#define FXWORD_GET_LSBFIRST(p)                                \
  (static_cast<uint16_t>((static_cast<uint16_t>(p[1]) << 8) | \
                         (static_cast<uint16_t>(p[0]))))
#define FXWORD_GET_MSBFIRST(p)                                \
  (static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | \
                         (static_cast<uint16_t>(p[1]))))
#define FXDWORD_GET_LSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[3]) << 24) | (static_cast<uint32_t>(p[2]) << 16) | \
   (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[0])))
#define FXDWORD_GET_MSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | \
   (static_cast<uint32_t>(p[2]) << 8) | (static_cast<uint32_t>(p[3])))

constexpr uint32_t kTestValues32[] = {
    0x0,      0x1,        0x2,        0x3,        0x4,       0xfe,
    0xff,     0x100,      0x101,      0xffff,     0x10000,   0x123456,
    0x345167, 0x2f3e4a5b, 0xff000000, 0xfffffffe, 0xffffffff};

}  // namespace

namespace fxcrt {

TEST(ByteOrder, ByteSwapToLE16) {
  // Since there are so few values, test them all.
  for (uint32_t v = 0; v < 0x10000; ++v) {
    const uint16_t v16 = v;
    uint16_t expected =
        FXWORD_GET_LSBFIRST(reinterpret_cast<const uint8_t*>(&v16));
    EXPECT_EQ(expected, ByteSwapToLE16(v16)) << v;
  }
}

TEST(ByteOrder, ByteSwapToLE32) {
  for (uint32_t v : kTestValues32) {
    uint32_t expected =
        FXDWORD_GET_LSBFIRST(reinterpret_cast<const uint8_t*>(&v));
    EXPECT_EQ(expected, ByteSwapToLE32(v)) << v;
  }
}

TEST(ByteOrder, ByteSwapToBE16) {
  // Since there are so few values, test them all.
  for (uint32_t v = 0; v < 0x10000; ++v) {
    const uint16_t v16 = v;
    uint16_t expected =
        FXWORD_GET_MSBFIRST(reinterpret_cast<const uint8_t*>(&v16));
    EXPECT_EQ(expected, ByteSwapToBE16(v16)) << v;
  }
}

TEST(ByteOrder, ByteSwapToBE32) {
  for (uint32_t v : kTestValues32) {
    uint32_t expected =
        FXDWORD_GET_MSBFIRST(reinterpret_cast<const uint8_t*>(&v));
    EXPECT_EQ(expected, ByteSwapToBE32(v)) << v;
  }
}

}  // namespace fxcrt
