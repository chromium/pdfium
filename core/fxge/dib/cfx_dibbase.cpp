// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_dibbase.h"

#include <string.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_cliprgn.h"
#include "core/fxge/dib/cfx_bitmapstorer.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/dib/cfx_imagetransformer.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"

namespace {

void ColorDecode(uint32_t pal_v, uint8_t* r, uint8_t* g, uint8_t* b) {
  *r = static_cast<uint8_t>((pal_v & 0xf00) >> 4);
  *g = static_cast<uint8_t>(pal_v & 0x0f0);
  *b = static_cast<uint8_t>((pal_v & 0x00f) << 4);
}

void Obtain_Pal(std::pair<uint32_t, uint32_t>* luts,
                uint32_t* dest_pal,
                uint32_t lut) {
  uint32_t lut_1 = lut - 1;
  for (int row = 0; row < 256; ++row) {
    int lut_offset = lut_1 - row;
    if (lut_offset < 0)
      lut_offset += 256;
    uint32_t color = luts[lut_offset].second;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    ColorDecode(color, &r, &g, &b);
    dest_pal[row] = (static_cast<uint32_t>(r) << 16) |
                    (static_cast<uint32_t>(g) << 8) | b | 0xff000000;
    luts[lut_offset].first = row;
  }
}

class CFX_Palette {
 public:
  explicit CFX_Palette(const RetainPtr<CFX_DIBBase>& pBitmap);
  ~CFX_Palette();

  const uint32_t* GetPalette() { return m_Palette.data(); }
  const std::pair<uint32_t, uint32_t>* GetLuts() const { return m_Luts.data(); }
  int32_t GetLutCount() const { return m_lut; }
  void SetAmountLut(int row, uint32_t value) { m_Luts[row].first = value; }

 private:
  std::vector<uint32_t> m_Palette;
  // (Amount, Color) pairs
  std::vector<std::pair<uint32_t, uint32_t>> m_Luts;
  int m_lut = 0;
};

CFX_Palette::CFX_Palette(const RetainPtr<CFX_DIBBase>& pBitmap)
    : m_Palette(256), m_Luts(4096) {
  int bpp = pBitmap->GetBPP() / 8;
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  for (int row = 0; row < height; ++row) {
    const uint8_t* scan_line = pBitmap->GetScanline(row);
    for (int col = 0; col < width; ++col) {
      const uint8_t* src_port = scan_line + col * bpp;
      uint32_t b = src_port[0] & 0xf0;
      uint32_t g = src_port[1] & 0xf0;
      uint32_t r = src_port[2] & 0xf0;
      uint32_t index = (r << 4) + g + (b >> 4);
      ++m_Luts[index].first;
    }
  }
  // Move non-zeros to the front and count them
  for (int row = 0; row < 4096; ++row) {
    if (m_Luts[row].first != 0) {
      m_Luts[m_lut].first = m_Luts[row].first;
      m_Luts[m_lut].second = row;
      ++m_lut;
    }
  }
  std::sort(m_Luts.begin(), m_Luts.begin() + m_lut,
            [](const std::pair<uint32_t, uint32_t>& arg1,
               const std::pair<uint32_t, uint32_t>& arg2) {
              return arg1.first < arg2.first;
            });
  Obtain_Pal(m_Luts.data(), m_Palette.data(), m_lut);
}

CFX_Palette::~CFX_Palette() = default;

void ConvertBuffer_1bppMask2Gray(uint8_t* dest_buf,
                                 int dest_pitch,
                                 int width,
                                 int height,
                                 const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                 int src_left,
                                 int src_top) {
  static constexpr uint8_t kSetGray = 0xff;
  static constexpr uint8_t kResetGray = 0x00;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    memset(dest_scan, kResetGray, width);
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
    for (int col = src_left; col < src_left + width; ++col) {
      if (src_scan[col / 8] & (1 << (7 - col % 8)))
        *dest_scan = kSetGray;
      ++dest_scan;
    }
  }
}

void ConvertBuffer_8bppMask2Gray(uint8_t* dest_buf,
                                 int dest_pitch,
                                 int width,
                                 int height,
                                 const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                 int src_left,
                                 int src_top) {
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
    memcpy(dest_scan, src_scan, width);
  }
}

void ConvertBuffer_1bppPlt2Gray(uint8_t* dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint8_t reset_r = FXARGB_R(src_palette[0]);
  uint8_t reset_g = FXARGB_G(src_palette[0]);
  uint8_t reset_b = FXARGB_B(src_palette[0]);
  uint8_t set_r = FXARGB_R(src_palette[1]);
  uint8_t set_g = FXARGB_G(src_palette[1]);
  uint8_t set_b = FXARGB_B(src_palette[1]);
  uint8_t gray[2];
  gray[0] = FXRGB2GRAY(reset_r, reset_g, reset_b);
  gray[1] = FXRGB2GRAY(set_r, set_g, set_b);

  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    memset(dest_scan, gray[0], width);
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
    for (int col = src_left; col < src_left + width; ++col) {
      if (src_scan[col / 8] & (1 << (7 - col % 8)))
        *dest_scan = gray[1];
      ++dest_scan;
    }
  }
}

void ConvertBuffer_8bppPlt2Gray(uint8_t* dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint8_t gray[256];
  for (size_t i = 0; i < pdfium::size(gray); ++i) {
    gray[i] = FXRGB2GRAY(FXARGB_R(src_palette[i]), FXARGB_G(src_palette[i]),
                         FXARGB_B(src_palette[i]));
  }

  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
    for (int col = 0; col < width; ++col)
      *dest_scan++ = gray[*src_scan++];
  }
}

void ConvertBuffer_Rgb2Gray(uint8_t* dest_buf,
                            int dest_pitch,
                            int width,
                            int height,
                            const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                            int src_left,
                            int src_top) {
  int Bpp = pSrcBitmap->GetBPP() / 8;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
    for (int col = 0; col < width; ++col) {
      *dest_scan++ = FXRGB2GRAY(src_scan[2], src_scan[1], src_scan[0]);
      src_scan += Bpp;
    }
  }
}

void ConvertBuffer_IndexCopy(uint8_t* dest_buf,
                             int dest_pitch,
                             int width,
                             int height,
                             const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                             int src_left,
                             int src_top) {
  if (pSrcBitmap->GetBPP() == 1) {
    for (int row = 0; row < height; ++row) {
      uint8_t* dest_scan = dest_buf + row * dest_pitch;
      // Set all destination pixels to be white initially.
      memset(dest_scan, 255, width);
      const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
      for (int col = src_left; col < src_left + width; ++col) {
        // If the source bit is set, then set the destination pixel to be black.
        if (src_scan[col / 8] & (1 << (7 - col % 8)))
          *dest_scan = 0;

        ++dest_scan;
      }
    }
  } else {
    for (int row = 0; row < height; ++row) {
      uint8_t* dest_scan = dest_buf + row * dest_pitch;
      const uint8_t* src_scan =
          pSrcBitmap->GetScanline(src_top + row) + src_left;
      memcpy(dest_scan, src_scan, width);
    }
  }
}

void ConvertBuffer_Plt2PltRgb8(uint8_t* dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top,
                               pdfium::span<uint32_t> dst_plt) {
  ConvertBuffer_IndexCopy(dest_buf, dest_pitch, width, height, pSrcBitmap,
                          src_left, src_top);
  const size_t plt_size = pSrcBitmap->GetRequiredPaletteSize();
  pdfium::span<const uint32_t> src_span = pSrcBitmap->GetPaletteSpan();
  CHECK(plt_size <= src_span.size());

  const uint32_t* src_plt = src_span.data();
  for (size_t i = 0; i < plt_size; ++i)
    dst_plt[i] = src_plt[i];
}

void ConvertBuffer_Rgb2PltRgb8(uint8_t* dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top,
                               pdfium::span<uint32_t> dst_plt) {
  int bpp = pSrcBitmap->GetBPP() / 8;
  CFX_Palette palette(pSrcBitmap);
  const std::pair<uint32_t, uint32_t>* Luts = palette.GetLuts();
  int lut = palette.GetLutCount();
  const uint32_t* pal = palette.GetPalette();
  if (lut > 256) {
    int err;
    int min_err;
    int lut_256 = lut - 256;
    for (int row = 0; row < lut_256; ++row) {
      min_err = 1000000;
      uint8_t r;
      uint8_t g;
      uint8_t b;
      ColorDecode(Luts[row].second, &r, &g, &b);
      uint32_t clrindex = 0;
      for (int col = 0; col < 256; ++col) {
        uint32_t p_color = pal[col];
        int d_r = r - static_cast<uint8_t>(p_color >> 16);
        int d_g = g - static_cast<uint8_t>(p_color >> 8);
        int d_b = b - static_cast<uint8_t>(p_color);
        err = d_r * d_r + d_g * d_g + d_b * d_b;
        if (err < min_err) {
          min_err = err;
          clrindex = col;
        }
      }
      palette.SetAmountLut(row, clrindex);
    }
  }
  int32_t lut_1 = lut - 1;
  for (int row = 0; row < height; ++row) {
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    for (int col = 0; col < width; ++col) {
      const uint8_t* src_port = src_scan + col * bpp;
      int r = src_port[2] & 0xf0;
      int g = src_port[1] & 0xf0;
      int b = src_port[0] & 0xf0;
      uint32_t clrindex = (r << 4) + g + (b >> 4);
      for (int i = lut_1; i >= 0; --i)
        if (clrindex == Luts[i].second) {
          *(dest_scan + col) = static_cast<uint8_t>(Luts[i].first);
          break;
        }
    }
  }
  for (size_t i = 0; i < 256; ++i)
    dst_plt[i] = pal[i];
}

void ConvertBuffer_1bppMask2Rgb(FXDIB_Format dest_format,
                                uint8_t* dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  static constexpr uint8_t kSetGray = 0xff;
  static constexpr uint8_t kResetGray = 0x00;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
    for (int col = src_left; col < src_left + width; ++col) {
      uint8_t value =
          (src_scan[col / 8] & (1 << (7 - col % 8))) ? kSetGray : kResetGray;
      memset(dest_scan, value, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_8bppMask2Rgb(FXDIB_Format dest_format,
                                uint8_t* dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
    for (int col = 0; col < width; ++col) {
      memset(dest_scan, *src_scan, 3);
      dest_scan += comps;
      ++src_scan;
    }
  }
}

void ConvertBuffer_1bppPlt2Rgb(FXDIB_Format dest_format,
                               uint8_t* dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint32_t plt[2];
  uint8_t* bgr_ptr = reinterpret_cast<uint8_t*>(plt);
  bgr_ptr[0] = FXARGB_B(src_palette[0]);
  bgr_ptr[1] = FXARGB_G(src_palette[0]);
  bgr_ptr[2] = FXARGB_R(src_palette[0]);
  bgr_ptr[3] = FXARGB_B(src_palette[1]);
  bgr_ptr[4] = FXARGB_G(src_palette[1]);
  bgr_ptr[5] = FXARGB_R(src_palette[1]);

  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
    for (int col = src_left; col < src_left + width; ++col) {
      size_t offset = (src_scan[col / 8] & (1 << (7 - col % 8))) ? 3 : 0;
      memcpy(dest_scan, bgr_ptr + offset, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_8bppPlt2Rgb(FXDIB_Format dest_format,
                               uint8_t* dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint32_t plt[256];
  uint8_t* bgr_ptr = reinterpret_cast<uint8_t*>(plt);
  for (int i = 0; i < 256; ++i) {
    *bgr_ptr++ = FXARGB_B(src_palette[i]);
    *bgr_ptr++ = FXARGB_G(src_palette[i]);
    *bgr_ptr++ = FXARGB_R(src_palette[i]);
  }
  bgr_ptr = reinterpret_cast<uint8_t*>(plt);

  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row) + src_left;
    for (int col = 0; col < width; ++col) {
      uint8_t* src_pixel = bgr_ptr + 3 * (*src_scan++);
      memcpy(dest_scan, src_pixel, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_24bppRgb2Rgb24(uint8_t* dest_buf,
                                  int dest_pitch,
                                  int width,
                                  int height,
                                  const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                  int src_left,
                                  int src_top) {
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row) + src_left * 3;
    memcpy(dest_scan, src_scan, width * 3);
  }
}

void ConvertBuffer_32bppRgb2Rgb24(uint8_t* dest_buf,
                                  int dest_pitch,
                                  int width,
                                  int height,
                                  const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                  int src_left,
                                  int src_top) {
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row) + src_left * 4;
    for (int col = 0; col < width; ++col) {
      memcpy(dest_scan, src_scan, 3);
      dest_scan += 3;
      src_scan += 4;
    }
  }
}

void ConvertBuffer_Rgb2Rgb32(uint8_t* dest_buf,
                             int dest_pitch,
                             int width,
                             int height,
                             const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                             int src_left,
                             int src_top) {
  int comps = pSrcBitmap->GetBPP() / 8;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = dest_buf + row * dest_pitch;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row) + src_left * comps;
    for (int col = 0; col < width; ++col) {
      memcpy(dest_scan, src_scan, 3);
      dest_scan += 4;
      src_scan += comps;
    }
  }
}

bool ConvertBuffer_8bppMask(int bpp,
                            uint8_t* dest_buf,
                            int dest_pitch,
                            int width,
                            int height,
                            const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                            int src_left,
                            int src_top) {
  switch (bpp) {
    case 1:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_1bppPlt2Gray(dest_buf, dest_pitch, width, height,
                                   pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_1bppMask2Gray(dest_buf, dest_pitch, width, height,
                                    pSrcBitmap, src_left, src_top);
      }
      return true;
    case 8:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_8bppPlt2Gray(dest_buf, dest_pitch, width, height,
                                   pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_8bppMask2Gray(dest_buf, dest_pitch, width, height,
                                    pSrcBitmap, src_left, src_top);
      }
      return true;
    case 24:
    case 32:
      ConvertBuffer_Rgb2Gray(dest_buf, dest_pitch, width, height, pSrcBitmap,
                             src_left, src_top);
      return true;
    default:
      return false;
  }
}

bool ConvertBuffer_Rgb(int bpp,
                       FXDIB_Format dest_format,
                       uint8_t* dest_buf,
                       int dest_pitch,
                       int width,
                       int height,
                       const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                       int src_left,
                       int src_top) {
  switch (bpp) {
    case 1:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_1bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width,
                                  height, pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_1bppMask2Rgb(dest_format, dest_buf, dest_pitch, width,
                                   height, pSrcBitmap, src_left, src_top);
      }
      return true;
    case 8:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_8bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width,
                                  height, pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_8bppMask2Rgb(dest_format, dest_buf, dest_pitch, width,
                                   height, pSrcBitmap, src_left, src_top);
      }
      return true;
    case 24:
      ConvertBuffer_24bppRgb2Rgb24(dest_buf, dest_pitch, width, height,
                                   pSrcBitmap, src_left, src_top);
      return true;
    case 32:
      ConvertBuffer_32bppRgb2Rgb24(dest_buf, dest_pitch, width, height,
                                   pSrcBitmap, src_left, src_top);
      return true;
    default:
      return false;
  }
}

bool ConvertBuffer_Argb(int bpp,
                        FXDIB_Format dest_format,
                        uint8_t* dest_buf,
                        int dest_pitch,
                        int width,
                        int height,
                        const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                        int src_left,
                        int src_top) {
  switch (bpp) {
    case 1:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_1bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width,
                                  height, pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_1bppMask2Rgb(dest_format, dest_buf, dest_pitch, width,
                                   height, pSrcBitmap, src_left, src_top);
      }
      return true;
    case 8:
      if (pSrcBitmap->HasPalette()) {
        ConvertBuffer_8bppPlt2Rgb(dest_format, dest_buf, dest_pitch, width,
                                  height, pSrcBitmap, src_left, src_top);
      } else {
        ConvertBuffer_8bppMask2Rgb(dest_format, dest_buf, dest_pitch, width,
                                   height, pSrcBitmap, src_left, src_top);
      }
      return true;
    case 24:
    case 32:
      ConvertBuffer_Rgb2Rgb32(dest_buf, dest_pitch, width, height, pSrcBitmap,
                              src_left, src_top);
      return true;
    default:
      return false;
  }
}

}  // namespace

CFX_DIBBase::CFX_DIBBase() = default;

CFX_DIBBase::~CFX_DIBBase() = default;

uint8_t* CFX_DIBBase::GetBuffer() const {
  return nullptr;
}

bool CFX_DIBBase::SkipToScanline(int line, PauseIndicatorIface* pPause) const {
  return false;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::Clone(const FX_RECT* pClip) const {
  FX_RECT rect(0, 0, m_Width, m_Height);
  if (pClip) {
    rect.Intersect(*pClip);
    if (rect.IsEmpty())
      return nullptr;
  }
  auto pNewBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pNewBitmap->Create(rect.Width(), rect.Height(), GetFormat()))
    return nullptr;

  pNewBitmap->SetPalette(GetPaletteSpan());
  pNewBitmap->SetAlphaMask(m_pAlphaMask, pClip);
  if (GetBPP() == 1 && rect.left % 8 != 0) {
    int left_shift = rect.left % 32;
    int right_shift = 32 - left_shift;
    int dword_count = pNewBitmap->m_Pitch / 4;
    for (int row = rect.top; row < rect.bottom; ++row) {
      const uint32_t* src_scan =
          reinterpret_cast<const uint32_t*>(GetScanline(row)) + rect.left / 32;
      uint32_t* dest_scan = reinterpret_cast<uint32_t*>(
          pNewBitmap->GetWritableScanline(row - rect.top));
      for (int i = 0; i < dword_count; ++i) {
        dest_scan[i] =
            (src_scan[i] << left_shift) | (src_scan[i + 1] >> right_shift);
      }
    }
  } else {
    int copy_len = (pNewBitmap->GetWidth() * pNewBitmap->GetBPP() + 7) / 8;
    if (m_Pitch < static_cast<uint32_t>(copy_len))
      copy_len = m_Pitch;

    for (int row = rect.top; row < rect.bottom; ++row) {
      const uint8_t* src_scan =
          GetScanline(row) + rect.left * GetBppFromFormat(m_Format) / 8;
      uint8_t* dest_scan = pNewBitmap->GetWritableScanline(row - rect.top);
      memcpy(dest_scan, src_scan, copy_len);
    }
  }
  return pNewBitmap;
}

void CFX_DIBBase::BuildPalette() {
  if (HasPalette())
    return;

  if (GetBPP() == 1) {
    m_palette = {0xff000000, 0xffffffff};
  } else if (GetBPP() == 8) {
    m_palette.resize(256);
    for (int i = 0; i < 256; ++i)
      m_palette[i] = ArgbEncode(0xff, i, i, i);
  }
}

bool CFX_DIBBase::BuildAlphaMask() {
  if (m_pAlphaMask)
    return true;

  m_pAlphaMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!m_pAlphaMask->Create(m_Width, m_Height, FXDIB_Format::k8bppMask)) {
    m_pAlphaMask = nullptr;
    return false;
  }
  memset(m_pAlphaMask->GetBuffer(), 0xff,
         m_pAlphaMask->GetHeight() * m_pAlphaMask->GetPitch());
  return true;
}

size_t CFX_DIBBase::GetRequiredPaletteSize() const {
  if (IsMaskFormat())
    return 0;

  switch (GetBppFromFormat(m_Format)) {
    case 1:
      return 2;
    case 8:
      return 256;
    default:
      return 0;
  }
}

uint32_t CFX_DIBBase::GetPaletteArgb(int index) const {
  DCHECK((GetBPP() == 1 || GetBPP() == 8) && !IsMaskFormat());
  if (HasPalette())
    return GetPaletteSpan()[index];

  if (GetBPP() == 1)
    return index ? 0xffffffff : 0xff000000;

  return ArgbEncode(0xff, index, index, index);
}

void CFX_DIBBase::SetPaletteArgb(int index, uint32_t color) {
  DCHECK((GetBPP() == 1 || GetBPP() == 8) && !IsMaskFormat());
  BuildPalette();
  m_palette[index] = color;
}

int CFX_DIBBase::FindPalette(uint32_t color) const {
  DCHECK((GetBPP() == 1 || GetBPP() == 8) && !IsMaskFormat());
  if (HasPalette()) {
    int palsize = (1 << GetBPP());
    pdfium::span<const uint32_t> palette = GetPaletteSpan();
    for (int i = 0; i < palsize; ++i) {
      if (palette[i] == color)
        return i;
    }
    return -1;
  }

  if (GetBPP() == 1)
    return (static_cast<uint8_t>(color) == 0xff) ? 1 : 0;
  return static_cast<uint8_t>(color);
}

bool CFX_DIBBase::GetOverlapRect(int& dest_left,
                                 int& dest_top,
                                 int& width,
                                 int& height,
                                 int src_width,
                                 int src_height,
                                 int& src_left,
                                 int& src_top,
                                 const CFX_ClipRgn* pClipRgn) const {
  if (width == 0 || height == 0)
    return false;

  DCHECK(width > 0);
  DCHECK(height > 0);

  if (dest_left > m_Width || dest_top > m_Height)
    return false;

  FX_SAFE_INT32 safe_src_width = src_left;
  safe_src_width += width;
  if (!safe_src_width.IsValid())
    return false;

  FX_SAFE_INT32 safe_src_height = src_top;
  safe_src_height += height;
  if (!safe_src_height.IsValid())
    return false;

  FX_RECT src_rect(src_left, src_top, safe_src_width.ValueOrDie(),
                   safe_src_height.ValueOrDie());
  FX_RECT src_bound(0, 0, src_width, src_height);
  src_rect.Intersect(src_bound);

  FX_SAFE_INT32 safe_x_offset = dest_left;
  safe_x_offset -= src_left;
  if (!safe_x_offset.IsValid())
    return false;

  FX_SAFE_INT32 safe_y_offset = dest_top;
  safe_y_offset -= src_top;
  if (!safe_y_offset.IsValid())
    return false;

  FX_SAFE_INT32 safe_dest_left = safe_x_offset;
  safe_dest_left += src_rect.left;
  if (!safe_dest_left.IsValid())
    return false;

  FX_SAFE_INT32 safe_dest_top = safe_y_offset;
  safe_dest_top += src_rect.top;
  if (!safe_dest_top.IsValid())
    return false;

  FX_SAFE_INT32 safe_dest_right = safe_x_offset;
  safe_dest_right += src_rect.right;
  if (!safe_dest_right.IsValid())
    return false;

  FX_SAFE_INT32 safe_dest_bottom = safe_y_offset;
  safe_dest_bottom += src_rect.bottom;
  if (!safe_dest_bottom.IsValid())
    return false;

  FX_RECT dest_rect(safe_dest_left.ValueOrDie(), safe_dest_top.ValueOrDie(),
                    safe_dest_right.ValueOrDie(),
                    safe_dest_bottom.ValueOrDie());
  FX_RECT dest_bound(0, 0, m_Width, m_Height);
  dest_rect.Intersect(dest_bound);

  if (pClipRgn)
    dest_rect.Intersect(pClipRgn->GetBox());
  dest_left = dest_rect.left;
  dest_top = dest_rect.top;

  FX_SAFE_INT32 safe_new_src_left = dest_left;
  safe_new_src_left -= safe_x_offset;
  if (!safe_new_src_left.IsValid())
    return false;
  src_left = safe_new_src_left.ValueOrDie();

  FX_SAFE_INT32 safe_new_src_top = dest_top;
  safe_new_src_top -= safe_y_offset;
  if (!safe_new_src_top.IsValid())
    return false;
  src_top = safe_new_src_top.ValueOrDie();

  if (dest_rect.IsEmpty())
    return false;

  width = dest_rect.Width();
  height = dest_rect.Height();
  return true;
}

void CFX_DIBBase::SetPalette(pdfium::span<const uint32_t> src_palette) {
  static const uint32_t kPaletteSize = 256;
  if (src_palette.empty() || GetBPP() > 8) {
    m_palette.clear();
    return;
  }
  uint32_t pal_size = 1 << GetBPP();
  if (m_palette.empty())
    m_palette.resize(pal_size);
  pal_size = std::min(pal_size, kPaletteSize);
  for (size_t i = 0; i < pal_size; ++i)
    m_palette[i] = src_palette[i];
}

void CFX_DIBBase::GetPalette(uint32_t* pal, int alpha) const {
  DCHECK(GetBPP() <= 8);

  if (GetBPP() == 1) {
    pal[0] = ((HasPalette() ? GetPaletteSpan()[0] : 0xff000000) & 0xffffff) |
             (alpha << 24);
    pal[1] = ((HasPalette() ? GetPaletteSpan()[1] : 0xffffffff) & 0xffffff) |
             (alpha << 24);
    return;
  }
  if (HasPalette()) {
    pdfium::span<const uint32_t> palette = GetPaletteSpan();
    for (int i = 0; i < 256; ++i)
      pal[i] = (palette[i] & 0x00ffffff) | (alpha << 24);
  } else {
    for (int i = 0; i < 256; ++i)
      pal[i] = ArgbEncode(alpha, i, i, i);
  }
}

uint32_t CFX_DIBBase::GetAlphaMaskPitch() const {
  return m_pAlphaMask ? m_pAlphaMask->GetPitch() : 0;
}

const uint8_t* CFX_DIBBase::GetAlphaMaskScanline(int line) const {
  return m_pAlphaMask ? m_pAlphaMask->GetScanline(line) : nullptr;
}

uint8_t* CFX_DIBBase::GetWritableAlphaMaskScanline(int line) {
  return m_pAlphaMask ? m_pAlphaMask->GetWritableScanline(line) : nullptr;
}

uint8_t* CFX_DIBBase::GetAlphaMaskBuffer() {
  return m_pAlphaMask ? m_pAlphaMask->GetBuffer() : nullptr;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::GetAlphaMask() {
  return m_pAlphaMask;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::CloneAlphaMask() const {
  DCHECK_EQ(GetFormat(), FXDIB_Format::kArgb);
  FX_RECT rect(0, 0, m_Width, m_Height);
  auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pMask->Create(rect.Width(), rect.Height(), FXDIB_Format::k8bppMask))
    return nullptr;

  for (int row = rect.top; row < rect.bottom; ++row) {
    const uint8_t* src_scan = GetScanline(row) + rect.left * 4 + 3;
    uint8_t* dest_scan = pMask->GetWritableScanline(row - rect.top);
    for (int col = rect.left; col < rect.right; ++col) {
      *dest_scan++ = *src_scan;
      src_scan += 4;
    }
  }
  return pMask;
}

bool CFX_DIBBase::SetAlphaMask(const RetainPtr<CFX_DIBBase>& pAlphaMask,
                               const FX_RECT* pClip) {
  if (!IsAlphaFormat() || GetFormat() == FXDIB_Format::kArgb)
    return false;

  if (!pAlphaMask) {
    m_pAlphaMask->Clear(0xff000000);
    return true;
  }
  FX_RECT rect(0, 0, pAlphaMask->m_Width, pAlphaMask->m_Height);
  if (pClip) {
    rect.Intersect(*pClip);
    if (rect.IsEmpty() || rect.Width() != m_Width ||
        rect.Height() != m_Height) {
      return false;
    }
  } else {
    if (pAlphaMask->m_Width != m_Width || pAlphaMask->m_Height != m_Height)
      return false;
  }
  for (int row = 0; row < m_Height; ++row) {
    memcpy(m_pAlphaMask->GetWritableScanline(row),
           pAlphaMask->GetScanline(row + rect.top) + rect.left,
           m_pAlphaMask->m_Pitch);
  }
  return true;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::FlipImage(bool bXFlip, bool bYFlip) const {
  auto pFlipped = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pFlipped->Create(m_Width, m_Height, GetFormat()))
    return nullptr;

  pFlipped->SetPalette(GetPaletteSpan());
  uint8_t* pDestBuffer = pFlipped->GetBuffer();
  int Bpp = GetBppFromFormat(m_Format) / 8;
  for (int row = 0; row < m_Height; ++row) {
    const uint8_t* src_scan = GetScanline(row);
    uint8_t* dest_scan =
        pDestBuffer + m_Pitch * (bYFlip ? (m_Height - row - 1) : row);
    if (!bXFlip) {
      memcpy(dest_scan, src_scan, m_Pitch);
      continue;
    }
    if (GetBppFromFormat(m_Format) == 1) {
      memset(dest_scan, 0, m_Pitch);
      for (int col = 0; col < m_Width; ++col) {
        if (src_scan[col / 8] & (1 << (7 - col % 8))) {
          int dest_col = m_Width - col - 1;
          dest_scan[dest_col / 8] |= (1 << (7 - dest_col % 8));
        }
      }
      continue;
    }

    dest_scan += (m_Width - 1) * Bpp;
    if (Bpp == 1) {
      for (int col = 0; col < m_Width; ++col) {
        *dest_scan = *src_scan;
        --dest_scan;
        ++src_scan;
      }
    } else if (Bpp == 3) {
      for (int col = 0; col < m_Width; ++col) {
        memcpy(dest_scan, src_scan, 3);
        dest_scan -= 3;
        src_scan += 3;
      }
    } else {
      DCHECK_EQ(Bpp, 4);
      for (int col = 0; col < m_Width; ++col) {
        const auto* src_scan32 = reinterpret_cast<const uint32_t*>(src_scan);
        uint32_t* dest_scan32 = reinterpret_cast<uint32_t*>(dest_scan);
        *dest_scan32 = *src_scan32;
        dest_scan -= 4;
        src_scan += 4;
      }
    }
  }
  if (m_pAlphaMask) {
    pDestBuffer = pFlipped->m_pAlphaMask->GetBuffer();
    uint32_t dest_pitch = pFlipped->m_pAlphaMask->GetPitch();
    for (int row = 0; row < m_Height; ++row) {
      const uint8_t* src_scan = m_pAlphaMask->GetScanline(row);
      uint8_t* dest_scan =
          pDestBuffer + dest_pitch * (bYFlip ? (m_Height - row - 1) : row);
      if (!bXFlip) {
        memcpy(dest_scan, src_scan, dest_pitch);
        continue;
      }
      dest_scan += (m_Width - 1);
      for (int col = 0; col < m_Width; ++col) {
        *dest_scan = *src_scan;
        --dest_scan;
        ++src_scan;
      }
    }
  }
  return pFlipped;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::CloneConvert(FXDIB_Format dest_format) {
  if (dest_format == GetFormat())
    return Clone(nullptr);

  auto pClone = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pClone->Create(m_Width, m_Height, dest_format))
    return nullptr;

  RetainPtr<CFX_DIBitmap> pSrcAlpha;
  if (IsAlphaFormat()) {
    pSrcAlpha =
        (GetFormat() == FXDIB_Format::kArgb) ? CloneAlphaMask() : m_pAlphaMask;
    if (!pSrcAlpha)
      return nullptr;
  }
  if (GetIsAlphaFromFormat(dest_format)) {
    bool ret;
    if (dest_format == FXDIB_Format::kArgb) {
      ret = pSrcAlpha ? pClone->SetAlphaFromBitmap(pSrcAlpha)
                      : pClone->SetUniformOpaqueAlpha();
    } else {
      ret = pClone->SetAlphaMask(pSrcAlpha, nullptr);
    }
    if (!ret)
      return nullptr;
  }

  RetainPtr<CFX_DIBBase> holder(this);
  std::vector<uint32_t, FxAllocAllocator<uint32_t>> pal_8bpp;
  if (!ConvertBuffer(dest_format, pClone->GetBuffer(), pClone->GetPitch(),
                     m_Width, m_Height, holder, 0, 0, &pal_8bpp)) {
    return nullptr;
  }
  if (!pal_8bpp.empty())
    pClone->SetPalette(pal_8bpp);

  return pClone;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::SwapXY(bool bXFlip, bool bYFlip) const {
  FX_RECT dest_clip(0, 0, m_Height, m_Width);
  if (dest_clip.IsEmpty())
    return nullptr;

  auto pTransBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  int result_height = dest_clip.Height();
  int result_width = dest_clip.Width();
  if (!pTransBitmap->Create(result_width, result_height, GetFormat()))
    return nullptr;

  pTransBitmap->SetPalette(GetPaletteSpan());
  int dest_pitch = pTransBitmap->GetPitch();
  uint8_t* dest_buf = pTransBitmap->GetBuffer();
  int row_start = bXFlip ? m_Height - dest_clip.right : dest_clip.left;
  int row_end = bXFlip ? m_Height - dest_clip.left : dest_clip.right;
  int col_start = bYFlip ? m_Width - dest_clip.bottom : dest_clip.top;
  int col_end = bYFlip ? m_Width - dest_clip.top : dest_clip.bottom;
  if (GetBPP() == 1) {
    memset(dest_buf, 0xff, dest_pitch * result_height);
    for (int row = row_start; row < row_end; ++row) {
      const uint8_t* src_scan = GetScanline(row);
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf;
      if (bYFlip)
        dest_scan += (result_height - 1) * dest_pitch;
      int dest_step = bYFlip ? -dest_pitch : dest_pitch;
      for (int col = col_start; col < col_end; ++col) {
        if (!(src_scan[col / 8] & (1 << (7 - col % 8))))
          dest_scan[dest_col / 8] &= ~(1 << (7 - dest_col % 8));
        dest_scan += dest_step;
      }
    }
  } else {
    int nBytes = GetBPP() / 8;
    int dest_step = bYFlip ? -dest_pitch : dest_pitch;
    if (nBytes == 3)
      dest_step -= 2;
    for (int row = row_start; row < row_end; ++row) {
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf + dest_col * nBytes;
      if (bYFlip)
        dest_scan += (result_height - 1) * dest_pitch;
      if (nBytes == 4) {
        const uint32_t* src_scan =
            reinterpret_cast<const uint32_t*>(GetScanline(row)) + col_start;
        for (int col = col_start; col < col_end; ++col) {
          uint32_t* dest_scan32 = reinterpret_cast<uint32_t*>(dest_scan);
          *dest_scan32 = *src_scan++;
          dest_scan += dest_step;
        }
      } else {
        const uint8_t* src_scan = GetScanline(row) + col_start * nBytes;
        if (nBytes == 1) {
          for (int col = col_start; col < col_end; ++col) {
            *dest_scan = *src_scan++;
            dest_scan += dest_step;
          }
        } else {
          for (int col = col_start; col < col_end; ++col) {
            memcpy(dest_scan, src_scan, 3);
            dest_scan += 2 + dest_step;
            src_scan += 3;
          }
        }
      }
    }
  }
  if (m_pAlphaMask) {
    dest_pitch = pTransBitmap->m_pAlphaMask->GetPitch();
    dest_buf = pTransBitmap->m_pAlphaMask->GetBuffer();
    int dest_step = bYFlip ? -dest_pitch : dest_pitch;
    for (int row = row_start; row < row_end; ++row) {
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf + dest_col;
      if (bYFlip)
        dest_scan += (result_height - 1) * dest_pitch;
      const uint8_t* src_scan = m_pAlphaMask->GetScanline(row) + col_start;
      for (int col = col_start; col < col_end; ++col) {
        *dest_scan = *src_scan++;
        dest_scan += dest_step;
      }
    }
  }
  return pTransBitmap;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::TransformTo(const CFX_Matrix& mtDest,
                                                 int* result_left,
                                                 int* result_top) {
  RetainPtr<CFX_DIBBase> holder(this);
  CFX_ImageTransformer transformer(holder, mtDest, FXDIB_ResampleOptions(),
                                   nullptr);
  transformer.Continue(nullptr);
  *result_left = transformer.result().left;
  *result_top = transformer.result().top;
  return transformer.DetachBitmap();
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::StretchTo(
    int dest_width,
    int dest_height,
    const FXDIB_ResampleOptions& options,
    const FX_RECT* pClip) {
  RetainPtr<CFX_DIBBase> holder(this);
  FX_RECT clip_rect(0, 0, abs(dest_width), abs(dest_height));
  if (pClip)
    clip_rect.Intersect(*pClip);

  if (clip_rect.IsEmpty())
    return nullptr;

  if (dest_width == m_Width && dest_height == m_Height)
    return Clone(&clip_rect);

  CFX_BitmapStorer storer;
  CFX_ImageStretcher stretcher(&storer, holder, dest_width, dest_height,
                               clip_rect, options);
  if (stretcher.Start())
    stretcher.Continue(nullptr);

  return storer.Detach();
}

// static
bool CFX_DIBBase::ConvertBuffer(
    FXDIB_Format dest_format,
    uint8_t* dest_buf,
    int dest_pitch,
    int width,
    int height,
    const RetainPtr<CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top,
    std::vector<uint32_t, FxAllocAllocator<uint32_t>>* pal) {
  FXDIB_Format src_format = pSrcBitmap->GetFormat();
  const int bpp = GetBppFromFormat(src_format);
  switch (dest_format) {
    case FXDIB_Format::k8bppMask: {
      return ConvertBuffer_8bppMask(bpp, dest_buf, dest_pitch, width, height,
                                    pSrcBitmap, src_left, src_top);
    }
    case FXDIB_Format::k8bppRgb: {
      const bool bpp_1_or_8 = (bpp == 1 || bpp == 8);
      if (bpp_1_or_8 && !pSrcBitmap->HasPalette()) {
        return ConvertBuffer(FXDIB_Format::k8bppMask, dest_buf, dest_pitch,
                             width, height, pSrcBitmap, src_left, src_top, pal);
      }
      pal->resize(256);
      if (bpp_1_or_8 && pSrcBitmap->HasPalette()) {
        ConvertBuffer_Plt2PltRgb8(dest_buf, dest_pitch, width, height,
                                  pSrcBitmap, src_left, src_top, *pal);
        return true;
      }
      if (bpp >= 24) {
        ConvertBuffer_Rgb2PltRgb8(dest_buf, dest_pitch, width, height,
                                  pSrcBitmap, src_left, src_top, *pal);
        return true;
      }
      return false;
    }
    case FXDIB_Format::kRgb: {
      return ConvertBuffer_Rgb(bpp, dest_format, dest_buf, dest_pitch, width,
                               height, pSrcBitmap, src_left, src_top);
    }
    case FXDIB_Format::kArgb:
    case FXDIB_Format::kRgb32: {
      return ConvertBuffer_Argb(bpp, dest_format, dest_buf, dest_pitch, width,
                                height, pSrcBitmap, src_left, src_top);
    }
    default:
      NOTREACHED();
      return false;
  }
}
