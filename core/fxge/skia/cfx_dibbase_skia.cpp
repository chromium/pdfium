// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibbase.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"
#include "third_party/base/notreached.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace {

// Releases `CFX_DIBBase` "leaked" by `CreateSkiaImageFromDib()`.
void ReleaseRetainedHeldBySkImage(const void* /*pixels*/,
                                  SkImages::ReleaseContext context) {
  RetainPtr<const CFX_DIBBase> retained;
  retained.Unleak(reinterpret_cast<const CFX_DIBBase*>(context));
}

// Creates an `SkImage` from a `CFX_DIBBase`, sharing the underlying pixels if
// possible.
//
// Note that an `SkImage` must be immutable, so if sharing pixels, they must not
// be modified during the lifetime of the `SkImage`.
sk_sp<SkImage> CreateSkiaImageFromDib(const CFX_DIBBase* source,
                                      SkColorType color_type,
                                      SkAlphaType alpha_type) {
  // Make sure the DIB is backed by a buffer.
  RetainPtr<const CFX_DIBBase> retained;
  if (source->GetBuffer().empty()) {
    retained = source->Realize();
    if (!retained) {
      return nullptr;
    }
    DCHECK(!retained->GetBuffer().empty());
  } else {
    retained.Reset(source);
  }

  // Convert unowned pointer to a retained pointer, then "leak" to `SkImage`.
  source = retained.Leak();
  SkImageInfo info = SkImageInfo::Make(source->GetWidth(), source->GetHeight(),
                                       color_type, alpha_type);
  return SkImages::RasterFromPixmap(
      SkPixmap(info, source->GetBuffer().data(), source->GetPitch()),
      /*rasterReleaseProc=*/ReleaseRetainedHeldBySkImage,
      /*releaseContext=*/const_cast<CFX_DIBBase*>(source));
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
                       const uint8_t* scanline,
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
                       const uint8_t* scanline,
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
                       const uint8_t* scanline,
                       size_t column) {
    size_t offset = column * 3;
    return pixel_transform(scanline[offset + 2], scanline[offset + 1],
                           scanline[offset]);
  }
};

void ValidateScanlineSize(pdfium::span<const uint8_t> scanline,
                          size_t min_row_bytes) {
  DCHECK_GE(scanline.size(), min_row_bytes);
}

void ValidateBufferSize(pdfium::span<const uint8_t> buffer,
                        const CFX_DIBBase& source) {
#if DCHECK_IS_ON()
  if (source.GetHeight() == 0) {
    return;
  }

  FX_SAFE_SIZE_T buffer_size = source.GetHeight() - 1;
  buffer_size *= source.GetPitch();
  buffer_size += fxge::CalculatePitch8OrDie(source.GetBPP(), /*components=*/1,
                                            source.GetWidth());

  DCHECK_GE(buffer.size(), buffer_size.ValueOrDie());
#endif  // DCHECK_IS_ON()
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

  // Transform source pixels to output pixels.
  pdfium::span<const uint8_t> source_buffer = source.GetBuffer();
  Result* output_cursor = reinterpret_cast<Result*>(output.get());
  if (source_buffer.empty()) {
    // No buffer; iterate by individual scanline.
    const size_t min_row_bytes =
        fxge::CalculatePitch8OrDie(source.GetBPP(), /*components=*/1, width);
    DCHECK_LE(min_row_bytes, source.GetPitch());

    int line = 0;
    for (int row = 0; row < height; ++row) {
      pdfium::span<const uint8_t> scanline = source.GetScanline(line++);
      ValidateScanlineSize(scanline, min_row_bytes);

      for (int column = 0; column < width; ++column) {
        *output_cursor++ =
            Traits::Invoke(std::forward<PixelTransform>(pixel_transform),
                           scanline.data(), column);
      }
    }
  } else {
    // Iterate over the entire buffer.
    ValidateBufferSize(source_buffer, source);
    const size_t row_bytes = source.GetPitch();

    const uint8_t* next_scanline = source_buffer.data();
    for (int row = 0; row < height; ++row) {
      const uint8_t* scanline = next_scanline;
      next_scanline += row_bytes;

      for (int column = 0; column < width; ++column) {
        *output_cursor++ = Traits::Invoke(
            std::forward<PixelTransform>(pixel_transform), scanline, column);
      }
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
            return SkPackARGB32NoCheck(0xFF, red, green, blue);
          });

    case 32:
      return CreateSkiaImageFromDib(
          this, kBGRA_8888_SkColorType,
          IsPremultiplied() ? kPremul_SkAlphaType : kUnpremul_SkAlphaType);

    default:
      NOTREACHED_NORETURN();
  }
}

bool CFX_DIBBase::IsPremultiplied() const {
  return false;
}
