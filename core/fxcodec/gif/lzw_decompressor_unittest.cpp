// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/gif/lzw_decompressor.h"

#include <stdint.h>
#include <string.h>

#include <array>
#include <iterator>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/stl_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

TEST(LZWDecompressor, CreateBadParams) {
  EXPECT_FALSE(LZWDecompressor::Create(0x10, 0x02));
  EXPECT_FALSE(LZWDecompressor::Create(0x04, 0x0F));
  EXPECT_FALSE(LZWDecompressor::Create(0x02, 0x02));
}

TEST(LZWDecompressor, ExtractData) {
  uint8_t palette_exp = 0x1;
  uint8_t code_exp = 0x2;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  // Check that 0 length extract does nothing
  {
    DataVector<uint8_t>* decompressed = decompressor->DecompressedForTest();
    *decompressed = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    *(decompressor->DecompressedNextForTest()) = decompressed->size();
    uint8_t dest_buf[20];
    fxcrt::Fill(dest_buf, 0xff);
    EXPECT_EQ(0u, decompressor->ExtractDataForTest(
                      pdfium::make_span(dest_buf).first(0u)));
    EXPECT_THAT(dest_buf, Each(static_cast<uint8_t>(-1)));
    EXPECT_EQ(10u, *(decompressor->DecompressedNextForTest()));
    for (size_t i = 0; i < *(decompressor->DecompressedNextForTest()); ++i) {
      EXPECT_EQ(i, (*decompressed)[i]);
    }
  }

  // Check that less than decompressed size only gets the expected number
  {
    DataVector<uint8_t>* decompressed = decompressor->DecompressedForTest();
    *decompressed = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    *(decompressor->DecompressedNextForTest()) = decompressed->size();
    std::array<uint8_t, 20> dest_buf;
    fxcrt::Fill(dest_buf, 0xff);
    EXPECT_EQ(5u, decompressor->ExtractDataForTest(
                      pdfium::make_span(dest_buf).first(5u)));
    size_t i = 0;
    for (; i < 5; ++i) {
      EXPECT_EQ(9 - i, dest_buf[i]);
    }
    EXPECT_THAT(pdfium::span(dest_buf).first(5), ElementsAre(9, 8, 7, 6, 5));
    EXPECT_THAT(pdfium::span(dest_buf).subspan(5),
                Each(static_cast<uint8_t>(-1)));
    EXPECT_EQ(5u, *(decompressor->DecompressedNextForTest()));
    for (i = 0; i < *(decompressor->DecompressedNextForTest()); ++i) {
      EXPECT_EQ(i, (*decompressed)[i]);
    }
  }

  // Check that greater than decompressed size depletes the decompressor
  {
    DataVector<uint8_t>* decompressed = decompressor->DecompressedForTest();
    *decompressed = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    *(decompressor->DecompressedNextForTest()) = decompressed->size();
    uint8_t dest_buf[20];
    fxcrt::Fill(dest_buf, 0xff);
    EXPECT_EQ(10u, decompressor->ExtractDataForTest(dest_buf));
    EXPECT_THAT(pdfium::span(dest_buf).first(10),
                ElementsAre(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
    EXPECT_THAT(pdfium::span(dest_buf).subspan(10),
                Each(static_cast<uint8_t>(-1)));
    EXPECT_EQ(0u, *(decompressor->DecompressedNextForTest()));
  }
}

TEST(LZWDecompressor, DecodeBadParams) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  uint8_t image_data[10];
  uint8_t output_data[10];
  uint32_t output_size = std::size(output_data);

  decompressor->SetSource(pdfium::span<uint8_t>());
  EXPECT_EQ(LZWDecompressor::Status::kUnfinished,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));

  decompressor->SetSource(image_data);
  EXPECT_EQ(LZWDecompressor::Status::kError,
            UNSAFE_TODO(decompressor->Decode(nullptr, &output_size)));
  EXPECT_EQ(LZWDecompressor::Status::kError,
            UNSAFE_TODO(decompressor->Decode(output_data, nullptr)));

  output_size = 0;
  EXPECT_EQ(LZWDecompressor::Status::kInsufficientDestSize,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));
}

TEST(LZWDecompressor, Decode1x1SingleColour) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  uint8_t image_data[] = {0x44, 0x01};
  uint8_t expected_data[] = {0x00};
  uint8_t output_data[std::size(expected_data)] = {};
  uint32_t output_size = std::size(output_data);

  decompressor->SetSource(image_data);
  EXPECT_EQ(LZWDecompressor::Status::kSuccess,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));

  EXPECT_EQ(std::size(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(expected_data, output_data, sizeof(expected_data)));
}

TEST(LZWDecompressor, Decode10x10SingleColour) {
  uint8_t palette_exp = 0x0;
  uint8_t code_exp = 0x2;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  static constexpr uint8_t kImageData[] = {0x84, 0x8F, 0xA9, 0xCB,
                                           0xED, 0x0F, 0x63, 0x2B};
  static constexpr uint8_t kExpectedData[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00};
  uint8_t output_data[std::size(kExpectedData)] = {};
  uint32_t output_size = std::size(output_data);

  decompressor->SetSource(kImageData);
  EXPECT_EQ(LZWDecompressor::Status::kSuccess,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));

  EXPECT_EQ(std::size(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(kExpectedData, output_data, sizeof(kExpectedData)));
}

TEST(LZWDecompressor, Decode10x10MultipleColour) {
  uint8_t palette_exp = 0x1;
  uint8_t code_exp = 0x2;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  static constexpr uint8_t kImageData[] = {
      0x8C, 0x2D, 0x99, 0x87, 0x2A, 0x1C, 0xDC, 0x33, 0xA0, 0x02, 0x75,
      0xEC, 0x95, 0xFA, 0xA8, 0xDE, 0x60, 0x8C, 0x04, 0x91, 0x4C, 0x01};
  static constexpr uint8_t kExpectedData[] = {
      0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02,
      0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
      0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02,
      0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
      0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01,
      0x01, 0x01, 0x01, 0x01};
  uint8_t output_data[std::size(kExpectedData)] = {};
  uint32_t output_size = std::size(output_data);

  decompressor->SetSource(kImageData);
  EXPECT_EQ(LZWDecompressor::Status::kSuccess,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));

  EXPECT_EQ(std::size(output_data), output_size);
  EXPECT_TRUE(0 == memcmp(kExpectedData, output_data, sizeof(kExpectedData)));
}

TEST(LZWDecompressor, MultipleDecodes) {
  auto decompressor = LZWDecompressor::Create(/*color_exp=*/0, /*code_exp=*/2);
  ASSERT_NE(nullptr, decompressor);

  static constexpr uint8_t kImageData[] = {0x84, 0x6f, 0x05};
  decompressor->SetSource(kImageData);

  static constexpr uint8_t kExpectedScanline[] = {0x00, 0x00, 0x00, 0x00};
  uint8_t output_data[std::size(kExpectedScanline)];
  fxcrt::Fill(output_data, 0xff);
  uint32_t output_size = std::size(output_data);
  EXPECT_EQ(LZWDecompressor::Status::kInsufficientDestSize,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));
  EXPECT_EQ(std::size(kExpectedScanline), output_size);
  EXPECT_THAT(output_data, ElementsAreArray(kExpectedScanline));

  fxcrt::Fill(output_data, 0xff);
  output_size = std::size(output_data);
  EXPECT_EQ(LZWDecompressor::Status::kSuccess,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));
  EXPECT_EQ(std::size(kExpectedScanline), output_size);
  EXPECT_THAT(output_data, ElementsAreArray(kExpectedScanline));
}

TEST(LZWDecompressor, HandleColourCodeOutOfPalette) {
  uint8_t palette_exp = 0x2;  // Image uses 10 colours, so the palette exp
                              // should be 3, 2^(3+1) = 16 colours.
  uint8_t code_exp = 0x4;
  auto decompressor = LZWDecompressor::Create(palette_exp, code_exp);
  ASSERT_NE(nullptr, decompressor);

  static constexpr uint8_t kImageData[] = {
      0x30, 0xC9, 0x49, 0x81, 0xBD, 0x78, 0xE8, 0xCD, 0x89, 0xFF,
      0x60, 0x20, 0x8E, 0xE4, 0x61, 0x9E, 0xA8, 0xA1, 0xAE, 0x2C,
      0xE2, 0xBE, 0xB0, 0x20, 0xCF, 0x74, 0x61, 0xDF, 0x78, 0x04};

  uint8_t output_data[100] = {};  // The uncompressed data is for a 10x10 image
  uint32_t output_size = std::size(output_data);

  decompressor->SetSource(kImageData);
  EXPECT_EQ(LZWDecompressor::Status::kError,
            UNSAFE_TODO(decompressor->Decode(output_data, &output_size)));
}
