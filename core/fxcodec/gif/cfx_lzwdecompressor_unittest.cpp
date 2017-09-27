// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/gif/cfx_lzwdecompressor.h"

#include "core/fxcrt/fx_memory.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CFX_LZWDecompressor, CreateBadParams) {
  EXPECT_EQ(nullptr, CFX_LZWDecompressor::Create(0x10, 0x2));
  EXPECT_EQ(nullptr, CFX_LZWDecompressor::Create(0x4, 0x0F));
}

TEST(CFX_LZWDecompressor, DecodeBadParams) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;

  auto decoder = CFX_LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decoder);

  uint8_t image_data[10];
  uint32_t image_size = FX_ArraySize(image_data);

  uint8_t output_data[10];
  uint32_t output_size = FX_ArraySize(output_data);

  EXPECT_EQ(CFX_GifDecodeStatus::Error,
            decoder->Decode(nullptr, image_size, output_data, &output_size));
  EXPECT_EQ(CFX_GifDecodeStatus::Error,
            decoder->Decode(image_data, 0, output_data, &output_size));
  EXPECT_EQ(CFX_GifDecodeStatus::Error,
            decoder->Decode(image_data, image_size, nullptr, &output_size));
  EXPECT_EQ(CFX_GifDecodeStatus::Error,
            decoder->Decode(image_data, image_size, output_data, nullptr));

  output_size = 0;
  EXPECT_EQ(CFX_GifDecodeStatus::InsufficientDestSize,
            decoder->Decode(image_data, image_size, output_data, &output_size));
}

TEST(CFX_LZWDecompressor, Decode1x1SingleColour) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;

  auto decoder = CFX_LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decoder);

  uint8_t image_data[] = {0x44, 0x01};
  uint32_t image_size = FX_ArraySize(image_data);

  uint8_t output_data[1];
  uint32_t output_size = FX_ArraySize(output_data);

  EXPECT_EQ(CFX_GifDecodeStatus::Success,
            decoder->Decode(image_data, image_size, output_data, &output_size));
  uint8_t expected_data[] = {0x00};

  EXPECT_EQ(FX_ArraySize(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(expected_data, output_data, sizeof(expected_data)));
}

TEST(CFX_LZWDecompressor, Decode10x10SingleColour) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;

  auto decoder = CFX_LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decoder);

  uint8_t image_data[] = {0x84, 0x8F, 0xA9, 0xCB, 0xED, 0x0F, 0x63, 0x2B};
  uint32_t image_size = FX_ArraySize(image_data);

  uint8_t output_data[100];
  memset(output_data, 0, sizeof(output_data));
  uint32_t output_size = FX_ArraySize(output_data);

  EXPECT_EQ(CFX_GifDecodeStatus::Success,
            decoder->Decode(image_data, image_size, output_data, &output_size));
  uint8_t expected_data[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00};

  EXPECT_EQ(FX_ArraySize(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(expected_data, output_data, sizeof(expected_data)));
}

TEST(CFX_LZWDecompressor, Decode10x10MultipleColour) {
  uint8_t palette_exp = 0x1;
  uint8_t code_exp = 0x2;

  auto decoder = CFX_LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decoder);

  uint8_t image_data[] = {0x8C, 0x2D, 0x99, 0x87, 0x2A, 0x1C, 0xDC, 0x33,
                          0xA0, 0x02, 0x75, 0xEC, 0x95, 0xFA, 0xA8, 0xDE,
                          0x60, 0x8C, 0x04, 0x91, 0x4C, 0x01};
  uint32_t image_size = FX_ArraySize(image_data);

  uint8_t output_data[100];
  memset(output_data, 0, sizeof(output_data));
  uint32_t output_size = FX_ArraySize(output_data);

  EXPECT_EQ(CFX_GifDecodeStatus::Success,
            decoder->Decode(image_data, image_size, output_data, &output_size));
  uint8_t expected_data[] = {
      0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02,
      0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
      0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02,
      0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
      0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01,
      0x01, 0x01, 0x01, 0x01};

  EXPECT_EQ(FX_ArraySize(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(expected_data, output_data, sizeof(expected_data)));
}

TEST(CFX_LZWDecompressor, HandleColourCodeOutOfPalette) {
  uint8_t palette_exp = 0x2;  // Image uses 10 colours, so the palette exp
                              // should be 3, 2^(3+1) = 16 colours.
  uint8_t code_exp = 0x4;

  auto decoder = CFX_LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decoder);

  uint8_t image_data[] = {0x30, 0xC9, 0x49, 0x81, 0xBD, 0x78, 0xE8, 0xCD,
                          0x89, 0xFF, 0x60, 0x20, 0x8E, 0xE4, 0x61, 0x9E,
                          0xA8, 0xA1, 0xAE, 0x2C, 0xE2, 0xBE, 0xB0, 0x20,
                          0xCF, 0x74, 0x61, 0xDF, 0x78, 0x04};
  uint32_t image_size = FX_ArraySize(image_data);

  uint8_t output_data[100];
  memset(output_data, 0, sizeof(output_data));
  uint32_t output_size = FX_ArraySize(output_data);

  EXPECT_EQ(CFX_GifDecodeStatus::Error,
            decoder->Decode(image_data, image_size, output_data, &output_size));
}
