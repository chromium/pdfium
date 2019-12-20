// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "public/fpdf_thumbnail.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/utils/hash.h"

class FPDFThumbnailEmbedderTest : public EmbedderTest {};

TEST_F(FPDFThumbnailEmbedderTest, GetDecodedThumbnailDataFromPageWithFilters) {
  ASSERT_TRUE(OpenDocument("simple_thumbnail.pdf"));

  {
    const char kHashedDecodedData[] = "7902d0be831c9024960f4ebd5d7df1f7";
    const unsigned long kExpectedSize = 1138u;

    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);

    unsigned long length_bytes =
        FPDFPage_GetDecodedThumbnailData(page, nullptr, 0);
    ASSERT_EQ(kExpectedSize, length_bytes);
    std::vector<uint8_t> thumb_buf(length_bytes);

    EXPECT_EQ(kExpectedSize, FPDFPage_GetDecodedThumbnailData(
                                 page, thumb_buf.data(), length_bytes));
    EXPECT_EQ(kHashedDecodedData,
              GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

    UnloadPage(page);
  }

  {
    const char kHashedDecodedData[] = "e81123a573378ba1ea80461d25cc41f6";
    const unsigned long kExpectedSize = 1110u;

    FPDF_PAGE page = LoadPage(1);
    ASSERT_TRUE(page);

    unsigned long length_bytes =
        FPDFPage_GetDecodedThumbnailData(page, nullptr, 0);
    ASSERT_EQ(kExpectedSize, length_bytes);
    std::vector<uint8_t> thumb_buf(length_bytes);

    EXPECT_EQ(kExpectedSize, FPDFPage_GetDecodedThumbnailData(
                                 page, thumb_buf.data(), length_bytes));
    EXPECT_EQ(kHashedDecodedData,
              GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

    UnloadPage(page);
  }
}

TEST_F(FPDFThumbnailEmbedderTest,
       GetDecodedThumbnailDataFromPageWithNoFilters) {
  ASSERT_TRUE(OpenDocument("thumbnail_with_no_filters.pdf"));

  const char kHashedDecodedData[] = "b5696e586382b3373741f8a1d651cab0";
  const unsigned long kExpectedSize = 301u;

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  unsigned long length_bytes =
      FPDFPage_GetDecodedThumbnailData(page, nullptr, 0);
  ASSERT_EQ(kExpectedSize, length_bytes);
  std::vector<uint8_t> thumb_buf(length_bytes);

  EXPECT_EQ(kExpectedSize, FPDFPage_GetDecodedThumbnailData(
                               page, thumb_buf.data(), length_bytes));
  EXPECT_EQ(kHashedDecodedData,
            GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest,
       GetDecodedThumbnailDataFromPageWithNoThumbnails) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(0u, FPDFPage_GetDecodedThumbnailData(page, nullptr, 0));

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest, GetDecodedThumbnailDataFromPageNullPage) {
  EXPECT_EQ(0u, FPDFPage_GetDecodedThumbnailData(nullptr, nullptr, 0));
}

TEST_F(FPDFThumbnailEmbedderTest, GetRawThumbnailDataFromPageWithFilters) {
  ASSERT_TRUE(OpenDocument("simple_thumbnail.pdf"));

  {
    const char kHashedRawData[] = "f6a8e8db01cccd52abb91ea433a17373";
    const unsigned long kExpectedSize = 1851u;

    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);

    unsigned long length_bytes = FPDFPage_GetRawThumbnailData(page, nullptr, 0);
    ASSERT_EQ(kExpectedSize, length_bytes);
    std::vector<uint8_t> thumb_buf(length_bytes);

    EXPECT_EQ(kExpectedSize, FPDFPage_GetRawThumbnailData(
                                 page, thumb_buf.data(), length_bytes));
    EXPECT_EQ(kHashedRawData,
              GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

    UnloadPage(page);
  }

  {
    const char kHashedRawData[] = "c7558a461d5ecfb1d4757218b473afc0";
    const unsigned long kExpectedSize = 1792u;

    FPDF_PAGE page = LoadPage(1);
    ASSERT_TRUE(page);

    unsigned long length_bytes = FPDFPage_GetRawThumbnailData(page, nullptr, 0);
    ASSERT_EQ(kExpectedSize, length_bytes);
    std::vector<uint8_t> thumb_buf(length_bytes);

    EXPECT_EQ(kExpectedSize, FPDFPage_GetRawThumbnailData(
                                 page, thumb_buf.data(), length_bytes));
    EXPECT_EQ(kHashedRawData,
              GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

    UnloadPage(page);
  }
}

TEST_F(FPDFThumbnailEmbedderTest, GetRawThumbnailDataFromPageWithNoFilters) {
  ASSERT_TRUE(OpenDocument("thumbnail_with_no_filters.pdf"));

  const char kHashedRawData[] = "b5696e586382b3373741f8a1d651cab0";
  const unsigned long kExpectedSize = 301u;

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  unsigned long length_bytes = FPDFPage_GetRawThumbnailData(page, nullptr, 0);
  ASSERT_EQ(kExpectedSize, length_bytes);
  std::vector<uint8_t> thumb_buf(length_bytes);

  EXPECT_EQ(kExpectedSize,
            FPDFPage_GetRawThumbnailData(page, thumb_buf.data(), length_bytes));
  EXPECT_EQ(kHashedRawData, GenerateMD5Base16(thumb_buf.data(), kExpectedSize));

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest, GetRawThumbnailDataFromPageWithNoThumbnails) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_EQ(0u, FPDFPage_GetRawThumbnailData(page, nullptr, 0));

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest, GetRawThumbnailDataFromPageNullPage) {
  EXPECT_EQ(0u, FPDFPage_GetRawThumbnailData(nullptr, nullptr, 0));
}

TEST_F(FPDFThumbnailEmbedderTest, GetThumbnailAsBitmapFromPage) {
  ASSERT_TRUE(OpenDocument("simple_thumbnail.pdf"));

  {
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);

    ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));

    EXPECT_EQ(50, FPDFBitmap_GetWidth(thumb_bitmap.get()));
    EXPECT_EQ(50, FPDFBitmap_GetHeight(thumb_bitmap.get()));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(thumb_bitmap.get()));
    CompareBitmap(thumb_bitmap.get(), 50, 50,
                  "52b75451e396f55e95d1cb68e6018226");

    UnloadPage(page);
  }

  {
    FPDF_PAGE page = LoadPage(1);
    ASSERT_TRUE(page);

    ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));

    EXPECT_EQ(50, FPDFBitmap_GetWidth(thumb_bitmap.get()));
    EXPECT_EQ(50, FPDFBitmap_GetHeight(thumb_bitmap.get()));
    EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(thumb_bitmap.get()));
    CompareBitmap(thumb_bitmap.get(), 50, 50,
                  "1f448be08c6e6043ccd0bad8ecc2a351");

    UnloadPage(page);
  }
}

TEST_F(FPDFThumbnailEmbedderTest,
       GetThumbnailAsBitmapFromPageWithoutThumbnail) {
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));
  ASSERT_EQ(nullptr, thumb_bitmap.get());

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest,
       GetThumbnailAsBitmapFromThumbnailWithEmptyStream) {
  ASSERT_TRUE(OpenDocument("thumbnail_with_empty_stream.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));
  ASSERT_EQ(nullptr, thumb_bitmap.get());

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest,
       GetThumbnailAsBitmapFromThumbnailWithNoFilters) {
  ASSERT_TRUE(OpenDocument("thumbnail_with_no_filters.pdf"));

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));

  EXPECT_EQ(10, FPDFBitmap_GetWidth(thumb_bitmap.get()));
  EXPECT_EQ(10, FPDFBitmap_GetHeight(thumb_bitmap.get()));
  EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(thumb_bitmap.get()));
  CompareBitmap(thumb_bitmap.get(), 10, 10, "fe02583f9e6d094042a942ff686e9936");

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest, GetThumbnailDoesNotAlterPage) {
  ASSERT_TRUE(OpenDocument("simple_thumbnail.pdf"));

  const char kHashedRawData[] = "f6a8e8db01cccd52abb91ea433a17373";
  const unsigned long kExpectedRawSize = 1851u;

  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  // Get the raw data
  unsigned long raw_size = FPDFPage_GetRawThumbnailData(page, nullptr, 0);
  ASSERT_EQ(kExpectedRawSize, raw_size);
  std::vector<uint8_t> raw_thumb_buf(raw_size);

  EXPECT_EQ(kExpectedRawSize,
            FPDFPage_GetRawThumbnailData(page, raw_thumb_buf.data(), raw_size));
  EXPECT_EQ(kHashedRawData,
            GenerateMD5Base16(raw_thumb_buf.data(), kExpectedRawSize));

  // Get the thumbnail
  ScopedFPDFBitmap thumb_bitmap(FPDFPage_GetThumbnailAsBitmap(page));

  EXPECT_EQ(50, FPDFBitmap_GetWidth(thumb_bitmap.get()));
  EXPECT_EQ(50, FPDFBitmap_GetHeight(thumb_bitmap.get()));
  EXPECT_EQ(FPDFBitmap_BGR, FPDFBitmap_GetFormat(thumb_bitmap.get()));
  CompareBitmap(thumb_bitmap.get(), 50, 50, "52b75451e396f55e95d1cb68e6018226");

  // Get the raw data again
  unsigned long new_raw_size = FPDFPage_GetRawThumbnailData(page, nullptr, 0);
  ASSERT_EQ(kExpectedRawSize, new_raw_size);
  std::vector<uint8_t> new_raw_thumb_buf(new_raw_size);

  EXPECT_EQ(kExpectedRawSize,
            FPDFPage_GetRawThumbnailData(page, new_raw_thumb_buf.data(),
                                         new_raw_size));
  EXPECT_EQ(kHashedRawData,
            GenerateMD5Base16(new_raw_thumb_buf.data(), kExpectedRawSize));

  UnloadPage(page);
}

TEST_F(FPDFThumbnailEmbedderTest, GetThumbnailAsBitmapFromPageNullPage) {
  EXPECT_EQ(nullptr, FPDFPage_GetThumbnailAsBitmap(nullptr));
}
