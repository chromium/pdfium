// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/progressive_decoder.h"

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <numeric>
#include <tuple>
#include <utility>

#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcodec/jpeg/jpeg_progressive_decoder.h"
#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "core/fxcrt/cfx_read_only_vector_stream.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#ifdef PDF_ENABLE_XFA_BMP
#include "core/fxcodec/bmp/bmp_decoder.h"
#include "core/fxcodec/bmp/bmp_progressive_decoder.h"
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
#include "core/fxcodec/gif/gif_decoder.h"
#include "core/fxcodec/gif/gif_progressive_decoder.h"
#endif  // PDF_ENABLE_XFA_GIF

namespace fxcodec {

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

template <size_t Size>
constexpr std::array<uint8_t, Size> IotaArray(uint8_t start) {
  std::array<uint8_t, Size> result;
  std::iota(result.begin(), result.end(), start);
  return result;
}

FXCODEC_STATUS DecodeToBitmap(ProgressiveDecoder& decoder,
                              RetainPtr<CFX_DIBitmap> bitmap) {
  FXCODEC_STATUS status = decoder.StartDecode(std::move(bitmap));
  while (status == FXCODEC_STATUS::kDecodeToBeContinued) {
    status = decoder.ContinueDecode();
  }
  return status;
}

class ProgressiveDecoderTest : public testing::Test {
  void SetUp() override {
#ifdef PDF_ENABLE_XFA_BMP
    BmpProgressiveDecoder::InitializeGlobals();
#endif
#ifdef PDF_ENABLE_XFA_GIF
    GifProgressiveDecoder::InitializeGlobals();
#endif
    JpegProgressiveDecoder::InitializeGlobals();
  }
  void TearDown() override {
    JpegProgressiveDecoder::DestroyGlobals();
#ifdef PDF_ENABLE_XFA_GIF
    GifProgressiveDecoder::DestroyGlobals();
#endif
#ifdef PDF_ENABLE_XFA_BMP
    BmpProgressiveDecoder::DestroyGlobals();
#endif
  }
};

}  // namespace

#ifdef PDF_ENABLE_XFA_BMP
TEST_F(ProgressiveDecoderTest, Indexed8Bmp) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a,
      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x04, 0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b,
      0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xc0,
      0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0x00));
}

TEST_F(ProgressiveDecoderTest, Indexed8BmpWithInvalidIndex) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a,
      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x04, 0x00, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b,
      0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xc0,
      0x80, 0x40, 0x00, 0x01, 0x00, 0x00, 0x00};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kError, status);
}

TEST_F(ProgressiveDecoderTest, Direct24Bmp) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
      0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x40, 0x00};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0x00));
}

TEST_F(ProgressiveDecoderTest, Direct32Bmp) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
      0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x40, 0xff};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgrx, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0x00));
}

TEST_F(ProgressiveDecoderTest, BmpWithDataOffsetBeforeEndOfHeader) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
      0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x40, 0x00};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0x00));
}

TEST_F(ProgressiveDecoderTest, BmpWithDataOffsetAfterEndOfHeader) {
  static constexpr uint8_t kInput[] = {
      0x42, 0x4d, 0x3b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
      0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x40, 0x00};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0x00));
}

TEST_F(ProgressiveDecoderTest, LargeBmp) {
  // Construct a 24-bit BMP larger than `kBlockSize` (4096 bytes).
  static constexpr uint8_t kWidth = 37;
  static constexpr uint8_t kHeight = 38;
  static constexpr size_t kScanlineSize = kWidth * 3 + 1;
  DataVector<uint8_t> input = {
      0x42,    0x4d, 0xd6, 0x10, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x36,
      0x00,    0x00, 0x00, 0x28, 0x00, 0x00, 0x00, kWidth, 0x00, 0x00, 0x00,
      kHeight, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00,   0x00, 0x00, 0x00,
      0x00,    0xa0, 0x10, 0x00, 0x00, 0x13, 0x0b, 0x00,   0x00, 0x13, 0x0b,
      0x00,    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00};
  input.resize(54 + kScanlineSize * kHeight);
  std::iota(input.begin() + 54, input.end(), 0);
  ASSERT_EQ(4310u, input.size());

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlyVectorStream>(std::move(input));
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_BMP, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(kWidth, decoder.GetWidth());
  ASSERT_EQ(kHeight, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgr, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);

  for (size_t row = 0; row < kHeight; ++row) {
    // BMP encodes rows from bottom to top by default.
    pdfium::span<const uint8_t> scanline =
        bitmap->GetScanline(kHeight - row - 1);

    EXPECT_THAT(
        scanline.first(kScanlineSize - 1),
        ElementsAreArray(IotaArray<kScanlineSize - 1>(row * kScanlineSize)));

    // Last byte is padding to a 32-bit boundary.
    EXPECT_EQ(0, scanline[kScanlineSize - 1]);
  }
}
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
TEST_F(ProgressiveDecoderTest, Gif87a) {
  static constexpr uint8_t kInput[] = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x01, 0x00, 0x01, 0x00, 0x80, 0x01,
      0x00, 0x40, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x3b};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_GIF, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0xff));
}

TEST_F(ProgressiveDecoderTest, Gif89a) {
  static constexpr uint8_t kInput[] = {
      0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00, 0x80,
      0x01, 0x00, 0x40, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x21, 0xf9, 0x04,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x01,
      0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x3b};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_GIF, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0), ElementsAre(0xc0, 0x80, 0x40, 0xff));
}

TEST_F(ProgressiveDecoderTest, GifInsufficientCodeSize) {
  // This GIF causes `LZWDecompressor::Create()` to fail because the minimum
  // code size is too small for the palette.
  static constexpr uint8_t kInput[] = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x01, 0x00, 0x01, 0x00, 0x82,
      0x01, 0x00, 0x40, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x81, 0x81, 0x81,
      0x82, 0x82, 0x82, 0x83, 0x83, 0x83, 0x84, 0x84, 0x84, 0x85, 0x85,
      0x85, 0x86, 0x86, 0x86, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x01, 0x00, 0x00, 0x02, 0x2,  0x44, 0x01, 0x00, 0x3b};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_GIF, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(1, decoder.GetWidth());
  ASSERT_EQ(1, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kError, status);
}

TEST_F(ProgressiveDecoderTest, GifDecodeAcrossScanlines) {
  // This GIF contains an LZW code unit split across 2 scanlines. The decoder
  // must continue decoding the second scanline using the residual data.
  static constexpr uint8_t kInput[] = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x04, 0x00, 0x02, 0x00, 0x80, 0x01,
      0x00, 0x40, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x2c, 0x00, 0x00, 0x00, 0x00,
      0x04, 0x00, 0x02, 0x00, 0x00, 0x02, 0x03, 0x84, 0x6f, 0x05, 0x00, 0x3b};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_GIF, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(4, decoder.GetWidth());
  ASSERT_EQ(2, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0),
              ElementsAre(0xc0, 0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff, 0xc0,
                          0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff));
  EXPECT_THAT(bitmap->GetScanline(1),
              ElementsAre(0xc0, 0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff, 0xc0,
                          0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff));
}

TEST_F(ProgressiveDecoderTest, GifDecodeAcrossSubblocks) {
  // This GIF contains a scanline split across 2 data sub-blocks. The decoder
  // must continue decoding in the second sub-block.
  static constexpr uint8_t kInput[] = {
      0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x04, 0x00, 0x02, 0x00,
      0x80, 0x01, 0x00, 0x40, 0x80, 0xc0, 0x80, 0x80, 0x80, 0x2c,
      0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x02,
      0x02, 0x84, 0x6f, 0x01, 0x05, 0x00, 0x3b};

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kInput);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_GIF, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(4, decoder.GetWidth());
  ASSERT_EQ(2, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0),
              ElementsAre(0xc0, 0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff, 0xc0,
                          0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff));
  EXPECT_THAT(bitmap->GetScanline(1),
              ElementsAre(0xc0, 0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff, 0xc0,
                          0x80, 0x40, 0xff, 0xc0, 0x80, 0x40, 0xff));
}
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
// `kGreenPng` has been taken from Chromium's
// `//third_party/blink/renderer/platform/testing/data/green.png`.
constexpr std::array<const uint8_t, 87> kGreenPng = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00,
    0x00, 0x32, 0x01, 0x03, 0x00, 0x00, 0x00, 0x90, 0xfb, 0xec, 0xfd,
    0x00, 0x00, 0x00, 0x03, 0x50, 0x4c, 0x54, 0x45, 0x00, 0xff, 0x00,
    0x34, 0x5e, 0xc0, 0xa8, 0x00, 0x00, 0x00, 0x0f, 0x49, 0x44, 0x41,
    0x54, 0x28, 0x15, 0x63, 0x60, 0x18, 0x05, 0xa3, 0x60, 0x68, 0x02,
    0x00, 0x02, 0xbc, 0x00, 0x01, 0x1b, 0xdd, 0xe3, 0x90, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};

TEST_F(ProgressiveDecoderTest, PngSmokeTest) {
  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(kGreenPng);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_PNG, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(100, decoder.GetWidth());
  ASSERT_EQ(50, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kDecodeFinished, status);
  EXPECT_THAT(bitmap->GetScanline(0).first(4u),
              ElementsAre(0x00, 0xFF, 0x00, 0xFF));
}

TEST_F(ProgressiveDecoderTest, TruncatedPng) {
  // Truncate the input in the middle of the IDAT chunk's payload.
  const auto input = pdfium::span(kGreenPng).first(64u);

  ProgressiveDecoder decoder;

  auto source = pdfium::MakeRetain<CFX_ReadOnlySpanStream>(input);
  CFX_DIBAttribute attr;
  FXCODEC_STATUS status =
      decoder.LoadImageInfo(std::move(source), FXCODEC_IMAGE_PNG, &attr, true);
  ASSERT_EQ(FXCODEC_STATUS::kFrameReady, status);

  ASSERT_EQ(100, decoder.GetWidth());
  ASSERT_EQ(50, decoder.GetHeight());
  ASSERT_EQ(FXDIB_Format::kBgra, decoder.GetBitmapFormat());

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  ASSERT_TRUE(bitmap->Create(decoder.GetWidth(), decoder.GetHeight(),
                             decoder.GetBitmapFormat()));

  size_t frames;
  std::tie(status, frames) = decoder.GetFrames();
  ASSERT_EQ(FXCODEC_STATUS::kDecodeReady, status);
  ASSERT_EQ(1u, frames);

  status = DecodeToBitmap(decoder, bitmap);
  EXPECT_EQ(FXCODEC_STATUS::kError, status);
}
#endif

}  // namespace fxcodec
