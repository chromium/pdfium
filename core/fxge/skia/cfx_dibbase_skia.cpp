// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/dib/cfx_dibbase.h"

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <utility>

#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkColorPriv.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace {

// A builder that can create an `SkImage` either from a `CFX_DIBBase`, or from
// newly-allocated memory.
class SkiaImageBuilder {
 public:
  SkiaImageBuilder() = default;

  // Constructs a builder backed by pixels shared with a `CFX_DIBBase`. The
  // pixels are not accessible from the builder. The `CFX_DIBBase` must outlive
  // the builder, as the reference count on the `CFX_DIBBase` is not incremented
  // until `Build()` is called.
  SkiaImageBuilder(const CFX_DIBBase* source, SkColorType color_type)
      : info_(SkImageInfo::Make(source->GetWidth(),
                                source->GetHeight(),
                                color_type,
                                kPremul_SkAlphaType)),
        data_(UnownedType(source)) {
    // TODO(crbug.com/pdfium/2047): `Realize()` if there's no buffer?
    row_bytes_ = source->GetPitch();
  }

  // Constructs a builder backed by allocated pixels.
  explicit SkiaImageBuilder(const SkImageInfo& info) : info_(info) {
    row_bytes_ = info_.minRowBytes();
    data_size_ = Fx2DSizeOrDie(row_bytes_, info_.height());
    data_ = AllocatedType(FX_Alloc(uint8_t, data_size_));
  }

  SkiaImageBuilder(SkiaImageBuilder&&) = default;
  SkiaImageBuilder& operator=(SkiaImageBuilder&&) = default;

  // Gets the number of bytes per row.
  size_t row_bytes() const { return row_bytes_; }

  // Gets the 8-bit pixels (if using allocated memory).
  pdfium::span<uint8_t> allocated_pixels8() {
    DCHECK_EQ(static_cast<size_t>(info_.bytesPerPixel()), sizeof(uint8_t));
    return pdfium::make_span(
        reinterpret_cast<uint8_t*>(absl::get<AllocatedType>(data_).get()),
        data_size_);
  }

  // Gets the 32-bit pixels (if using allocated memory).
  pdfium::span<uint32_t> allocated_pixels32() {
    DCHECK_EQ(static_cast<size_t>(info_.bytesPerPixel()), sizeof(uint32_t));
    return pdfium::make_span(
        reinterpret_cast<uint32_t*>(absl::get<AllocatedType>(data_).get()),
        data_size_ / sizeof(uint32_t));
  }

  // Builds an `SkImage` that takes ownership of the pixels. If sharing pixels
  // with a `CFX_DIBBase`, this increments the reference count on the
  // `CFX_DIBBase`.
  //
  // Note that an `SkImage` must be immutable, so the pixels must not be
  // modified during the lifetime of the `SkImage`.
  sk_sp<SkImage> Build() && {
    if (absl::holds_alternative<AllocatedType>(data_)) {
      // "Leak" allocated memory to `SkImage`.
      return SkImages::RasterFromPixmap(
          SkPixmap(info_, absl::get<AllocatedType>(data_).release(),
                   row_bytes_),
          /*rasterReleaseProc=*/ReleaseAllocated,
          /*releaseContext=*/nullptr);
    }

    // Convert unowned pointer to a retained pointer, then "leak" to `SkImage`.
    RetainPtr<const CFX_DIBBase> retained(
        absl::get<UnownedType>(data_).ExtractAsDangling());
    const CFX_DIBBase* source = retained.Leak();
    return SkImages::RasterFromPixmap(
        SkPixmap(info_, source->GetBuffer().data(), row_bytes_),
        /*rasterReleaseProc=*/ReleaseRetained,
        /*releaseContext=*/const_cast<CFX_DIBBase*>(source));
  }

 private:
  using UnownedType = UnownedPtr<const CFX_DIBBase>;
  using AllocatedType = std::unique_ptr<void, FxFreeDeleter>;

  // Releases `CFX_DIBBase` "leaked" by `Build()`.
  static void ReleaseRetained(const void* /*pixels*/,
                              SkImages::ReleaseContext context) {
    RetainPtr<const CFX_DIBBase> retained;
    retained.Unleak(reinterpret_cast<const CFX_DIBBase*>(context));
  }

  // Releases allocated memory "leaked" by `Build()`.
  static void ReleaseAllocated(const void* pixels,
                               SkImages::ReleaseContext /*context*/) {
    FX_Free(const_cast<void*>(pixels));
  }

  SkImageInfo info_;
  size_t row_bytes_;

  // Similar to `MaybeOwned<uint32_t, FxFreeDeleter>`, but holds either an
  // unowned `CFX_DIBBase` pointer or an owned `void` pointer.
  absl::variant<UnownedType, AllocatedType> data_;
  size_t data_size_;
};

// Creates a `SkiaImageBuilder` using colors from a 1-bit-per-pixel palette.
SkiaImageBuilder CreateSkiaImageBuilderUsingSingleBitPalette(
    const CFX_DIBBase* source) {
  DCHECK_EQ(1, source->GetBPP());
  int width = source->GetWidth();
  int height = source->GetHeight();
  const void* buffer = source->GetBuffer().data();
  DCHECK(buffer);

  uint32_t color0 = source->GetPaletteArgb(0);
  uint32_t color1 = source->GetPaletteArgb(1);
  SkiaImageBuilder image_builder(SkImageInfo::Make(
      width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType));
  pdfium::span<SkPMColor> dst32_pixels(image_builder.allocated_pixels32());

  for (int y = 0; y < height; ++y) {
    const uint8_t* src_row =
        static_cast<const uint8_t*>(buffer) + y * source->GetPitch();
    pdfium::span<uint32_t> dst_row = dst32_pixels.subspan(y * width);
    for (int x = 0; x < width; ++x) {
      bool use_color1 = src_row[x / 8] & (1 << (7 - x % 8));
      dst_row[x] = use_color1 ? color1 : color0;
    }
  }
  return image_builder;
}

// Creates a `SkiaImageBuilder` using colors from `palette`.
SkiaImageBuilder CreateSkiaImageBuilderUsingPalette(
    const CFX_DIBBase* source,
    pdfium::span<const uint32_t> palette) {
  DCHECK_EQ(8, source->GetBPP());
  int width = source->GetWidth();
  int height = source->GetHeight();
  const void* buffer = source->GetBuffer().data();
  DCHECK(buffer);
  SkiaImageBuilder image_builder(SkImageInfo::Make(
      width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType));
  pdfium::span<SkPMColor> dst32_pixels(image_builder.allocated_pixels32());

  for (int y = 0; y < height; ++y) {
    const uint8_t* src_row =
        static_cast<const uint8_t*>(buffer) + y * source->GetPitch();
    pdfium::span<uint32_t> dst_row = dst32_pixels.subspan(y * width);
    for (int x = 0; x < width; ++x) {
      unsigned index = src_row[x];
      if (index >= palette.size()) {
        index = 0;
      }
      dst_row[x] = palette[index];
    }
  }
  return image_builder;
}

bool IsRGBColorGrayScale(uint32_t color) {
  return FXARGB_R(color) == FXARGB_G(color) &&
         FXARGB_R(color) == FXARGB_B(color);
}

}  // namespace

sk_sp<SkImage> CFX_DIBBase::RealizeSkImage(bool force_alpha) const {
  const uint8_t* const buffer = GetBuffer().data();
  if (!buffer) {
    return nullptr;
  }

  const SkColorType color_type = force_alpha || IsMaskFormat()
                                     ? SkColorType::kAlpha_8_SkColorType
                                     : SkColorType::kGray_8_SkColorType;
  const int width = GetWidth();
  const int height = GetHeight();
  const int row_bytes = GetPitch();
  SkiaImageBuilder image_builder(this, color_type);
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
          image_builder = CreateSkiaImageBuilderUsingSingleBitPalette(this);
          break;
        }

        color0 = FXARGB_R(palette_color0);
        color1 = FXARGB_R(palette_color1);
      }

      image_builder = SkiaImageBuilder(
          SkImageInfo::Make(width, height, color_type, kPremul_SkAlphaType));
      pdfium::span<uint8_t> dst8_pixels = image_builder.allocated_pixels8();
      for (int y = 0; y < height; ++y) {
        const uint8_t* src_row = buffer + y * row_bytes;
        pdfium::span<uint8_t> dst_row =
            dst8_pixels.subspan(y * image_builder.row_bytes());
        for (int x = 0; x < width; ++x) {
          dst_row[x] = src_row[x >> 3] & (1 << (~x & 0x07)) ? color1 : color0;
        }
      }
      break;
    }
    case 8:
      // we upscale ctables to 32bit.
      if (HasPalette()) {
        const size_t src_palette_size = GetRequiredPaletteSize();
        pdfium::span<const uint32_t> src_palette = GetPaletteSpan();
        CHECK_LE(src_palette_size, src_palette.size());
        if (src_palette_size < src_palette.size()) {
          src_palette = src_palette.first(src_palette_size);
        }

        image_builder = CreateSkiaImageBuilderUsingPalette(this, src_palette);
      }
      break;
    case 24: {
      image_builder = SkiaImageBuilder(SkImageInfo::Make(
          width, height, kBGRA_8888_SkColorType, kOpaque_SkAlphaType));
      pdfium::span<uint32_t> dst32_pixels = image_builder.allocated_pixels32();
      for (int y = 0; y < height; ++y) {
        const uint8_t* src_row = buffer + y * row_bytes;
        pdfium::span<uint32_t> dst_row = dst32_pixels.subspan(y * width);
        for (int x = 0; x < width; ++x) {
          dst_row[x] = SkPackARGB32NoCheck(
              0xFF, src_row[x * 3 + 2], src_row[x * 3 + 1], src_row[x * 3 + 0]);
        }
      }
      break;
    }
    case 32:
      image_builder = SkiaImageBuilder(this, kBGRA_8888_SkColorType);
      break;
    default:
      NOTREACHED();
  }

  return std::move(image_builder).Build();
}
