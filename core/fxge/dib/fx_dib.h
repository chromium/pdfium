// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_DIB_FX_DIB_H_
#define CORE_FXGE_DIB_FX_DIB_H_

#include <stdint.h>

#include <tuple>
#include <utility>

// Encoding:
// - Bits-per-pixel: value & 0xFF
// - Is mask: value & 0x100
// - Has alpha: value & 0x200
enum class FXDIB_Format : uint16_t {
  kInvalid = 0,
  k1bppRgb = 0x001,
  k8bppRgb = 0x008,
  kRgb = 0x018,
  kRgb32 = 0x020,
  k1bppMask = 0x101,
  k8bppMask = 0x108,
  kArgb = 0x220,
};

using FX_ARGB = uint32_t;
using FX_CMYK = uint32_t;

// FX_COLORREF, like win32 COLORREF, is BGR.
using FX_COLORREF = uint32_t;

struct FXDIB_ResampleOptions {
  FXDIB_ResampleOptions();

  bool HasAnyOptions() const;

  bool bInterpolateBilinear = false;
  bool bHalftone = false;
  bool bNoSmoothing = false;
  bool bLossy = false;
};

// See PDF 1.7 spec, table 7.2 and 7.3. The enum values need to be in the same
// order as listed in the spec.
enum class BlendMode {
  kNormal = 0,
  kMultiply,
  kScreen,
  kOverlay,
  kDarken,
  kLighten,
  kColorDodge,
  kColorBurn,
  kHardLight,
  kSoftLight,
  kDifference,
  kExclusion,
  kHue,
  kSaturation,
  kColor,
  kLuminosity,
  kLast = kLuminosity,
};

constexpr uint32_t FXSYS_BGR(uint8_t b, uint8_t g, uint8_t r) {
  return (b << 16) | (g << 8) | r;
}

constexpr uint8_t FXSYS_GetRValue(uint32_t bgr) {
  return bgr & 0xff;
}

constexpr uint8_t FXSYS_GetGValue(uint32_t bgr) {
  return (bgr >> 8) & 0xff;
}

constexpr uint8_t FXSYS_GetBValue(uint32_t bgr) {
  return (bgr >> 16) & 0xff;
}

constexpr unsigned int FXSYS_GetUnsignedAlpha(float alpha) {
  return static_cast<unsigned int>(alpha * 255.f + 0.5f);
}

// Bits per pixel, not bytes.
inline int GetBppFromFormat(FXDIB_Format format) {
  return static_cast<uint16_t>(format) & 0xff;
}

// AKA bytes per pixel, assuming 8-bits per component.
inline int GetCompsFromFormat(FXDIB_Format format) {
  return (static_cast<uint16_t>(format) & 0xff) / 8;
}

inline bool GetIsMaskFromFormat(FXDIB_Format format) {
  return !!(static_cast<uint16_t>(format) & 0x100);
}

FXDIB_Format MakeRGBFormat(int bpp);

constexpr FX_CMYK CmykEncode(uint32_t c, uint32_t m, uint32_t y, uint32_t k) {
  return (c << 24) | (m << 16) | (y << 8) | k;
}

// Returns (a, r, g, b)
std::tuple<int, int, int, int> ArgbDecode(FX_ARGB argb);

// Returns (a, FX_COLORREF)
std::pair<int, FX_COLORREF> ArgbToAlphaAndColorRef(FX_ARGB argb);

// Returns FX_COLORREF.
FX_COLORREF ArgbToColorRef(FX_ARGB argb);

constexpr FX_ARGB ArgbEncode(uint32_t a, uint32_t r, uint32_t g, uint32_t b) {
  return (a << 24) | (r << 16) | (g << 8) | b;
}

FX_ARGB AlphaAndColorRefToArgb(int a, FX_COLORREF colorref);

#define FXARGB_A(argb) ((uint8_t)((argb) >> 24))
#define FXARGB_R(argb) ((uint8_t)((argb) >> 16))
#define FXARGB_G(argb) ((uint8_t)((argb) >> 8))
#define FXARGB_B(argb) ((uint8_t)(argb))
#define FXARGB_MUL_ALPHA(argb, alpha) \
  (((((argb) >> 24) * (alpha) / 255) << 24) | ((argb)&0xffffff))

#define FXRGB2GRAY(r, g, b) (((b)*11 + (g)*59 + (r)*30) / 100)
#define FXDIB_ALPHA_MERGE(backdrop, source, source_alpha) \
  (((backdrop) * (255 - (source_alpha)) + (source) * (source_alpha)) / 255)
#define FXARGB_GETDIB(p)                              \
  ((((uint8_t*)(p))[0]) | (((uint8_t*)(p))[1] << 8) | \
   (((uint8_t*)(p))[2] << 16) | (((uint8_t*)(p))[3] << 24))
#define FXARGB_SETDIB(p, argb)                  \
  ((uint8_t*)(p))[0] = (uint8_t)(argb),         \
  ((uint8_t*)(p))[1] = (uint8_t)((argb) >> 8),  \
  ((uint8_t*)(p))[2] = (uint8_t)((argb) >> 16), \
  ((uint8_t*)(p))[3] = (uint8_t)((argb) >> 24)
#define FXARGB_SETRGBORDERDIB(p, argb)          \
  ((uint8_t*)(p))[3] = (uint8_t)(argb >> 24),   \
  ((uint8_t*)(p))[0] = (uint8_t)((argb) >> 16), \
  ((uint8_t*)(p))[1] = (uint8_t)((argb) >> 8),  \
  ((uint8_t*)(p))[2] = (uint8_t)(argb)
#define FXCMYK_TODIB(cmyk)                                    \
  ((uint8_t)((cmyk) >> 24) | ((uint8_t)((cmyk) >> 16)) << 8 | \
   ((uint8_t)((cmyk) >> 8)) << 16 | ((uint8_t)(cmyk) << 24))
#define FXARGB_TOBGRORDERDIB(argb)                       \
  ((uint8_t)(argb >> 16) | ((uint8_t)(argb >> 8)) << 8 | \
   ((uint8_t)(argb)) << 16 | ((uint8_t)(argb >> 24) << 24))

inline void ReverseCopy3Bytes(uint8_t* dest, const uint8_t* src) {
  dest[2] = src[0];
  dest[1] = src[1];
  dest[0] = src[2];
}

#endif  // CORE_FXGE_DIB_FX_DIB_H_
