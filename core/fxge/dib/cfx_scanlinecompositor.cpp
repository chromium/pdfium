// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_scanlinecompositor.h"

#include <algorithm>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/dib/blend.h"
#include "core/fxge/dib/fx_dib.h"

using fxge::Blend;

#define FXDIB_ALPHA_UNION(dest, src) ((dest) + (src) - (dest) * (src) / 255)
#define FXARGB_RGBORDERCOPY(dest, src)                  \
  *((dest) + 3) = *((src) + 3), *(dest) = *((src) + 2), \
             *((dest) + 1) = *((src) + 1), *((dest) + 2) = *((src))

namespace {

int Lum(FX_RGB_STRUCT<int> color) {
  return (color.red * 30 + color.green * 59 + color.blue * 11) / 100;
}

FX_RGB_STRUCT<int> ClipColor(FX_RGB_STRUCT<int> color) {
  int l = Lum(color);
  int n = std::min(color.red, std::min(color.green, color.blue));
  int x = std::max(color.red, std::max(color.green, color.blue));
  if (n < 0) {
    color.red = l + ((color.red - l) * l / (l - n));
    color.green = l + ((color.green - l) * l / (l - n));
    color.blue = l + ((color.blue - l) * l / (l - n));
  }
  if (x > 255) {
    color.red = l + ((color.red - l) * (255 - l) / (x - l));
    color.green = l + ((color.green - l) * (255 - l) / (x - l));
    color.blue = l + ((color.blue - l) * (255 - l) / (x - l));
  }
  return color;
}

FX_RGB_STRUCT<int> SetLum(FX_RGB_STRUCT<int> color, int l) {
  int d = l - Lum(color);
  color.red += d;
  color.green += d;
  color.blue += d;
  return ClipColor(color);
}

int Sat(FX_RGB_STRUCT<int> color) {
  return std::max(color.red, std::max(color.green, color.blue)) -
         std::min(color.red, std::min(color.green, color.blue));
}

FX_RGB_STRUCT<int> SetSat(FX_RGB_STRUCT<int> color, int s) {
  int min = std::min(color.red, std::min(color.green, color.blue));
  int max = std::max(color.red, std::max(color.green, color.blue));
  if (min == max)
    return {};

  color.red = (color.red - min) * s / (max - min);
  color.green = (color.green - min) * s / (max - min);
  color.blue = (color.blue - min) * s / (max - min);
  return color;
}

void RGB_Blend(BlendMode blend_mode,
               const uint8_t* src_scan,
               const uint8_t* dest_scan,
               int results[3]) {
  UNSAFE_TODO({
    FX_RGB_STRUCT<int> result = {};
    FX_RGB_STRUCT<int> src = {
        .red = src_scan[2], .green = src_scan[1], .blue = src_scan[0]};
    FX_RGB_STRUCT<int> back = {
        .red = dest_scan[2], .green = dest_scan[1], .blue = dest_scan[0]};
    switch (blend_mode) {
      case BlendMode::kHue:
        result = SetLum(SetSat(src, Sat(back)), Lum(back));
        break;
      case BlendMode::kSaturation:
        result = SetLum(SetSat(back, Sat(src)), Lum(back));
        break;
      case BlendMode::kColor:
        result = SetLum(src, Lum(back));
        break;
      case BlendMode::kLuminosity:
        result = SetLum(back, Lum(src));
        break;
      default:
        break;
    }
    results[0] = result.blue;
    results[1] = result.green;
    results[2] = result.red;
  });
}

int GetAlpha(uint8_t src_alpha, const uint8_t* clip_scan, int col) {
  return clip_scan ? UNSAFE_TODO(clip_scan[col]) * src_alpha / 255 : src_alpha;
}

int GetAlphaWithSrc(uint8_t src_alpha,
                    const uint8_t* clip_scan,
                    const uint8_t* src_scan,
                    int col) {
  UNSAFE_TODO({
    int result = src_alpha * src_scan[col];
    if (clip_scan) {
      result *= clip_scan[col];
      result /= 255;
    }
    return result / 255;
  });
}

void CompositeRow_AlphaToMask(pdfium::span<uint8_t> dest_span,
                              pdfium::span<const uint8_t> src_span,
                              int pixel_count,
                              pdfium::span<const uint8_t> clip_span,
                              uint8_t stride) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    src_scan += stride - 1;
    for (int col = 0; col < pixel_count; ++col) {
      int src_alpha = GetAlpha(*src_scan, clip_scan, col);
      uint8_t back_alpha = *dest_scan;
      if (!back_alpha) {
        *dest_scan = src_alpha;
      } else if (src_alpha) {
        *dest_scan = back_alpha + src_alpha - back_alpha * src_alpha / 255;
      }
      ++dest_scan;
      src_scan += stride;
    }
  });
}

void CompositeRow_Rgb2Mask(pdfium::span<uint8_t> dest_span,
                           int width,
                           pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    if (!clip_scan) {
      FXSYS_memset(dest_scan, 0xff, width);
      return;
    }
    for (int i = 0; i < width; ++i) {
      *dest_scan = FXDIB_ALPHA_UNION(*dest_scan, *clip_scan);
      ++dest_scan;
      ++clip_scan;
    }
  });
}

bool IsNonSeparableBlendMode(BlendMode mode) {
  switch (mode) {
    case BlendMode::kHue:
    case BlendMode::kSaturation:
    case BlendMode::kColor:
    case BlendMode::kLuminosity:
      return true;
    default:
      return false;
  }
}

uint8_t GetGray(const uint8_t* src_scan) {
  return UNSAFE_TODO(FXRGB2GRAY(src_scan[2], src_scan[1], *src_scan));
}

uint8_t GetGrayWithBlend(const uint8_t* src_scan,
                         const uint8_t* dest_scan,
                         BlendMode blend_type) {
  uint8_t gray = GetGray(src_scan);
  if (IsNonSeparableBlendMode(blend_type))
    gray = blend_type == BlendMode::kLuminosity ? gray : *dest_scan;
  else if (blend_type != BlendMode::kNormal)
    gray = Blend(blend_type, *dest_scan, gray);
  return gray;
}

void CompositeRow_Argb2Gray(pdfium::span<uint8_t> dest_span,
                            pdfium::span<const uint8_t> src_span,
                            int pixel_count,
                            BlendMode blend_type,
                            pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  constexpr size_t kOffset = 4;
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; ++col) {
      int src_alpha = GetAlpha(src_scan[3], clip_scan, col);
      if (src_alpha) {
        uint8_t gray = GetGrayWithBlend(src_scan, dest_scan, blend_type);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, src_alpha);
      }
      ++dest_scan;
      src_scan += kOffset;
    }
  });
}

void CompositeRow_Rgb2Gray(pdfium::span<uint8_t> dest_span,
                           pdfium::span<const uint8_t> src_span,
                           int src_Bpp,
                           int pixel_count,
                           BlendMode blend_type,
                           pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; ++col) {
      uint8_t gray = GetGrayWithBlend(src_scan, dest_scan, blend_type);
      if (clip_scan && clip_scan[col] < 255) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, clip_scan[col]);
      } else {
        *dest_scan = gray;
      }
      ++dest_scan;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_Argb2Argb(pdfium::span<uint8_t> dest_span,
                            pdfium::span<const uint8_t> src_span,
                            int pixel_count,
                            BlendMode blend_type,
                            pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  constexpr size_t kOffset = 4;
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; ++col) {
      uint8_t back_alpha = dest_scan[3];
      uint8_t src_alpha = GetAlpha(src_scan[3], clip_scan, col);
      if (back_alpha == 0) {
        if (clip_scan) {
          FXARGB_SetDIB(dest_scan, (FXARGB_GetDIB(src_scan) & 0xffffff) |
                                       (src_alpha << 24));
        } else {
          FXSYS_memcpy(dest_scan, src_scan, 4);
        }
        dest_scan += kOffset;
        src_scan += kOffset;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += kOffset;
        src_scan += kOffset;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; ++color) {
        if (blend_type != BlendMode::kNormal) {
          int blended = bNonseparableBlend
                            ? blended_colors[color]
                            : Blend(blend_type, *dest_scan, *src_scan);
          blended = FXDIB_ALPHA_MERGE(*src_scan, blended, back_alpha);
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        } else {
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, alpha_ratio);
        }
        ++dest_scan;
        ++src_scan;
      }
      ++dest_scan;
      ++src_scan;
    }
  });
}

void CompositeRow_Rgb2Argb_Blend_NoClip(pdfium::span<uint8_t> dest_span,
                                        pdfium::span<const uint8_t> src_span,
                                        int width,
                                        BlendMode blend_type,
                                        int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; ++col) {
      uint8_t* dest_alpha = &dest_scan[3];
      uint8_t back_alpha = *dest_alpha;
      if (back_alpha == 0) {
        if (src_Bpp == 4) {
          FXARGB_SetDIB(dest_scan, 0xff000000 | FXARGB_GetDIB(src_scan));
        } else {
          FXARGB_SetDIB(dest_scan, ArgbEncode(0xff, src_scan[2], src_scan[1],
                                              src_scan[0]));
        }
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      *dest_alpha = 0xff;
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; ++color) {
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, *dest_scan, src_color);
        *dest_scan = FXDIB_ALPHA_MERGE(src_color, blended, back_alpha);
        ++dest_scan;
        ++src_scan;
      }
      ++dest_scan;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Argb_Blend_Clip(pdfium::span<uint8_t> dest_span,
                                      pdfium::span<const uint8_t> src_span,
                                      int width,
                                      BlendMode blend_type,
                                      int src_Bpp,
                                      pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; ++col) {
      int src_alpha = *clip_scan++;
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        FXSYS_memcpy(dest_scan, src_scan, 3);
        dest_scan += 3;
        src_scan += src_Bpp;
        dest_scan++;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, *dest_scan, src_color);
        blended = FXDIB_ALPHA_MERGE(src_color, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        dest_scan++;
        src_scan++;
      }
      src_scan += src_gap;
      dest_scan++;
    }
  });
}

void CompositeRow_Rgb2Argb_NoBlend_Clip(pdfium::span<uint8_t> dest_span,
                                        pdfium::span<const uint8_t> src_span,
                                        int width,
                                        int src_Bpp,
                                        pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_alpha = clip_scan[col];
      if (src_alpha == 255) {
        FXSYS_memcpy(dest_scan, src_scan, 3);
        dest_scan += 3;
        *dest_scan++ = 255;
        src_scan += src_Bpp;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      for (int color = 0; color < 3; color++) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, alpha_ratio);
        dest_scan++;
        src_scan++;
      }
      dest_scan++;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Argb_NoBlend_NoClip(pdfium::span<uint8_t> dest_span,
                                          pdfium::span<const uint8_t> src_span,
                                          int width,
                                          int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      if (src_Bpp == 4) {
        FXARGB_SetDIB(dest_scan, 0xff000000 | FXARGB_GetDIB(src_scan));
      } else {
        FXARGB_SetDIB(dest_scan,
                      ArgbEncode(0xff, src_scan[2], src_scan[1], src_scan[0]));
      }
      dest_scan += 4;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_Argb2Rgb_Blend(pdfium::span<uint8_t> dest_span,
                                 pdfium::span<const uint8_t> src_span,
                                 int width,
                                 BlendMode blend_type,
                                 int dest_Bpp,
                                 pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int dest_gap = dest_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha;
      if (clip_scan) {
        src_alpha = src_scan[3] * (*clip_scan++) / 255;
      } else {
        src_alpha = src_scan[3];
      }
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int back_color = *dest_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, *src_scan);
        *dest_scan = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        dest_scan++;
        src_scan++;
      }
      dest_scan += dest_gap;
      src_scan++;
    }
  });
}

void CompositeRow_Argb2Rgb_NoBlend(pdfium::span<uint8_t> dest_span,
                                   pdfium::span<const uint8_t> src_span,
                                   int width,
                                   int dest_Bpp,
                                   pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int dest_gap = dest_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha;
      if (clip_scan) {
        src_alpha = src_scan[3] * (*clip_scan++) / 255;
      } else {
        src_alpha = src_scan[3];
      }
      if (src_alpha == 255) {
        FXSYS_memcpy(dest_scan, src_scan, 3);
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      for (int color = 0; color < 3; color++) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, src_alpha);
        dest_scan++;
        src_scan++;
      }
      dest_scan += dest_gap;
      src_scan++;
    }
  });
}

void CompositeRow_Rgb2Rgb_Blend_NoClip(pdfium::span<uint8_t> dest_span,
                                       pdfium::span<const uint8_t> src_span,
                                       int width,
                                       BlendMode blend_type,
                                       int dest_Bpp,
                                       int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int dest_gap = dest_Bpp - 3;
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int back_color = *dest_scan;
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, src_color);
        *dest_scan = blended;
        dest_scan++;
        src_scan++;
      }
      dest_scan += dest_gap;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Rgb_Blend_Clip(pdfium::span<uint8_t> dest_span,
                                     pdfium::span<const uint8_t> src_span,
                                     int width,
                                     BlendMode blend_type,
                                     int dest_Bpp,
                                     int src_Bpp,
                                     pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int dest_gap = dest_Bpp - 3;
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha = *clip_scan++;
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += src_Bpp;
        continue;
      }
      if (bNonseparableBlend) {
        RGB_Blend(blend_type, src_scan, dest_scan, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int src_color = *src_scan;
        int back_color = *dest_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, src_color);
        *dest_scan = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        dest_scan++;
        src_scan++;
      }
      dest_scan += dest_gap;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Rgb_NoBlend_NoClip(pdfium::span<uint8_t> dest_span,
                                         pdfium::span<const uint8_t> src_span,
                                         int width,
                                         int dest_Bpp,
                                         int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  UNSAFE_TODO({
    if (dest_Bpp == src_Bpp) {
      FXSYS_memcpy(dest_scan, src_scan, width * dest_Bpp);
      return;
    }
    for (int col = 0; col < width; col++) {
      FXSYS_memcpy(dest_scan, src_scan, 3);
      dest_scan += dest_Bpp;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_Rgb2Rgb_NoBlend_Clip(pdfium::span<uint8_t> dest_span,
                                       pdfium::span<const uint8_t> src_span,
                                       int width,
                                       int dest_Bpp,
                                       int src_Bpp,
                                       pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_alpha = clip_scan[col];
      if (src_alpha == 255) {
        FXSYS_memcpy(dest_scan, src_scan, 3);
      } else if (src_alpha) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, src_alpha);
        dest_scan++;
        src_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, src_alpha);
        dest_scan++;
        src_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, *src_scan, src_alpha);
        dest_scan += dest_Bpp - 2;
        src_scan += src_Bpp - 2;
        continue;
      }
      dest_scan += dest_Bpp;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_8bppPal2Gray(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               pdfium::span<const uint8_t> palette_span,
                               int pixel_count,
                               BlendMode blend_type,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  const uint8_t* pPalette = palette_span.data();
  UNSAFE_TODO({
    if (blend_type != BlendMode::kNormal) {
      bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
      for (int col = 0; col < pixel_count; col++) {
        uint8_t gray = pPalette[*src_scan];
        if (bNonseparableBlend) {
          gray = blend_type == BlendMode::kLuminosity ? gray : *dest_scan;
        } else {
          gray = Blend(blend_type, *dest_scan, gray);
        }
        if (clip_scan && clip_scan[col] < 255) {
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, clip_scan[col]);
        } else {
          *dest_scan = gray;
        }
        dest_scan++;
        src_scan++;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      uint8_t gray = pPalette[*src_scan];
      if (clip_scan && clip_scan[col] < 255)
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, clip_scan[col]);
      else
        *dest_scan = gray;
      dest_scan++;
      src_scan++;
    }
  });
}

void CompositeRow_1bppPal2Gray(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               int src_left,
                               pdfium::span<const uint8_t> src_palette,
                               int pixel_count,
                               BlendMode blend_type,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int reset_gray = src_palette[0];
  int set_gray = src_palette[1];
  UNSAFE_TODO({
    if (blend_type != BlendMode::kNormal) {
      bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
      for (int col = 0; col < pixel_count; col++) {
        uint8_t gray =
            (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8)))
                ? set_gray
                : reset_gray;
        if (bNonseparableBlend) {
          gray = blend_type == BlendMode::kLuminosity ? gray : *dest_scan;
        } else {
          gray = Blend(blend_type, *dest_scan, gray);
        }
        if (clip_scan && clip_scan[col] < 255) {
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, clip_scan[col]);
        } else {
          *dest_scan = gray;
        }
        dest_scan++;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      uint8_t gray =
          (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8)))
              ? set_gray
              : reset_gray;
      if (clip_scan && clip_scan[col] < 255) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, clip_scan[col]);
      } else {
        *dest_scan = gray;
      }
      dest_scan++;
    }
  });
}

void CompositeRow_8bppRgb2Rgb_NoBlend(pdfium::span<uint8_t> dest_span,
                                      pdfium::span<const uint8_t> src_span,
                                      pdfium::span<const uint32_t> palette_span,
                                      int pixel_count,
                                      int DestBpp,
                                      pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  const uint32_t* pPalette = palette_span.data();
  FX_ARGB argb = 0;
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      argb = pPalette[*src_scan];
      int src_r = FXARGB_R(argb);
      int src_g = FXARGB_G(argb);
      int src_b = FXARGB_B(argb);
      if (clip_scan && clip_scan[col] < 255) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, clip_scan[col]);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, clip_scan[col]);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, clip_scan[col]);
        dest_scan++;
      } else {
        *dest_scan++ = src_b;
        *dest_scan++ = src_g;
        *dest_scan++ = src_r;
      }
      if (DestBpp == 4) {
        dest_scan++;
      }
      src_scan++;
    }
  });
}

void CompositeRow_1bppRgb2Rgb_NoBlend(pdfium::span<uint8_t> dest_span,
                                      pdfium::span<const uint8_t> src_span,
                                      int src_left,
                                      pdfium::span<const uint32_t> src_palette,
                                      int pixel_count,
                                      int DestBpp,
                                      pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int reset_r = FXARGB_R(src_palette[0]);
  int reset_g = FXARGB_G(src_palette[0]);
  int reset_b = FXARGB_B(src_palette[0]);
  int set_r = FXARGB_R(src_palette[1]);
  int set_g = FXARGB_G(src_palette[1]);
  int set_b = FXARGB_B(src_palette[1]);
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_r;
      int src_g;
      int src_b;
      if (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8))) {
        src_r = set_r;
        src_g = set_g;
        src_b = set_b;
      } else {
        src_r = reset_r;
        src_g = reset_g;
        src_b = reset_b;
      }
      if (clip_scan && clip_scan[col] < 255) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, clip_scan[col]);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, clip_scan[col]);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, clip_scan[col]);
        dest_scan++;
      } else {
        *dest_scan++ = src_b;
        *dest_scan++ = src_g;
        *dest_scan++ = src_r;
      }
      if (DestBpp == 4) {
        dest_scan++;
      }
    }
  });
}

void CompositeRow_8bppRgb2Argb_NoBlend(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    pdfium::span<const uint32_t> palette_span,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  const uint32_t* pPalette = palette_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      FX_ARGB argb = pPalette[*src_scan];
      int src_r = FXARGB_R(argb);
      int src_g = FXARGB_G(argb);
      int src_b = FXARGB_B(argb);
      if (!clip_scan || clip_scan[col] == 255) {
        *dest_scan++ = src_b;
        *dest_scan++ = src_g;
        *dest_scan++ = src_r;
        *dest_scan++ = 255;
        src_scan++;
        continue;
      }
      int src_alpha = clip_scan[col];
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan++;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, alpha_ratio);
      dest_scan++;
      dest_scan++;
      src_scan++;
    }
  });
}

void CompositeRow_1bppRgb2Argb_NoBlend(pdfium::span<uint8_t> dest_span,
                                       pdfium::span<const uint8_t> src_span,
                                       int src_left,
                                       int width,
                                       pdfium::span<const uint32_t> src_palette,
                                       pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int reset_r = FXARGB_R(src_palette[0]);
  int reset_g = FXARGB_G(src_palette[0]);
  int reset_b = FXARGB_B(src_palette[0]);
  int set_r = FXARGB_R(src_palette[1]);
  int set_g = FXARGB_G(src_palette[1]);
  int set_b = FXARGB_B(src_palette[1]);
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_r;
      int src_g;
      int src_b;
      if (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8))) {
        src_r = set_r;
        src_g = set_g;
        src_b = set_b;
      } else {
        src_r = reset_r;
        src_g = reset_g;
        src_b = reset_b;
      }
      if (!clip_scan || clip_scan[col] == 255) {
        *dest_scan++ = src_b;
        *dest_scan++ = src_g;
        *dest_scan++ = src_r;
        *dest_scan++ = 255;
        continue;
      }
      int src_alpha = clip_scan[col];
      if (src_alpha == 0) {
        dest_scan += 4;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, alpha_ratio);
      dest_scan++;
      dest_scan++;
    }
  });
}

void CompositeRow_ByteMask2Argb(pdfium::span<uint8_t> dest_span,
                                pdfium::span<const uint8_t> src_span,
                                int mask_alpha,
                                int src_r,
                                int src_g,
                                int src_b,
                                int pixel_count,
                                BlendMode blend_type,
                                pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        FXARGB_SetDIB(dest_scan, ArgbEncode(src_alpha, src_r, src_g, src_b));
        dest_scan += 4;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        RGB_Blend(blend_type, scan, dest_scan, blended_colors);
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[0], alpha_ratio);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[1], alpha_ratio);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[2], alpha_ratio);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, *dest_scan, src_b);
        blended = FXDIB_ALPHA_MERGE(src_b, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_g);
        blended = FXDIB_ALPHA_MERGE(src_g, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_r);
        blended = FXDIB_ALPHA_MERGE(src_r, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
      } else {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, alpha_ratio);
      }
      dest_scan += 2;
    }
  });
}

void CompositeRow_ByteMask2Rgb(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               int mask_alpha,
                               int src_r,
                               int src_g,
                               int src_b,
                               int pixel_count,
                               BlendMode blend_type,
                               int Bpp,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
      if (src_alpha == 0) {
        dest_scan += Bpp;
        continue;
      }
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        RGB_Blend(blend_type, scan, dest_scan, blended_colors);
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[0], src_alpha);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[1], src_alpha);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[2], src_alpha);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, *dest_scan, src_b);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_g);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_r);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
      } else {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, src_alpha);
      }
      dest_scan += Bpp - 2;
    }
  });
}

void CompositeRow_ByteMask2Mask(pdfium::span<uint8_t> dest_span,
                                pdfium::span<const uint8_t> src_span,
                                int mask_alpha,
                                int pixel_count,
                                pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  for (int col = 0; col < pixel_count; col++) {
    int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
    uint8_t back_alpha = *dest_scan;
    if (!back_alpha) {
      *dest_scan = src_alpha;
    } else if (src_alpha) {
      *dest_scan = back_alpha + src_alpha - back_alpha * src_alpha / 255;
    }
    UNSAFE_TODO(dest_scan++);
  }
}

void CompositeRow_ByteMask2Gray(pdfium::span<uint8_t> dest_span,
                                pdfium::span<const uint8_t> src_span,
                                int mask_alpha,
                                int src_gray,
                                int pixel_count,
                                pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  for (int col = 0; col < pixel_count; col++) {
    int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
    if (src_alpha) {
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_gray, src_alpha);
    }
    UNSAFE_TODO(dest_scan++);
  }
}

void CompositeRow_BitMask2Argb(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               int mask_alpha,
                               int src_r,
                               int src_g,
                               int src_b,
                               int src_left,
                               int pixel_count,
                               BlendMode blend_type,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    if (blend_type == BlendMode::kNormal && !clip_scan && mask_alpha == 255) {
      FX_ARGB argb = ArgbEncode(0xff, src_r, src_g, src_b);
      for (int col = 0; col < pixel_count; col++) {
        if (src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8))) {
          FXARGB_SetDIB(dest_scan, argb);
        }
        dest_scan += 4;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan += 4;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        FXARGB_SetDIB(dest_scan, ArgbEncode(src_alpha, src_r, src_g, src_b));
        dest_scan += 4;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        RGB_Blend(blend_type, scan, dest_scan, blended_colors);
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[0], alpha_ratio);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[1], alpha_ratio);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[2], alpha_ratio);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, *dest_scan, src_b);
        blended = FXDIB_ALPHA_MERGE(src_b, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_g);
        blended = FXDIB_ALPHA_MERGE(src_g, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_r);
        blended = FXDIB_ALPHA_MERGE(src_r, blended, back_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, alpha_ratio);
      } else {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, alpha_ratio);
      }
      dest_scan += 2;
    }
  });
}

void CompositeRow_BitMask2Rgb(pdfium::span<uint8_t> dest_span,
                              pdfium::span<const uint8_t> src_span,
                              int mask_alpha,
                              int src_r,
                              int src_g,
                              int src_b,
                              int src_left,
                              int pixel_count,
                              BlendMode blend_type,
                              int Bpp,
                              pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    if (blend_type == BlendMode::kNormal && !clip_scan && mask_alpha == 255) {
      for (int col = 0; col < pixel_count; col++) {
        if (src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8))) {
          dest_scan[2] = src_r;
          dest_scan[1] = src_g;
          dest_scan[0] = src_b;
        }
        dest_scan += Bpp;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan += Bpp;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      if (src_alpha == 0) {
        dest_scan += Bpp;
        continue;
      }
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        RGB_Blend(blend_type, scan, dest_scan, blended_colors);
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[0], src_alpha);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[1], src_alpha);
        dest_scan++;
        *dest_scan =
            FXDIB_ALPHA_MERGE(*dest_scan, blended_colors[2], src_alpha);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, *dest_scan, src_b);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_g);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
        dest_scan++;
        blended = Blend(blend_type, *dest_scan, src_r);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, blended, src_alpha);
      } else {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_b, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_g, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_r, src_alpha);
      }
      dest_scan += Bpp - 2;
    }
  });
}

void CompositeRow_BitMask2Mask(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               int mask_alpha,
                               int src_left,
                               int pixel_count,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan++;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      uint8_t back_alpha = *dest_scan;
      if (!back_alpha) {
        *dest_scan = src_alpha;
      } else if (src_alpha) {
        *dest_scan = back_alpha + src_alpha - back_alpha * src_alpha / 255;
      }
      dest_scan++;
    }
  });
}

void CompositeRow_BitMask2Gray(pdfium::span<uint8_t> dest_span,
                               pdfium::span<const uint8_t> src_span,
                               int mask_alpha,
                               int src_gray,
                               int src_left,
                               int pixel_count,
                               pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan++;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      if (src_alpha) {
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, src_gray, src_alpha);
      }
      dest_scan++;
    }
  });
}

void CompositeRow_Argb2Argb_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int pixel_count,
    BlendMode blend_type,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int blended_colors[3];
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        if (clip_scan) {
          int src_alpha = clip_scan[col] * src_scan[3] / 255;
          ReverseCopy3Bytes(dest_scan, src_scan);
          dest_scan[3] = src_alpha;
        } else {
          FXARGB_RGBORDERCOPY(dest_scan, src_scan);
        }
        dest_scan += 4;
        src_scan += 4;
        continue;
      }
      uint8_t src_alpha = GetAlpha(src_scan[3], clip_scan, col);
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan += 4;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        if (blend_type != BlendMode::kNormal) {
          int blended = bNonseparableBlend
                            ? blended_colors[color]
                            : Blend(blend_type, dest_scan[index], *src_scan);
          blended = FXDIB_ALPHA_MERGE(*src_scan, blended, back_alpha);
          dest_scan[index] =
              FXDIB_ALPHA_MERGE(dest_scan[index], blended, alpha_ratio);
        } else {
          dest_scan[index] =
              FXDIB_ALPHA_MERGE(dest_scan[index], *src_scan, alpha_ratio);
        }
        src_scan++;
      }
      dest_scan += 4;
      src_scan++;
    }
  });
}

void CompositeRow_Rgb2Argb_Blend_NoClip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    BlendMode blend_type,
    int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int src_gap = src_Bpp - 3;
  int blended_colors[3];
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        if (src_Bpp == 4) {
          FXARGB_SetRGBOrderDIB(dest_scan,
                                0xff000000 | FXARGB_GetDIB(src_scan));
        } else {
          FXARGB_SetRGBOrderDIB(
              dest_scan,
              ArgbEncode(0xff, src_scan[2], src_scan[1], src_scan[0]));
        }
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      dest_scan[3] = 0xff;
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, dest_scan[index], src_color);
        dest_scan[index] = FXDIB_ALPHA_MERGE(src_color, blended, back_alpha);
        src_scan++;
      }
      dest_scan += 4;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Argb2Rgb_Blend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    BlendMode blend_type,
    int dest_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha;
      if (clip_scan) {
        src_alpha = src_scan[3] * (*clip_scan++) / 255;
      } else {
        src_alpha = src_scan[3];
      }
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        int back_color = dest_scan[index];
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, *src_scan);
        dest_scan[index] = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        src_scan++;
      }
      dest_scan += dest_Bpp;
      src_scan++;
    }
  });
}

void CompositeRow_Rgb2Argb_NoBlend_NoClip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      if (src_Bpp == 4) {
        FXARGB_SetRGBOrderDIB(dest_scan, 0xff000000 | FXARGB_GetDIB(src_scan));
      } else {
        FXARGB_SetRGBOrderDIB(
            dest_scan, ArgbEncode(0xff, src_scan[2], src_scan[1], src_scan[0]));
      }
      dest_scan += 4;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_Rgb2Rgb_Blend_NoClip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    BlendMode blend_type,
    int dest_Bpp,
    int src_Bpp) {
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        int back_color = dest_scan[index];
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, src_color);
        dest_scan[index] = blended;
        src_scan++;
      }
      dest_scan += dest_Bpp;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Argb2Rgb_NoBlend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    int dest_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha;
      if (clip_scan) {
        src_alpha = src_scan[3] * (*clip_scan++) / 255;
      } else {
        src_alpha = src_scan[3];
      }
      if (src_alpha == 255) {
        ReverseCopy3Bytes(dest_scan, src_scan);
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += 4;
        continue;
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        dest_scan[index] =
            FXDIB_ALPHA_MERGE(dest_scan[index], *src_scan, src_alpha);
        src_scan++;
      }
      dest_scan += dest_Bpp;
      src_scan++;
    }
  });
}

void CompositeRow_Rgb2Rgb_NoBlend_NoClip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    int dest_Bpp,
    int src_Bpp) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      ReverseCopy3Bytes(dest_scan, src_scan);
      dest_scan += dest_Bpp;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_Rgb2Argb_Blend_Clip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    BlendMode blend_type,
    int src_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_alpha = *clip_scan++;
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        ReverseCopy3Bytes(dest_scan, src_scan);
        src_scan += src_Bpp;
        dest_scan += 4;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        int src_color = *src_scan;
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, dest_scan[index], src_color);
        blended = FXDIB_ALPHA_MERGE(src_color, blended, back_alpha);
        dest_scan[index] =
            FXDIB_ALPHA_MERGE(dest_scan[index], blended, alpha_ratio);
        src_scan++;
      }
      dest_scan += 4;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Rgb_Blend_Clip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    BlendMode blend_type,
    int dest_Bpp,
    int src_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int blended_colors[3];
  bool bNonseparableBlend = IsNonSeparableBlendMode(blend_type);
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      uint8_t src_alpha = *clip_scan++;
      if (src_alpha == 0) {
        dest_scan += dest_Bpp;
        src_scan += src_Bpp;
        continue;
      }
      if (bNonseparableBlend) {
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, src_scan, dest_scan_o, blended_colors);
      }
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        int src_color = *src_scan;
        int back_color = dest_scan[index];
        int blended = bNonseparableBlend
                          ? blended_colors[color]
                          : Blend(blend_type, back_color, src_color);
        dest_scan[index] = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        src_scan++;
      }
      dest_scan += dest_Bpp;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Argb_NoBlend_Clip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    int src_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int src_gap = src_Bpp - 3;
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_alpha = clip_scan[col];
      if (src_alpha == 255) {
        ReverseCopy3Bytes(dest_scan, src_scan);
        dest_scan[3] = 255;
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan += src_Bpp;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      for (int color = 0; color < 3; color++) {
        int index = 2 - color;
        dest_scan[index] =
            FXDIB_ALPHA_MERGE(dest_scan[index], *src_scan, alpha_ratio);
        src_scan++;
      }
      dest_scan += 4;
      src_scan += src_gap;
    }
  });
}

void CompositeRow_Rgb2Rgb_NoBlend_Clip_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    int dest_Bpp,
    int src_Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_alpha = clip_scan[col];
      if (src_alpha == 255) {
        ReverseCopy3Bytes(dest_scan, src_scan);
      } else if (src_alpha) {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], *src_scan, src_alpha);
        src_scan++;
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], *src_scan, src_alpha);
        src_scan++;
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], *src_scan, src_alpha);
        dest_scan += dest_Bpp;
        src_scan += src_Bpp - 2;
        continue;
      }
      dest_scan += dest_Bpp;
      src_scan += src_Bpp;
    }
  });
}

void CompositeRow_8bppRgb2Rgb_NoBlend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    const FX_ARGB* pPalette,
    int pixel_count,
    int DestBpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      FX_ARGB argb = pPalette ? pPalette[*src_scan]
                              : ArgbEncode(0, *src_scan, *src_scan, *src_scan);
      int src_r = FXARGB_R(argb);
      int src_g = FXARGB_G(argb);
      int src_b = FXARGB_B(argb);
      if (clip_scan && clip_scan[col] < 255) {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, clip_scan[col]);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, clip_scan[col]);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, clip_scan[col]);
      } else {
        dest_scan[2] = src_b;
        dest_scan[1] = src_g;
        dest_scan[0] = src_r;
      }
      dest_scan += DestBpp;
      src_scan++;
    }
  });
}

void CompositeRow_1bppRgb2Rgb_NoBlend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int src_left,
    pdfium::span<const FX_ARGB> src_palette,
    int pixel_count,
    int DestBpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int reset_r;
  int reset_g;
  int reset_b;
  int set_r;
  int set_g;
  int set_b;
  if (!src_palette.empty()) {
    reset_r = FXARGB_R(src_palette[0]);
    reset_g = FXARGB_G(src_palette[0]);
    reset_b = FXARGB_B(src_palette[0]);
    set_r = FXARGB_R(src_palette[1]);
    set_g = FXARGB_G(src_palette[1]);
    set_b = FXARGB_B(src_palette[1]);
  } else {
    reset_r = reset_g = reset_b = 0;
    set_r = set_g = set_b = 255;
  }
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_r;
      int src_g;
      int src_b;
      if (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8))) {
        src_r = set_r;
        src_g = set_g;
        src_b = set_b;
      } else {
        src_r = reset_r;
        src_g = reset_g;
        src_b = reset_b;
      }
      if (clip_scan && clip_scan[col] < 255) {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, clip_scan[col]);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, clip_scan[col]);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, clip_scan[col]);
      } else {
        dest_scan[2] = src_b;
        dest_scan[1] = src_g;
        dest_scan[0] = src_r;
      }
      dest_scan += DestBpp;
    }
  });
}

void CompositeRow_8bppRgb2Argb_NoBlend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int width,
    const FX_ARGB* pPalette,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_r;
      int src_g;
      int src_b;
      if (pPalette) {
        FX_ARGB argb = pPalette[*src_scan];
        src_r = FXARGB_R(argb);
        src_g = FXARGB_G(argb);
        src_b = FXARGB_B(argb);
      } else {
        src_r = src_g = src_b = *src_scan;
      }
      if (!clip_scan || clip_scan[col] == 255) {
        dest_scan[2] = src_b;
        dest_scan[1] = src_g;
        dest_scan[0] = src_r;
        dest_scan[3] = 255;
        src_scan++;
        dest_scan += 4;
        continue;
      }
      int src_alpha = clip_scan[col];
      if (src_alpha == 0) {
        dest_scan += 4;
        src_scan++;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, alpha_ratio);
      dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, alpha_ratio);
      dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, alpha_ratio);
      dest_scan += 4;
      src_scan++;
    }
  });
}

void CompositeRow_1bppRgb2Argb_NoBlend_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int src_left,
    int width,
    pdfium::span<const FX_ARGB> src_palette,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  int reset_r;
  int reset_g;
  int reset_b;
  int set_r;
  int set_g;
  int set_b;
  if (!src_palette.empty()) {
    reset_r = FXARGB_R(src_palette[0]);
    reset_g = FXARGB_G(src_palette[0]);
    reset_b = FXARGB_B(src_palette[0]);
    set_r = FXARGB_R(src_palette[1]);
    set_g = FXARGB_G(src_palette[1]);
    set_b = FXARGB_B(src_palette[1]);
  } else {
    reset_r = reset_g = reset_b = 0;
    set_r = set_g = set_b = 255;
  }
  UNSAFE_TODO({
    for (int col = 0; col < width; col++) {
      int src_r;
      int src_g;
      int src_b;
      if (src_scan[(col + src_left) / 8] & (1 << (7 - (col + src_left) % 8))) {
        src_r = set_r;
        src_g = set_g;
        src_b = set_b;
      } else {
        src_r = reset_r;
        src_g = reset_g;
        src_b = reset_b;
      }
      if (!clip_scan || clip_scan[col] == 255) {
        dest_scan[2] = src_b;
        dest_scan[1] = src_g;
        dest_scan[0] = src_r;
        dest_scan[3] = 255;
        dest_scan += 4;
        continue;
      }
      int src_alpha = clip_scan[col];
      if (src_alpha == 0) {
        dest_scan += 4;
        continue;
      }
      int back_alpha = dest_scan[3];
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, alpha_ratio);
      dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, alpha_ratio);
      dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, alpha_ratio);
      dest_scan += 4;
    }
  });
}

void CompositeRow_ByteMask2Argb_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int mask_alpha,
    int src_r,
    int src_g,
    int src_b,
    int pixel_count,
    BlendMode blend_type,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        FXARGB_SetRGBOrderDIB(dest_scan,
                              ArgbEncode(src_alpha, src_r, src_g, src_b));
        dest_scan += 4;
        continue;
      }
      if (src_alpha == 0) {
        dest_scan += 4;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, scan, dest_scan_o, blended_colors);
        dest_scan[2] =
            FXDIB_ALPHA_MERGE(dest_scan[2], blended_colors[0], alpha_ratio);
        dest_scan[1] =
            FXDIB_ALPHA_MERGE(dest_scan[1], blended_colors[1], alpha_ratio);
        dest_scan[0] =
            FXDIB_ALPHA_MERGE(dest_scan[0], blended_colors[2], alpha_ratio);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, dest_scan[2], src_b);
        blended = FXDIB_ALPHA_MERGE(src_b, blended, back_alpha);
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], blended, alpha_ratio);
        blended = Blend(blend_type, dest_scan[1], src_g);
        blended = FXDIB_ALPHA_MERGE(src_g, blended, back_alpha);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], blended, alpha_ratio);
        blended = Blend(blend_type, dest_scan[0], src_r);
        blended = FXDIB_ALPHA_MERGE(src_r, blended, back_alpha);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], blended, alpha_ratio);
      } else {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, alpha_ratio);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, alpha_ratio);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, alpha_ratio);
      }
      dest_scan += 4;
    }
  });
}

void CompositeRow_ByteMask2Rgb_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int mask_alpha,
    int src_r,
    int src_g,
    int src_b,
    int pixel_count,
    BlendMode blend_type,
    int Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    for (int col = 0; col < pixel_count; col++) {
      int src_alpha = GetAlphaWithSrc(mask_alpha, clip_scan, src_scan, col);
      if (src_alpha == 0) {
        dest_scan += Bpp;
        continue;
      }
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, scan, dest_scan_o, blended_colors);
        dest_scan[2] =
            FXDIB_ALPHA_MERGE(dest_scan[2], blended_colors[0], src_alpha);
        dest_scan[1] =
            FXDIB_ALPHA_MERGE(dest_scan[1], blended_colors[1], src_alpha);
        dest_scan[0] =
            FXDIB_ALPHA_MERGE(dest_scan[0], blended_colors[2], src_alpha);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, dest_scan[2], src_b);
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], blended, src_alpha);
        blended = Blend(blend_type, dest_scan[1], src_g);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], blended, src_alpha);
        blended = Blend(blend_type, dest_scan[0], src_r);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], blended, src_alpha);
      } else {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, src_alpha);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, src_alpha);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, src_alpha);
      }
      dest_scan += Bpp;
    }
  });
}

void CompositeRow_BitMask2Argb_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int mask_alpha,
    int src_r,
    int src_g,
    int src_b,
    int src_left,
    int pixel_count,
    BlendMode blend_type,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    if (blend_type == BlendMode::kNormal && !clip_scan && mask_alpha == 255) {
      FX_ARGB argb = ArgbEncode(0xff, src_r, src_g, src_b);
      for (int col = 0; col < pixel_count; col++) {
        if (src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8))) {
          FXARGB_SetRGBOrderDIB(dest_scan, argb);
        }
        dest_scan += 4;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan += 4;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      uint8_t back_alpha = dest_scan[3];
      if (back_alpha == 0) {
        FXARGB_SetRGBOrderDIB(dest_scan,
                              ArgbEncode(src_alpha, src_r, src_g, src_b));
        dest_scan += 4;
        continue;
      }
      uint8_t dest_alpha =
          back_alpha + src_alpha - back_alpha * src_alpha / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, scan, dest_scan_o, blended_colors);
        dest_scan[2] =
            FXDIB_ALPHA_MERGE(dest_scan[2], blended_colors[0], alpha_ratio);
        dest_scan[1] =
            FXDIB_ALPHA_MERGE(dest_scan[1], blended_colors[1], alpha_ratio);
        dest_scan[0] =
            FXDIB_ALPHA_MERGE(dest_scan[0], blended_colors[2], alpha_ratio);
      } else if (blend_type != BlendMode::kNormal) {
        int blended = Blend(blend_type, dest_scan[2], src_b);
        blended = FXDIB_ALPHA_MERGE(src_b, blended, back_alpha);
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], blended, alpha_ratio);
        blended = Blend(blend_type, dest_scan[1], src_g);
        blended = FXDIB_ALPHA_MERGE(src_g, blended, back_alpha);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], blended, alpha_ratio);
        blended = Blend(blend_type, dest_scan[0], src_r);
        blended = FXDIB_ALPHA_MERGE(src_r, blended, back_alpha);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], blended, alpha_ratio);
      } else {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, alpha_ratio);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, alpha_ratio);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, alpha_ratio);
      }
      dest_scan += 4;
    }
  });
}

void CompositeRow_BitMask2Rgb_RgbByteOrder(
    pdfium::span<uint8_t> dest_span,
    pdfium::span<const uint8_t> src_span,
    int mask_alpha,
    int src_r,
    int src_g,
    int src_b,
    int src_left,
    int pixel_count,
    BlendMode blend_type,
    int Bpp,
    pdfium::span<const uint8_t> clip_span) {
  uint8_t* dest_scan = dest_span.data();
  const uint8_t* src_scan = src_span.data();
  const uint8_t* clip_scan = clip_span.data();
  UNSAFE_TODO({
    if (blend_type == BlendMode::kNormal && !clip_scan && mask_alpha == 255) {
      for (int col = 0; col < pixel_count; col++) {
        if (src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8))) {
          dest_scan[2] = src_b;
          dest_scan[1] = src_g;
          dest_scan[0] = src_r;
        }
        dest_scan += Bpp;
      }
      return;
    }
    for (int col = 0; col < pixel_count; col++) {
      if (!(src_scan[(src_left + col) / 8] &
            (1 << (7 - (src_left + col) % 8)))) {
        dest_scan += Bpp;
        continue;
      }
      int src_alpha = GetAlpha(mask_alpha, clip_scan, col);
      if (src_alpha == 0) {
        dest_scan += Bpp;
        continue;
      }
      if (IsNonSeparableBlendMode(blend_type)) {
        int blended_colors[3];
        uint8_t scan[3] = {static_cast<uint8_t>(src_b),
                           static_cast<uint8_t>(src_g),
                           static_cast<uint8_t>(src_r)};
        uint8_t dest_scan_o[3];
        ReverseCopy3Bytes(dest_scan_o, dest_scan);
        RGB_Blend(blend_type, scan, dest_scan_o, blended_colors);
        dest_scan[2] =
            FXDIB_ALPHA_MERGE(dest_scan[2], blended_colors[0], src_alpha);
        dest_scan[1] =
            FXDIB_ALPHA_MERGE(dest_scan[1], blended_colors[1], src_alpha);
        dest_scan[0] =
            FXDIB_ALPHA_MERGE(dest_scan[0], blended_colors[2], src_alpha);
      } else if (blend_type != BlendMode::kNormal) {
        int back_color = dest_scan[2];
        int blended = Blend(blend_type, back_color, src_b);
        dest_scan[2] = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        back_color = dest_scan[1];
        blended = Blend(blend_type, back_color, src_g);
        dest_scan[1] = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
        back_color = dest_scan[0];
        blended = Blend(blend_type, back_color, src_r);
        dest_scan[0] = FXDIB_ALPHA_MERGE(back_color, blended, src_alpha);
      } else {
        dest_scan[2] = FXDIB_ALPHA_MERGE(dest_scan[2], src_b, src_alpha);
        dest_scan[1] = FXDIB_ALPHA_MERGE(dest_scan[1], src_g, src_alpha);
        dest_scan[0] = FXDIB_ALPHA_MERGE(dest_scan[0], src_r, src_alpha);
      }
      dest_scan += Bpp;
    }
  });
}

}  // namespace

CFX_ScanlineCompositor::CFX_ScanlineCompositor() = default;

CFX_ScanlineCompositor::~CFX_ScanlineCompositor() = default;

bool CFX_ScanlineCompositor::Init(FXDIB_Format dest_format,
                                  FXDIB_Format src_format,
                                  pdfium::span<const uint32_t> src_palette,
                                  uint32_t mask_color,
                                  BlendMode blend_type,
                                  bool bClip,
                                  bool bRgbByteOrder) {
  m_SrcFormat = src_format;
  m_DestFormat = dest_format;
  m_BlendType = blend_type;
  m_bRgbByteOrder = bRgbByteOrder;
  m_bClip = bClip;
  if (m_DestFormat == FXDIB_Format::k1bppMask ||
      m_DestFormat == FXDIB_Format::k1bppRgb) {
    return false;
  }

  if (m_bRgbByteOrder && (m_DestFormat == FXDIB_Format::k8bppMask ||
                          m_DestFormat == FXDIB_Format::k8bppRgb)) {
    return false;
  }

  if (m_SrcFormat == FXDIB_Format::k1bppMask ||
      m_SrcFormat == FXDIB_Format::k8bppMask) {
    InitSourceMask(mask_color);
    return true;
  }
  if ((m_SrcFormat == FXDIB_Format::k1bppRgb ||
       m_SrcFormat == FXDIB_Format::k8bppRgb) &&
      m_DestFormat != FXDIB_Format::k8bppMask) {
    InitSourcePalette(src_palette);
  }
  return true;
}

void CFX_ScanlineCompositor::InitSourceMask(uint32_t mask_color) {
  m_MaskAlpha = FXARGB_A(mask_color);
  m_MaskRed = FXARGB_R(mask_color);
  m_MaskGreen = FXARGB_G(mask_color);
  m_MaskBlue = FXARGB_B(mask_color);
  if (m_DestFormat == FXDIB_Format::k8bppMask)
    return;

  if (m_DestFormat == FXDIB_Format::k8bppRgb)
    m_MaskRed = FXRGB2GRAY(m_MaskRed, m_MaskGreen, m_MaskBlue);
}

void CFX_ScanlineCompositor::InitSourcePalette(
    pdfium::span<const uint32_t> src_palette) {
  DCHECK_NE(m_DestFormat, FXDIB_Format::k8bppMask);

  m_SrcPalette.Reset();
  const bool bIsDestBpp8 = m_DestFormat == FXDIB_Format::k8bppRgb;
  const size_t pal_count = static_cast<size_t>(1)
                           << GetBppFromFormat(m_SrcFormat);

  if (!src_palette.empty()) {
    if (bIsDestBpp8) {
      pdfium::span<uint8_t> gray_pal = m_SrcPalette.Make8BitPalette(pal_count);
      for (size_t i = 0; i < pal_count; ++i) {
        FX_ARGB argb = src_palette[i];
        gray_pal[i] =
            FXRGB2GRAY(FXARGB_R(argb), FXARGB_G(argb), FXARGB_B(argb));
      }
      return;
    }
    pdfium::span<uint32_t> pPalette = m_SrcPalette.Make32BitPalette(pal_count);
    fxcrt::spancpy(pPalette, src_palette.first(pal_count));
    return;
  }
  if (bIsDestBpp8) {
    pdfium::span<uint8_t> gray_pal = m_SrcPalette.Make8BitPalette(pal_count);
    if (pal_count == 2) {
      gray_pal[0] = 0;
      gray_pal[1] = 255;
    } else {
      for (size_t i = 0; i < pal_count; ++i)
        gray_pal[i] = i;
    }
    return;
  }
  pdfium::span<uint32_t> pPalette = m_SrcPalette.Make32BitPalette(pal_count);
  if (pal_count == 2) {
    pPalette[0] = 0xff000000;
    pPalette[1] = 0xffffffff;
  } else {
    for (size_t i = 0; i < pal_count; ++i) {
      uint32_t v = static_cast<uint32_t>(i);
      pPalette[i] = ArgbEncode(0, v, v, v);
    }
  }
}

void CFX_ScanlineCompositor::CompositeRgbBitmapLine(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan,
    int width,
    pdfium::span<const uint8_t> clip_scan) const {
  DCHECK(m_SrcFormat == FXDIB_Format::kRgb ||
         m_SrcFormat == FXDIB_Format::kRgb32 ||
         m_SrcFormat == FXDIB_Format::kArgb);

  int src_Bpp = GetCompsFromFormat(m_SrcFormat);
  int dest_Bpp = GetCompsFromFormat(m_DestFormat);
  if (m_bRgbByteOrder) {
    if (m_SrcFormat == FXDIB_Format::kArgb) {
      if (m_DestFormat == FXDIB_Format::kArgb) {
        CompositeRow_Argb2Argb_RgbByteOrder(dest_scan, src_scan, width,
                                            m_BlendType, clip_scan);
        return;
      }
      if (m_BlendType == BlendMode::kNormal) {
        CompositeRow_Argb2Rgb_NoBlend_RgbByteOrder(dest_scan, src_scan, width,
                                                   dest_Bpp, clip_scan);
        return;
      }
      CompositeRow_Argb2Rgb_Blend_RgbByteOrder(
          dest_scan, src_scan, width, m_BlendType, dest_Bpp, clip_scan);
      return;
    }

    if (m_DestFormat == FXDIB_Format::kArgb) {
      if (m_BlendType == BlendMode::kNormal) {
        if (m_bClip) {
          CompositeRow_Rgb2Argb_NoBlend_Clip_RgbByteOrder(
              dest_scan, src_scan, width, src_Bpp, clip_scan);
          return;
        }
        CompositeRow_Rgb2Argb_NoBlend_NoClip_RgbByteOrder(dest_scan, src_scan,
                                                          width, src_Bpp);
        return;
      }
      if (m_bClip) {
        CompositeRow_Rgb2Argb_Blend_Clip_RgbByteOrder(
            dest_scan, src_scan, width, m_BlendType, src_Bpp, clip_scan);
        return;
      }
      CompositeRow_Rgb2Argb_Blend_NoClip_RgbByteOrder(
          dest_scan, src_scan, width, m_BlendType, src_Bpp);
      return;
    }

    if (m_BlendType == BlendMode::kNormal) {
      if (m_bClip) {
        CompositeRow_Rgb2Rgb_NoBlend_Clip_RgbByteOrder(
            dest_scan, src_scan, width, dest_Bpp, src_Bpp, clip_scan);
        return;
      }
      CompositeRow_Rgb2Rgb_NoBlend_NoClip_RgbByteOrder(
          dest_scan, src_scan, width, dest_Bpp, src_Bpp);
      return;
    }
    if (m_bClip) {
      CompositeRow_Rgb2Rgb_Blend_Clip_RgbByteOrder(dest_scan, src_scan, width,
                                                   m_BlendType, dest_Bpp,
                                                   src_Bpp, clip_scan);
      return;
    }
    CompositeRow_Rgb2Rgb_Blend_NoClip_RgbByteOrder(
        dest_scan, src_scan, width, m_BlendType, dest_Bpp, src_Bpp);
    return;
  }

  if (m_DestFormat == FXDIB_Format::k8bppMask) {
    if (m_SrcFormat == FXDIB_Format::kArgb) {
      CompositeRow_AlphaToMask(dest_scan, src_scan, width, clip_scan, 4);
    } else {
      CompositeRow_Rgb2Mask(dest_scan, width, clip_scan);
    }
    return;
  }

  if (m_DestFormat == FXDIB_Format::k8bppRgb) {
    if (m_SrcFormat == FXDIB_Format::kArgb) {
      CompositeRow_Argb2Gray(dest_scan, src_scan, width, m_BlendType,
                             clip_scan);
    } else {
      CompositeRow_Rgb2Gray(dest_scan, src_scan, src_Bpp, width, m_BlendType,
                            clip_scan);
    }
    return;
  }

  // TODO(thestig): Tighten this check.
  DCHECK_NE(GetBppFromFormat(m_DestFormat), 8);

  if (m_SrcFormat == FXDIB_Format::kArgb) {
    if (m_DestFormat == FXDIB_Format::kArgb) {
      CompositeRow_Argb2Argb(dest_scan, src_scan, width, m_BlendType,
                             clip_scan);
      return;
    }
    if (m_BlendType == BlendMode::kNormal) {
      CompositeRow_Argb2Rgb_NoBlend(dest_scan, src_scan, width, dest_Bpp,
                                    clip_scan);
      return;
    }
    CompositeRow_Argb2Rgb_Blend(dest_scan, src_scan, width, m_BlendType,
                                dest_Bpp, clip_scan);
    return;
  }

  if (m_DestFormat == FXDIB_Format::kArgb) {
    if (m_BlendType == BlendMode::kNormal) {
      if (m_bClip) {
        CompositeRow_Rgb2Argb_NoBlend_Clip(dest_scan, src_scan, width, src_Bpp,
                                           clip_scan);
        return;
      }
      CompositeRow_Rgb2Argb_NoBlend_NoClip(dest_scan, src_scan, width, src_Bpp);
      return;
    }
    if (m_bClip) {
      CompositeRow_Rgb2Argb_Blend_Clip(dest_scan, src_scan, width, m_BlendType,
                                       src_Bpp, clip_scan);
      return;
    }
    CompositeRow_Rgb2Argb_Blend_NoClip(dest_scan, src_scan, width, m_BlendType,
                                       src_Bpp);
    return;
  }

  if (m_BlendType == BlendMode::kNormal) {
    if (m_bClip) {
      CompositeRow_Rgb2Rgb_NoBlend_Clip(dest_scan, src_scan, width, dest_Bpp,
                                        src_Bpp, clip_scan);
      return;
    }
    CompositeRow_Rgb2Rgb_NoBlend_NoClip(dest_scan, src_scan, width, dest_Bpp,
                                        src_Bpp);
    return;
  }
  if (m_bClip) {
    CompositeRow_Rgb2Rgb_Blend_Clip(dest_scan, src_scan, width, m_BlendType,
                                    dest_Bpp, src_Bpp, clip_scan);
    return;
  }
  CompositeRow_Rgb2Rgb_Blend_NoClip(dest_scan, src_scan, width, m_BlendType,
                                    dest_Bpp, src_Bpp);
}

void CFX_ScanlineCompositor::CompositePalBitmapLine(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan,
    int src_left,
    int width,
    pdfium::span<const uint8_t> clip_scan) const {
  DCHECK(m_SrcFormat == FXDIB_Format::k1bppRgb ||
         m_SrcFormat == FXDIB_Format::k8bppRgb);

  if (m_bRgbByteOrder) {
    if (m_SrcFormat == FXDIB_Format::k1bppRgb) {
      if (m_DestFormat == FXDIB_Format::k8bppRgb) {
        return;
      }
      if (m_DestFormat == FXDIB_Format::kArgb) {
        CompositeRow_1bppRgb2Argb_NoBlend_RgbByteOrder(
            dest_scan, src_scan, src_left, width,
            m_SrcPalette.Get32BitPalette(), clip_scan);
      } else {
        CompositeRow_1bppRgb2Rgb_NoBlend_RgbByteOrder(
            dest_scan, src_scan, src_left, m_SrcPalette.Get32BitPalette(),
            width, GetCompsFromFormat(m_DestFormat), clip_scan);
      }
    } else {
      if (m_DestFormat == FXDIB_Format::k8bppRgb) {
        return;
      }
      if (m_DestFormat == FXDIB_Format::kArgb) {
        CompositeRow_8bppRgb2Argb_NoBlend_RgbByteOrder(
            dest_scan, src_scan, width, m_SrcPalette.Get32BitPalette().data(),
            clip_scan);
      } else {
        CompositeRow_8bppRgb2Rgb_NoBlend_RgbByteOrder(
            dest_scan, src_scan, m_SrcPalette.Get32BitPalette().data(), width,
            GetCompsFromFormat(m_DestFormat), clip_scan);
      }
    }
    return;
  }

  if (m_DestFormat == FXDIB_Format::k8bppMask) {
    CompositeRow_Rgb2Mask(dest_scan, width, clip_scan);
    return;
  }

  if (m_DestFormat == FXDIB_Format::k8bppRgb) {
    if (m_SrcFormat == FXDIB_Format::k1bppRgb) {
      CompositeRow_1bppPal2Gray(dest_scan, src_scan, src_left,
                                m_SrcPalette.Get8BitPalette(), width,
                                m_BlendType, clip_scan);
      return;
    }
    CompositeRow_8bppPal2Gray(dest_scan, src_scan,
                              m_SrcPalette.Get8BitPalette(), width, m_BlendType,
                              clip_scan);
    return;
  }

  // TODO(thestig): Tighten this check.
  DCHECK_NE(GetBppFromFormat(m_DestFormat), 8);

  if (m_DestFormat == FXDIB_Format::kArgb) {
    if (m_SrcFormat == FXDIB_Format::k1bppRgb) {
      CompositeRow_1bppRgb2Argb_NoBlend(dest_scan, src_scan, src_left, width,
                                        m_SrcPalette.Get32BitPalette(),
                                        clip_scan);
      return;
    }
    CompositeRow_8bppRgb2Argb_NoBlend(
        dest_scan, src_scan, width, m_SrcPalette.Get32BitPalette(), clip_scan);
    return;
  }

  if (m_SrcFormat == FXDIB_Format::k8bppRgb) {
    CompositeRow_8bppRgb2Rgb_NoBlend(
        dest_scan, src_scan, m_SrcPalette.Get32BitPalette(), width,
        GetCompsFromFormat(m_DestFormat), clip_scan);
    return;
  }

  CompositeRow_1bppRgb2Rgb_NoBlend(dest_scan, src_scan, src_left,
                                   m_SrcPalette.Get32BitPalette(), width,
                                   GetCompsFromFormat(m_DestFormat), clip_scan);
}

void CFX_ScanlineCompositor::CompositeByteMaskLine(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan,
    int width,
    pdfium::span<const uint8_t> clip_scan) const {
  if (m_DestFormat == FXDIB_Format::k8bppMask) {
    CompositeRow_ByteMask2Mask(dest_scan, src_scan, m_MaskAlpha, width,
                               clip_scan);
    return;
  }
  if (m_DestFormat == FXDIB_Format::k8bppRgb) {
    CompositeRow_ByteMask2Gray(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                               width, clip_scan);
    return;
  }

  // TODO(thestig): Tighten this check.
  DCHECK_NE(GetBppFromFormat(m_DestFormat), 8);

  if (m_bRgbByteOrder) {
    if (m_DestFormat == FXDIB_Format::kArgb) {
      CompositeRow_ByteMask2Argb_RgbByteOrder(
          dest_scan, src_scan, m_MaskAlpha, m_MaskRed, m_MaskGreen, m_MaskBlue,
          width, m_BlendType, clip_scan);
    } else {
      CompositeRow_ByteMask2Rgb_RgbByteOrder(
          dest_scan, src_scan, m_MaskAlpha, m_MaskRed, m_MaskGreen, m_MaskBlue,
          width, m_BlendType, GetCompsFromFormat(m_DestFormat), clip_scan);
    }
    return;
  }

  if (m_DestFormat == FXDIB_Format::kArgb) {
    CompositeRow_ByteMask2Argb(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                               m_MaskGreen, m_MaskBlue, width, m_BlendType,
                               clip_scan);
    return;
  }

  if (m_DestFormat == FXDIB_Format::kRgb ||
      m_DestFormat == FXDIB_Format::kRgb32) {
    CompositeRow_ByteMask2Rgb(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                              m_MaskGreen, m_MaskBlue, width, m_BlendType,
                              GetCompsFromFormat(m_DestFormat), clip_scan);
    return;
  }

  // TODO(thestig): Is this line reachable?
}

void CFX_ScanlineCompositor::CompositeBitMaskLine(
    pdfium::span<uint8_t> dest_scan,
    pdfium::span<const uint8_t> src_scan,
    int src_left,
    int width,
    pdfium::span<const uint8_t> clip_scan) const {
  if (m_DestFormat == FXDIB_Format::k8bppMask) {
    CompositeRow_BitMask2Mask(dest_scan, src_scan, m_MaskAlpha, src_left, width,
                              clip_scan);
    return;
  }

  if (m_DestFormat == FXDIB_Format::k8bppRgb) {
    CompositeRow_BitMask2Gray(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                              src_left, width, clip_scan);
    return;
  }

  // TODO(thestig): Tighten this check.
  DCHECK_NE(GetBppFromFormat(m_DestFormat), 8);

  if (m_bRgbByteOrder) {
    if (m_DestFormat == FXDIB_Format::kArgb) {
      CompositeRow_BitMask2Argb_RgbByteOrder(
          dest_scan, src_scan, m_MaskAlpha, m_MaskRed, m_MaskGreen, m_MaskBlue,
          src_left, width, m_BlendType, clip_scan);
    } else {
      CompositeRow_BitMask2Rgb_RgbByteOrder(
          dest_scan, src_scan, m_MaskAlpha, m_MaskRed, m_MaskGreen, m_MaskBlue,
          src_left, width, m_BlendType, GetCompsFromFormat(m_DestFormat),
          clip_scan);
    }
    return;
  }

  if (m_DestFormat == FXDIB_Format::kArgb) {
    CompositeRow_BitMask2Argb(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                              m_MaskGreen, m_MaskBlue, src_left, width,
                              m_BlendType, clip_scan);
    return;
  }

  if (m_DestFormat == FXDIB_Format::kRgb ||
      m_DestFormat == FXDIB_Format::kRgb32) {
    CompositeRow_BitMask2Rgb(dest_scan, src_scan, m_MaskAlpha, m_MaskRed,
                             m_MaskGreen, m_MaskBlue, src_left, width,
                             m_BlendType, GetCompsFromFormat(m_DestFormat),
                             clip_scan);
    return;
  }

  // TODO(thestig): Is this line reachable?
}

CFX_ScanlineCompositor::Palette::Palette() = default;

CFX_ScanlineCompositor::Palette::~Palette() = default;

void CFX_ScanlineCompositor::Palette::Reset() {
  m_Width = 0;
  m_nElements = 0;
  m_pData.reset();
}

pdfium::span<uint8_t> CFX_ScanlineCompositor::Palette::Make8BitPalette(
    size_t nElements) {
  m_Width = sizeof(uint8_t);
  m_nElements = nElements;
  m_pData.reset(reinterpret_cast<uint32_t*>(FX_Alloc(uint8_t, m_nElements)));
  // SAFETY: `m_nElements` passed to FX_Alloc() of type uint8_t.
  return UNSAFE_BUFFERS(pdfium::make_span(
      reinterpret_cast<uint8_t*>(m_pData.get()), m_nElements));
}

pdfium::span<uint32_t> CFX_ScanlineCompositor::Palette::Make32BitPalette(
    size_t nElements) {
  m_Width = sizeof(uint32_t);
  m_nElements = nElements;
  m_pData.reset(FX_Alloc(uint32_t, m_nElements));
  // SAFETY: `m_nElements` passed to FX_Alloc() of type uint32_t.
  return UNSAFE_BUFFERS(pdfium::make_span(m_pData.get(), m_nElements));
}

pdfium::span<const uint8_t> CFX_ScanlineCompositor::Palette::Get8BitPalette()
    const {
  CHECK(!m_pData || m_Width == sizeof(uint8_t));
  // SAFETY: `m_Width` only set to sizeof(uint8_t) just prior to passing
  // `m_nElements` to FX_Alloc() of type uint8_t.
  return UNSAFE_BUFFERS(pdfium::make_span(
      reinterpret_cast<const uint8_t*>(m_pData.get()), m_nElements));
}

pdfium::span<const uint32_t> CFX_ScanlineCompositor::Palette::Get32BitPalette()
    const {
  CHECK(!m_pData || m_Width == sizeof(uint32_t));
  // SAFETY: `m_Width` only set to sizeof(uint32_t) just prior to passing
  // `m_nElements` to FX_Alloc() of type uint32_t.
  return UNSAFE_BUFFERS(pdfium::make_span(m_pData.get(), m_nElements));
}
