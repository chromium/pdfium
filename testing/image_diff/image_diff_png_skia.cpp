// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/image_diff/image_diff_png.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/checked_math.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkStream.h"

#ifdef PDF_ENABLE_RUST_PNG
#include "third_party/skia/include/codec/SkPngRustDecoder.h"
#include "third_party/skia/include/encode/SkPngRustEncoder.h"
#else
#include "third_party/skia/include/codec/SkPngDecoder.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"
#endif

namespace image_diff_png {

namespace {

std::vector<uint8_t> EncodePNG(pdfium::span<const uint8_t> input,
                               SkColorType color,
                               SkAlphaType alpha,
                               int width,
                               int height,
                               size_t row_byte_width) {
  SkImageInfo info =
      SkImageInfo::Make(width, height, color, alpha, SkColorSpace::MakeSRGB());
  CHECK_NE(info.minRowBytes(), 0);  // 0 means conversion problems.
  CHECK_LE(info.minRowBytes(), row_byte_width);
  CHECK_NE(info.computeMinByteSize(), 0);  // 0 means conversion problems.
  CHECK_LE(info.computeMinByteSize(), input.size());
  SkPixmap pixmap(info, input.data(), row_byte_width);

  SkDynamicMemoryWStream output;
#ifdef PDF_ENABLE_RUST_PNG
  bool success = SkPngRustEncoder::Encode(&output, pixmap, {});
#else
  bool success = SkPngEncoder::Encode(&output, pixmap, {});
#endif
  if (!success) {
    return {};
  }
  return output.detachAsVector();
}

}  // namespace

std::vector<uint8_t> DecodePNG(pdfium::span<const uint8_t> input,
                               bool reverse_byte_order,
                               int* width,
                               int* height) {
  CHECK(width);
  CHECK(height);

  auto stream = std::make_unique<SkMemoryStream>(input.data(), input.size(),
                                                 /*copyData=*/false);
#ifdef PDF_ENABLE_RUST_PNG
  std::unique_ptr<SkCodec> codec =
      SkPngRustDecoder::Decode(std::move(stream), nullptr);
#else
  std::unique_ptr<SkCodec> codec =
      SkPngDecoder::Decode(std::move(stream), nullptr);
#endif
  if (!codec) {
    return {};
  }

  SkColorType format =
      reverse_byte_order ? kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
  SkImageInfo info = codec->getInfo();
  info = info.makeColorType(format);
  info = info.makeColorSpace(SkColorSpace::MakeSRGB());

  std::vector<uint8_t> output;
  output.resize(info.computeMinByteSize());

  SkCodec::Result result =
      codec->getPixels(info, output.data(), info.minRowBytes());
  if (result != SkCodec::kSuccess) {
    return {};
  }

  *width = info.width();
  *height = info.height();
  return output;
}

std::vector<uint8_t> EncodeBGRPNG(pdfium::span<const uint8_t> bgr_input,
                                  int width,
                                  int height,
                                  int row_byte_width) {
  // Check inputs.  Expected values are manually calculated (instead of using
  // `SkImageInfo`'s `computeMinByteSize` and/or `minRowBytes`), because
  // `SkColorType` doesn't cover a format with 3 bytes per pixel (bpp) - e.g.
  // `kRGB_565_SkColorType` is 2 bpp and `kRGB_888x_SkColorType` is 4 bpp.
  size_t row_byte_width_as_size_t =
      pdfium::checked_cast<size_t>(row_byte_width);
  FX_SAFE_SIZE_T expected_minimum_row_byte_width = 3;
  expected_minimum_row_byte_width *= width;
  CHECK_LE(expected_minimum_row_byte_width.ValueOrDie(),
           row_byte_width_as_size_t);

  FX_SAFE_SIZE_T expected_minimum_input_size = row_byte_width_as_size_t;
  expected_minimum_input_size *= height;
  CHECK_LE(expected_minimum_input_size.ValueOrDie(), bgr_input.size());

  // Convert `bgr_input` into `intermediate_bgra_buf` (because Skia doesn't
  // allow encoding BGR pixels - see the comment at the top of the function
  // that talks about limitations of `SkColorType`).
  SkImageInfo intermediate_bgra_info =
      SkImageInfo::Make(width, height, kBGRA_8888_SkColorType,
                        kOpaque_SkAlphaType, SkColorSpace::MakeSRGB());
  size_t intermediate_bgra_row_byte_width =
      intermediate_bgra_info.minRowBytes();
  CHECK_NE(0,
           intermediate_bgra_row_byte_width);  // 0 means conversion problems.
  std::vector<uint8_t> intermediate_bgra_buf;
  intermediate_bgra_buf.resize(intermediate_bgra_info.computeMinByteSize());
  CHECK_NE(0, intermediate_bgra_buf.size());  // 0 means conversion problems.
  {
    pdfium::span<const uint8_t> src = bgr_input;
    pdfium::span<uint8_t> dst = intermediate_bgra_buf;
    size_t height_as_size_t = pdfium::checked_cast<size_t>(height);
    size_t width_as_size_t = pdfium::checked_cast<size_t>(width);
    for (size_t y = 0; y < height_as_size_t; y++) {
      for (size_t x = 0; x < width_as_size_t; x++) {
        // If `computeMinByteSize` didn't report an error (`0`), then integer
        // overflow won't happen in the `x * N` expressions below.
        pdfium::span<uint8_t> dst_pixel = dst.subspan(x * 4u).first(4u);
        pdfium::span<const uint8_t> src_pixel = src.subspan(x * 3u).first(3u);

        fxcrt::spancpy(dst_pixel.first(3u), src_pixel);  // Copy BGR channels.
        dst_pixel[3] = 0xFF;  // Set alpha channel to "opaque".
      }
      src = src.subspan(row_byte_width_as_size_t);
      dst = dst.subspan(intermediate_bgra_row_byte_width);
    }
  }

  return EncodePNG(intermediate_bgra_buf, kBGRA_8888_SkColorType,
                   kOpaque_SkAlphaType, width, height,
                   intermediate_bgra_row_byte_width);
}

std::vector<uint8_t> EncodeRGBAPNG(pdfium::span<const uint8_t> input,
                                   int width,
                                   int height,
                                   int row_byte_width) {
  return EncodePNG(input, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, width,
                   height, pdfium::checked_cast<size_t>(row_byte_width));
}

std::vector<uint8_t> EncodeBGRAPNG(pdfium::span<const uint8_t> input,
                                   int width,
                                   int height,
                                   int row_byte_width,
                                   bool discard_transparency) {
  return EncodePNG(input, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, width,
                   height, pdfium::checked_cast<size_t>(row_byte_width));
}

std::vector<uint8_t> EncodeGrayPNG(pdfium::span<const uint8_t> input,
                                   int width,
                                   int height,
                                   int row_byte_width) {
  return EncodePNG(input, kGray_8_SkColorType, kOpaque_SkAlphaType, width,
                   height, pdfium::checked_cast<size_t>(row_byte_width));
}

}  // namespace image_diff_png
