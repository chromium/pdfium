// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibbase.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace {

// Releases `CFX_DIBitmap` "leaked" by `CreateSkiaImageFromDib()`.
void ReleaseRetainedHeldBySkImage(const void* /*pixels*/,
                                  SkImages::ReleaseContext context) {
  RetainPtr<const CFX_DIBitmap> realized_bitmap;
  realized_bitmap.Unleak(reinterpret_cast<const CFX_DIBitmap*>(context));
}

// Creates an `SkImage` from a `CFX_DIBBase`.
sk_sp<SkImage> CreateSkiaImageFromDib(const CFX_DIBBase* source,
                                      SkColorType color_type,
                                      SkAlphaType alpha_type) {
  // Make sure the DIB is backed by a buffer.
  RetainPtr<const CFX_DIBitmap> realized_bitmap = source->RealizeIfNeeded();
  if (!realized_bitmap) {
    return nullptr;
  }
  CHECK(!realized_bitmap->GetBuffer().empty());

  // Transfer ownership of `realized_bitmap` to `bitmap`, which will be freed by
  // ReleaseRetainedHeldBySkImage().
  const CFX_DIBitmap* bitmap = realized_bitmap.Leak();
  SkImageInfo info = SkImageInfo::Make(bitmap->GetWidth(), bitmap->GetHeight(),
                                       color_type, alpha_type);
  auto result = SkImages::RasterFromPixmap(
      SkPixmap(info, bitmap->GetBuffer().data(), bitmap->GetPitch()),
      /*rasterReleaseProc=*/ReleaseRetainedHeldBySkImage,
      /*releaseContext=*/const_cast<CFX_DIBitmap*>(bitmap));
  CHECK(result);  // Otherwise, `bitmap` leaks.
  return result;
}

// Releases allocated memory "leaked" by `CreateSkiaImageFromTransformedDib()`.
void ReleaseAllocatedHeldBySkImage(const void* pixels,
                                   SkImages::ReleaseContext /*context*/) {
  FX_Free(const_cast<void*>(pixels));
}

// Template defining traits of a pixel transform function.
template <size_t source_bits_per_pixel, typename PixelTransform>
class PixelTransformTraits;

template <typename PixelTransform>
class PixelTransformTraits<1, PixelTransform> {
 public:
  using Result = std::invoke_result_t<PixelTransform, bool>;

  static Result Invoke(PixelTransform&& pixel_transform,
                       pdfium::span<const uint8_t> scanline,
                       size_t column) {
    uint8_t kMask = 1 << (7 - column % 8);
    return pixel_transform(!!(scanline[column / 8] & kMask));
  }
};

template <typename PixelTransform>
class PixelTransformTraits<8, PixelTransform> {
 public:
  using Result = std::invoke_result_t<PixelTransform, uint8_t>;

  static Result Invoke(PixelTransform&& pixel_transform,
                       pdfium::span<const uint8_t> scanline,
                       size_t column) {
    return pixel_transform(scanline[column]);
  }
};

template <typename PixelTransform>
class PixelTransformTraits<24, PixelTransform> {
 public:
  using Result =
      std::invoke_result_t<PixelTransform, uint8_t, uint8_t, uint8_t>;

  static Result Invoke(PixelTransform&& pixel_transform,
                       pdfium::span<const uint8_t> scanline,
                       size_t column) {
    size_t offset = column * 3;
    return pixel_transform(scanline[offset + 2], scanline[offset + 1],
                           scanline[offset]);
  }
};

template <typename PixelTransform>
class PixelTransformTraits<32, PixelTransform> {
 public:
  using Result =
      std::invoke_result_t<PixelTransform, uint8_t, uint8_t, uint8_t>;

  static Result Invoke(PixelTransform&& pixel_transform,
                       pdfium::span<const uint8_t> scanline,
                       size_t column) {
    size_t offset = column * 4;
    return pixel_transform(scanline[offset + 2], scanline[offset + 1],
                           scanline[offset]);
  }
};

void ValidateScanlineSize(pdfium::span<const uint8_t> scanline,
                          size_t min_row_bytes) {
  DCHECK_GE(scanline.size(), min_row_bytes);
}

// Creates an `SkImage` from a `CFX_DIBBase`, transforming the source pixels
// using `pixel_transform`.
//
// TODO(crbug.com/pdfium/2048): Consolidate with `CFX_DIBBase::ConvertBuffer()`.
template <size_t source_bits_per_pixel, typename PixelTransform>
sk_sp<SkImage> CreateSkiaImageFromTransformedDib(
    const CFX_DIBBase& source,
    SkColorType color_type,
    SkAlphaType alpha_type,
    PixelTransform&& pixel_transform) {
  using Traits = PixelTransformTraits<source_bits_per_pixel, PixelTransform>;
  using Result = typename Traits::Result;

  // Allocate output buffer.
  const int width = source.GetWidth();
  const int height = source.GetHeight();
  SkImageInfo info = SkImageInfo::Make(width, height, color_type, alpha_type);
  DCHECK_EQ(info.minRowBytes(), width * sizeof(Result));

  size_t output_size = Fx2DSizeOrDie(info.minRowBytes(), height);
  std::unique_ptr<void, FxFreeDeleter> output(
      FX_TryAlloc(uint8_t, output_size));
  if (!output) {
    return nullptr;
  }

  // Transform source pixels to output pixels. Iterate by individual scanline.
  Result* output_cursor = reinterpret_cast<Result*>(output.get());
  const size_t min_row_bytes =
      fxge::CalculatePitch8OrDie(source.GetBPP(), /*components=*/1, width);
  DCHECK_LE(min_row_bytes, source.GetPitch());

  int line = 0;
  for (int row = 0; row < height; ++row) {
    pdfium::span<const uint8_t> scanline = source.GetScanline(line++);
    ValidateScanlineSize(scanline, min_row_bytes);

    for (int column = 0; column < width; ++column) {
      UNSAFE_TODO(*output_cursor++) = Traits::Invoke(
          std::forward<PixelTransform>(pixel_transform), scanline, column);
    }
  }

  // "Leak" allocated memory to `SkImage`.
  return SkImages::RasterFromPixmap(
      SkPixmap(info, output.release(), info.minRowBytes()),
      /*rasterReleaseProc=*/ReleaseAllocatedHeldBySkImage,
      /*releaseContext=*/nullptr);
}

bool IsRGBColorGrayScale(uint32_t color) {
  return FXARGB_R(color) == FXARGB_G(color) &&
         FXARGB_R(color) == FXARGB_B(color);
}

}  // namespace

sk_sp<SkImage> CFX_DIBBase::RealizeSkImage() const {
  switch (GetBPP()) {
    case 1: {
      // By default, the two colors for grayscale are 0xFF and 0x00 unless they
      // are specified in the palette.
      uint8_t color0 = 0x00;
      uint8_t color1 = 0xFF;

      if (GetFormat() == FXDIB_Format::k1bppRgb && HasPalette()) {
        uint32_t palette_color0 = GetPaletteArgb(0);
        uint32_t palette_color1 = GetPaletteArgb(1);
        bool use_gray_colors = IsRGBColorGrayScale(palette_color0) &&
                               IsRGBColorGrayScale(palette_color1);
        if (!use_gray_colors) {
          return CreateSkiaImageFromTransformedDib</*source_bits_per_pixel=*/1>(
              *this, kBGRA_8888_SkColorType, kPremul_SkAlphaType,
              [palette_color0, palette_color1](bool bit) {
                return bit ? palette_color1 : palette_color0;
              });
        }

        color0 = FXARGB_R(palette_color0);
        color1 = FXARGB_R(palette_color1);
      }

      return CreateSkiaImageFromTransformedDib</*source_bits_per_pixel=*/1>(
          *this, IsMaskFormat() ? kAlpha_8_SkColorType : kGray_8_SkColorType,
          kPremul_SkAlphaType,
          [color0, color1](bool bit) { return bit ? color1 : color0; });
    }

    case 8:
      // we upscale ctables to 32bit.
      if (HasPalette()) {
        return CreateSkiaImageFromTransformedDib</*source_bits_per_pixel=*/8>(
            *this, kBGRA_8888_SkColorType, kPremul_SkAlphaType,
            [palette = GetPaletteSpan().first(GetRequiredPaletteSize())](
                uint8_t index) {
              if (index >= palette.size()) {
                index = 0;
              }
              return palette[index];
            });
      }
      return CreateSkiaImageFromDib(
          this, IsMaskFormat() ? kAlpha_8_SkColorType : kGray_8_SkColorType,
          kPremul_SkAlphaType);

    case 24:
      return CreateSkiaImageFromTransformedDib</*source_bits_per_pixel=*/24>(
          *this, kBGRA_8888_SkColorType, kOpaque_SkAlphaType,
          [](uint8_t red, uint8_t green, uint8_t blue) {
            return SkPackARGB32(0xFF, red, green, blue);
          });

    case 32:
      switch (GetFormat()) {
        case FXDIB_Format::kBgrx:
          return CreateSkiaImageFromTransformedDib<
              /*source_bits_per_pixel=*/32>(
              *this, kBGRA_8888_SkColorType, kOpaque_SkAlphaType,
              [](uint8_t red, uint8_t green, uint8_t blue) {
                return SkPackARGB32(0xFF, red, green, blue);
              });
        case FXDIB_Format::kBgra:
          return CreateSkiaImageFromDib(this, kBGRA_8888_SkColorType,
                                        kUnpremul_SkAlphaType);
        case FXDIB_Format::kBgraPremul:
          return CreateSkiaImageFromDib(this, kBGRA_8888_SkColorType,
                                        kPremul_SkAlphaType);
        default:
          NOTREACHED_NORETURN();
      }
    default:
      NOTREACHED_NORETURN();
  }
}
