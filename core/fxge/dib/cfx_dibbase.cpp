// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_dibbase.h"

#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/cfx_cliprgn.h"
#include "core/fxge/dib/cfx_bitmapstorer.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagestretcher.h"
#include "core/fxge/dib/cfx_imagetransformer.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"
#include "third_party/base/notreached.h"

namespace {

void ColorDecode(uint32_t pal_v, uint8_t* r, uint8_t* g, uint8_t* b) {
  *r = static_cast<uint8_t>((pal_v & 0xf00) >> 4);
  *g = static_cast<uint8_t>(pal_v & 0x0f0);
  *b = static_cast<uint8_t>((pal_v & 0x00f) << 4);
}

class CFX_Palette {
 public:
  // (Amount, Color) pairs
  using LutsData = std::pair<uint32_t, uint32_t>;

  explicit CFX_Palette(const RetainPtr<const CFX_DIBBase>& pBitmap);
  ~CFX_Palette();

  pdfium::span<const uint32_t> GetPalette() { return m_Palette; }
  pdfium::span<const LutsData> GetValidLuts() {
    return pdfium::make_span(m_Luts).first(m_lut);
  }

 private:
  void ObtainPalette();

  std::vector<uint32_t> m_Palette;
  std::vector<LutsData> m_Luts;
  uint32_t m_lut = 0;
};

CFX_Palette::CFX_Palette(const RetainPtr<const CFX_DIBBase>& pBitmap)
    : m_Palette(256), m_Luts(4096) {
  int bpp = pBitmap->GetBPP() / 8;
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  for (int row = 0; row < height; ++row) {
    pdfium::span<const uint8_t> scan_line = pBitmap->GetScanline(row);
    for (int col = 0; col < width; ++col) {
      const uint8_t* src_port =
          scan_line.subspan(Fx2DSizeOrDie(col, bpp)).data();
      uint32_t b = src_port[0] & 0xf0;
      uint32_t g = src_port[1] & 0xf0;
      uint32_t r = src_port[2] & 0xf0;
      uint32_t index = (r << 4) + g + (b >> 4);
      ++m_Luts[index].first;
    }
  }
  // Move non-zeros to the front and count them
  for (uint32_t row = 0; row < m_Luts.size(); ++row) {
    if (m_Luts[row].first != 0) {
      m_Luts[m_lut].first = m_Luts[row].first;
      m_Luts[m_lut].second = row;
      ++m_lut;
    }
  }
  pdfium::span<LutsData> lut_span = pdfium::make_span(m_Luts).first(m_lut);
  std::sort(lut_span.begin(), lut_span.end(),
            [](const LutsData& arg1, const LutsData& arg2) {
              return arg1.first < arg2.first;
            });
  ObtainPalette();
}

CFX_Palette::~CFX_Palette() = default;

void CFX_Palette::ObtainPalette() {
  for (uint32_t row = 0; row < m_Palette.size(); ++row) {
    const uint32_t lut_offset = (m_lut - row - 1) % m_Palette.size();
    const uint32_t color = m_Luts[lut_offset].second;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    ColorDecode(color, &r, &g, &b);
    m_Palette[row] = (static_cast<uint32_t>(r) << 16) |
                     (static_cast<uint32_t>(g) << 8) | b | 0xff000000;
    m_Luts[lut_offset].first = row;
  }
  if (m_lut > 256) {
    int err;
    int min_err;
    const uint32_t lut_256 = m_lut - 256;
    for (uint32_t row = 0; row < lut_256; ++row) {
      min_err = 1000000;
      uint8_t r;
      uint8_t g;
      uint8_t b;
      ColorDecode(m_Luts[row].second, &r, &g, &b);
      uint32_t clrindex = 0;
      for (int col = 0; col < 256; ++col) {
        uint32_t p_color = m_Palette[col];
        int d_r = r - static_cast<uint8_t>(p_color >> 16);
        int d_g = g - static_cast<uint8_t>(p_color >> 8);
        int d_b = b - static_cast<uint8_t>(p_color);
        err = d_r * d_r + d_g * d_g + d_b * d_b;
        if (err < min_err) {
          min_err = err;
          clrindex = col;
        }
      }
      m_Luts[row].first = clrindex;
    }
  }
}

void ConvertBuffer_1bppMask2Gray(pdfium::span<uint8_t> dest_buf,
                                 int dest_pitch,
                                 int width,
                                 int height,
                                 const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                 int src_left,
                                 int src_top) {
  static constexpr uint8_t kSetGray = 0xff;
  static constexpr uint8_t kResetGray = 0x00;
  for (int row = 0; row < height; ++row) {
    pdfium::span<uint8_t> dest_span =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch));
    pdfium::span<const uint8_t> src_span =
        pSrcBitmap->GetScanline(src_top + row);
    fxcrt::spanset(dest_span.first(width), kResetGray);
    uint8_t* dest_scan = dest_span.data();
    const uint8_t* src_scan = src_span.data();
    for (int col = src_left; col < src_left + width; ++col) {
      if (src_scan[col / 8] & (1 << (7 - col % 8)))
        *dest_scan = kSetGray;
      ++dest_scan;
    }
  }
}

void ConvertBuffer_8bppMask2Gray(pdfium::span<uint8_t> dest_buf,
                                 int dest_pitch,
                                 int width,
                                 int height,
                                 const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                 int src_left,
                                 int src_top) {
  for (int row = 0; row < height; ++row) {
    fxcrt::spancpy(
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)),
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left, width));
  }
}

void ConvertBuffer_1bppPlt2Gray(pdfium::span<uint8_t> dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  const uint8_t reset_r = FXARGB_R(src_palette[0]);
  const uint8_t reset_g = FXARGB_G(src_palette[0]);
  const uint8_t reset_b = FXARGB_B(src_palette[0]);
  const uint8_t set_r = FXARGB_R(src_palette[1]);
  const uint8_t set_g = FXARGB_G(src_palette[1]);
  const uint8_t set_b = FXARGB_B(src_palette[1]);
  const uint8_t gray0 = FXRGB2GRAY(reset_r, reset_g, reset_b);
  const uint8_t gray1 = FXRGB2GRAY(set_r, set_g, set_b);

  for (int row = 0; row < height; ++row) {
    pdfium::span<uint8_t> dest_span =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch));
    fxcrt::spanset(dest_span.first(width), gray0);
    uint8_t* dest_scan = dest_span.data();
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row).data();
    for (int col = src_left; col < src_left + width; ++col) {
      if (src_scan[col / 8] & (1 << (7 - col % 8)))
        *dest_scan = gray1;
      ++dest_scan;
    }
  }
}

void ConvertBuffer_8bppPlt2Gray(pdfium::span<uint8_t> dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint8_t gray[256];
  for (size_t i = 0; i < std::size(gray); ++i) {
    gray[i] = FXRGB2GRAY(FXARGB_R(src_palette[i]), FXARGB_G(src_palette[i]),
                         FXARGB_B(src_palette[i]));
  }

  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left).data();
    for (int col = 0; col < width; ++col)
      *dest_scan++ = gray[*src_scan++];
  }
}

void ConvertBuffer_Rgb2Gray(pdfium::span<uint8_t> dest_buf,
                            int dest_pitch,
                            int width,
                            int height,
                            const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                            int src_left,
                            int src_top) {
  const int Bpp = pSrcBitmap->GetBPP() / 8;
  const size_t x_offset = Fx2DSizeOrDie(src_left, Bpp);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(x_offset).data();
    for (int col = 0; col < width; ++col) {
      *dest_scan++ = FXRGB2GRAY(src_scan[2], src_scan[1], src_scan[0]);
      src_scan += Bpp;
    }
  }
}

void ConvertBuffer_IndexCopy(pdfium::span<uint8_t> dest_buf,
                             int dest_pitch,
                             int width,
                             int height,
                             const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                             int src_left,
                             int src_top) {
  if (pSrcBitmap->GetBPP() == 1) {
    for (int row = 0; row < height; ++row) {
      pdfium::span<uint8_t> dest_span =
          dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch));
      // Set all destination pixels to be white initially.
      fxcrt::spanset(dest_span.first(width), 255);
      uint8_t* dest_scan = dest_span.data();
      const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row).data();
      for (int col = src_left; col < src_left + width; ++col) {
        // If the source bit is set, then set the destination pixel to be black.
        if (src_scan[col / 8] & (1 << (7 - col % 8)))
          *dest_scan = 0;

        ++dest_scan;
      }
    }
  } else {
    for (int row = 0; row < height; ++row) {
      fxcrt::spancpy(
          dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)),
          pSrcBitmap->GetScanline(src_top + row).subspan(src_left, width));
    }
  }
}

void ConvertBuffer_Plt2PltRgb8(pdfium::span<uint8_t> dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top,
                               pdfium::span<uint32_t> dst_plt) {
  ConvertBuffer_IndexCopy(dest_buf, dest_pitch, width, height, pSrcBitmap,
                          src_left, src_top);
  const size_t plt_size = pSrcBitmap->GetRequiredPaletteSize();
  pdfium::span<const uint32_t> src_span = pSrcBitmap->GetPaletteSpan();
  CHECK_LE(plt_size, src_span.size());

  const uint32_t* src_plt = src_span.data();
  for (size_t i = 0; i < plt_size; ++i)
    dst_plt[i] = src_plt[i];
}

void ConvertBuffer_Rgb2PltRgb8(pdfium::span<uint8_t> dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top,
                               pdfium::span<uint32_t> dst_plt) {
  int bpp = pSrcBitmap->GetBPP() / 8;
  CFX_Palette palette(pSrcBitmap);
  pdfium::span<const CFX_Palette::LutsData> luts = palette.GetValidLuts();
  for (int row = 0; row < height; ++row) {
    pdfium::span<const uint8_t> src_span =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left);
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    for (int col = 0; col < width; ++col) {
      const uint8_t* src_port =
          src_span.subspan(Fx2DSizeOrDie(col, bpp)).data();
      int r = src_port[2] & 0xf0;
      int g = src_port[1] & 0xf0;
      int b = src_port[0] & 0xf0;
      uint32_t clrindex = (r << 4) + g + (b >> 4);
      for (size_t i = luts.size(); i > 0; --i) {
        if (clrindex == luts[i - 1].second) {
          *(dest_scan + col) = static_cast<uint8_t>(luts[i - 1].first);
          break;
        }
      }
    }
  }
  fxcrt::spancpy(dst_plt, palette.GetPalette());
}

void ConvertBuffer_1bppMask2Rgb(FXDIB_Format dest_format,
                                pdfium::span<uint8_t> dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  static constexpr uint8_t kSetGray = 0xff;
  static constexpr uint8_t kResetGray = 0x00;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row).data();
    for (int col = src_left; col < src_left + width; ++col) {
      uint8_t value =
          (src_scan[col / 8] & (1 << (7 - col % 8))) ? kSetGray : kResetGray;
      memset(dest_scan, value, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_8bppMask2Rgb(FXDIB_Format dest_format,
                                pdfium::span<uint8_t> dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top) {
  int comps = GetCompsFromFormat(dest_format);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left).data();
    for (int col = 0; col < width; ++col) {
      memset(dest_scan, *src_scan, 3);
      dest_scan += comps;
      ++src_scan;
    }
  }
}

void ConvertBuffer_1bppPlt2Rgb(FXDIB_Format dest_format,
                               pdfium::span<uint8_t> dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  const uint8_t dst_palette[6] = {
      FXARGB_B(src_palette[0]), FXARGB_G(src_palette[0]),
      FXARGB_R(src_palette[0]), FXARGB_B(src_palette[1]),
      FXARGB_G(src_palette[1]), FXARGB_R(src_palette[1])};
  int comps = GetCompsFromFormat(dest_format);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row).data();
    for (int col = src_left; col < src_left + width; ++col) {
      size_t offset = (src_scan[col / 8] & (1 << (7 - col % 8))) ? 3 : 0;
      memcpy(dest_scan, dst_palette + offset, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_8bppPlt2Rgb(FXDIB_Format dest_format,
                               pdfium::span<uint8_t> dest_buf,
                               int dest_pitch,
                               int width,
                               int height,
                               const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                               int src_left,
                               int src_top) {
  pdfium::span<const uint32_t> src_palette = pSrcBitmap->GetPaletteSpan();
  uint8_t dst_palette[768];
  for (int i = 0; i < 256; ++i) {
    dst_palette[3 * i] = FXARGB_B(src_palette[i]);
    dst_palette[3 * i + 1] = FXARGB_G(src_palette[i]);
    dst_palette[3 * i + 2] = FXARGB_R(src_palette[i]);
  }
  int comps = GetCompsFromFormat(dest_format);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left).data();
    for (int col = 0; col < width; ++col) {
      uint8_t* src_pixel = dst_palette + 3 * (*src_scan++);
      memcpy(dest_scan, src_pixel, 3);
      dest_scan += comps;
    }
  }
}

void ConvertBuffer_24bppRgb2Rgb24(
    pdfium::span<uint8_t> dest_buf,
    int dest_pitch,
    int width,
    int height,
    const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top) {
  const size_t x_offset = Fx2DSizeOrDie(src_left, 3);
  const size_t byte_count = Fx2DSizeOrDie(width, 3);
  for (int row = 0; row < height; ++row) {
    fxcrt::spancpy(
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)),
        pSrcBitmap->GetScanline(src_top + row).subspan(x_offset, byte_count));
  }
}

void ConvertBuffer_32bppRgb2Rgb24(
    pdfium::span<uint8_t> dest_buf,
    int dest_pitch,
    int width,
    int height,
    const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top) {
  const size_t x_offset = Fx2DSizeOrDie(src_left, 4);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(x_offset).data();
    for (int col = 0; col < width; ++col) {
      memcpy(dest_scan, src_scan, 3);
      dest_scan += 3;
      src_scan += 4;
    }
  }
}

void ConvertBuffer_Rgb2Rgb32(pdfium::span<uint8_t> dest_buf,
                             int dest_pitch,
                             int width,
                             int height,
                             const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                             int src_left,
                             int src_top) {
  const int comps = pSrcBitmap->GetBPP() / 8;
  const size_t x_offset = Fx2DSizeOrDie(src_left, comps);
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        dest_buf.subspan(Fx2DSizeOrDie(row, dest_pitch)).data();
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(x_offset).data();
    for (int col = 0; col < width; ++col) {
      memcpy(dest_scan, src_scan, 3);
      dest_scan += 4;
      src_scan += comps;
    }
  }
}

bool ConvertBuffer_8bppMask(int bpp,
                            pdfium::span<uint8_t> dest_buf,
                            int dest_pitch,
                            int width,
                            int height,
                            const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
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
                       pdfium::span<uint8_t> dest_buf,
                       int dest_pitch,
                       int width,
                       int height,
                       const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
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
                        pdfium::span<uint8_t> dest_buf,
                        int dest_pitch,
                        int width,
                        int height,
                        const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
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

bool CFX_DIBBase::SkipToScanline(int line, PauseIndicatorIface* pPause) const {
  return false;
}

size_t CFX_DIBBase::GetEstimatedImageMemoryBurden() const {
  return GetRequiredPaletteSize() * sizeof(uint32_t);
}

#if BUILDFLAG(IS_WIN) || defined(_SKIA_SUPPORT_)
RetainPtr<const CFX_DIBitmap> CFX_DIBBase::RealizeIfNeeded() const {
  return Realize();
}
#endif

RetainPtr<CFX_DIBitmap> CFX_DIBBase::Realize() const {
  return ClipToInternal(nullptr);
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::ClipTo(const FX_RECT& rect) const {
  return ClipToInternal(&rect);
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::ClipToInternal(
    const FX_RECT* pClip) const {
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
  if (GetBPP() == 1 && rect.left % 8 != 0) {
    int left_shift = rect.left % 32;
    int right_shift = 32 - left_shift;
    int dword_count = pNewBitmap->m_Pitch / 4;
    for (int row = rect.top; row < rect.bottom; ++row) {
      const uint32_t* src_scan =
          reinterpret_cast<const uint32_t*>(GetScanline(row).data()) +
          rect.left / 32;
      uint32_t* dest_scan = reinterpret_cast<uint32_t*>(
          pNewBitmap->GetWritableScanline(row - rect.top).data());
      for (int i = 0; i < dword_count; ++i) {
        dest_scan[i] =
            (src_scan[i] << left_shift) | (src_scan[i + 1] >> right_shift);
      }
    }
  } else {
    absl::optional<uint32_t> copy_len = fxge::CalculatePitch8(
        pNewBitmap->GetBPP(), /*components=*/1, pNewBitmap->GetWidth());
    if (!copy_len.has_value()) {
      return nullptr;
    }

    copy_len = std::min<uint32_t>(m_Pitch, copy_len.value());

    FX_SAFE_UINT32 offset = rect.left;
    offset *= GetBppFromFormat(m_Format);
    offset /= 8;
    if (!offset.IsValid())
      return nullptr;

    for (int row = rect.top; row < rect.bottom; ++row) {
      const uint8_t* src_scan =
          GetScanline(row).subspan(offset.ValueOrDie()).data();
      uint8_t* dest_scan =
          pNewBitmap->GetWritableScanline(row - rect.top).data();
      memcpy(dest_scan, src_scan, copy_len.value());
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

  DCHECK_GT(width, 0);
  DCHECK_GT(height, 0);

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

RetainPtr<CFX_DIBitmap> CFX_DIBBase::CloneAlphaMask() const {
  DCHECK_EQ(GetFormat(), FXDIB_Format::kArgb);
  auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pMask->Create(m_Width, m_Height, FXDIB_Format::k8bppMask))
    return nullptr;

  for (int row = 0; row < m_Height; ++row) {
    const uint8_t* src_scan = GetScanline(row).subspan(3).data();
    uint8_t* dest_scan = pMask->GetWritableScanline(row).data();
    for (int col = 0; col < m_Width; ++col) {
      *dest_scan++ = *src_scan;
      src_scan += 4;
    }
  }
  return pMask;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::FlipImage(bool bXFlip, bool bYFlip) const {
  auto pFlipped = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pFlipped->Create(m_Width, m_Height, GetFormat()))
    return nullptr;

  pFlipped->SetPalette(GetPaletteSpan());
  int Bpp = GetBppFromFormat(m_Format) / 8;
  for (int row = 0; row < m_Height; ++row) {
    const uint8_t* src_scan = GetScanline(row).data();
    uint8_t* dest_scan =
        pFlipped->GetWritableScanline(bYFlip ? m_Height - row - 1 : row).data();
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
  return pFlipped;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::ConvertTo(FXDIB_Format dest_format) const {
  if (dest_format == GetFormat())
    return Realize();

  auto pClone = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pClone->Create(m_Width, m_Height, dest_format))
    return nullptr;

  if (dest_format == FXDIB_Format::kArgb) {
    if (!pClone->SetUniformOpaqueAlpha()) {
      return nullptr;
    }
  }

  RetainPtr<const CFX_DIBBase> holder(this);
  DataVector<uint32_t> pal_8bpp;
  if (!ConvertBuffer(dest_format, pClone->GetWritableBuffer(),
                     pClone->GetPitch(), m_Width, m_Height, holder, 0, 0,
                     &pal_8bpp)) {
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
  const int result_height = dest_clip.Height();
  const int result_width = dest_clip.Width();
  if (!pTransBitmap->Create(result_width, result_height, GetFormat()))
    return nullptr;

  pTransBitmap->SetPalette(GetPaletteSpan());
  const int dest_pitch = pTransBitmap->GetPitch();
  pdfium::span<uint8_t> dest_span = pTransBitmap->GetWritableBuffer().first(
      Fx2DSizeOrDie(dest_pitch, result_height));
  const size_t dest_last_row_offset =
      Fx2DSizeOrDie(dest_pitch, result_height - 1);
  const int row_start = bXFlip ? m_Height - dest_clip.right : dest_clip.left;
  const int row_end = bXFlip ? m_Height - dest_clip.left : dest_clip.right;
  const int col_start = bYFlip ? m_Width - dest_clip.bottom : dest_clip.top;
  const int col_end = bYFlip ? m_Width - dest_clip.top : dest_clip.bottom;
  if (GetBPP() == 1) {
    fxcrt::spanset(dest_span, 0xff);
    if (bYFlip)
      dest_span = dest_span.subspan(dest_last_row_offset);
    const int dest_step = bYFlip ? -dest_pitch : dest_pitch;
    for (int row = row_start; row < row_end; ++row) {
      const uint8_t* src_scan = GetScanline(row).data();
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_span.data();
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
    if (bYFlip)
      dest_span = dest_span.subspan(dest_last_row_offset);
    for (int row = row_start; row < row_end; ++row) {
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      size_t dest_offset = Fx2DSizeOrDie(dest_col, nBytes);
      uint8_t* dest_scan = dest_span.subspan(dest_offset).data();
      if (nBytes == 4) {
        const uint32_t* src_scan =
            reinterpret_cast<const uint32_t*>(GetScanline(row).data()) +
            col_start;
        for (int col = col_start; col < col_end; ++col) {
          uint32_t* dest_scan32 = reinterpret_cast<uint32_t*>(dest_scan);
          *dest_scan32 = *src_scan++;
          dest_scan += dest_step;
        }
      } else {
        const uint8_t* src_scan =
            GetScanline(row).subspan(col_start * nBytes).data();
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
  return pTransBitmap;
}

RetainPtr<CFX_DIBitmap> CFX_DIBBase::TransformTo(const CFX_Matrix& mtDest,
                                                 int* result_left,
                                                 int* result_top) const {
  RetainPtr<const CFX_DIBBase> holder(this);
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
    const FX_RECT* pClip) const {
  RetainPtr<const CFX_DIBBase> holder(this);
  FX_RECT clip_rect(0, 0, abs(dest_width), abs(dest_height));
  if (pClip)
    clip_rect.Intersect(*pClip);

  if (clip_rect.IsEmpty())
    return nullptr;

  if (dest_width == m_Width && dest_height == m_Height)
    return ClipTo(clip_rect);

  CFX_BitmapStorer storer;
  CFX_ImageStretcher stretcher(&storer, holder, dest_width, dest_height,
                               clip_rect, options);
  if (stretcher.Start())
    stretcher.Continue(nullptr);

  return storer.Detach();
}

// static
bool CFX_DIBBase::ConvertBuffer(FXDIB_Format dest_format,
                                pdfium::span<uint8_t> dest_buf,
                                int dest_pitch,
                                int width,
                                int height,
                                const RetainPtr<const CFX_DIBBase>& pSrcBitmap,
                                int src_left,
                                int src_top,
                                DataVector<uint32_t>* pal) {
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
      NOTREACHED_NORETURN();
  }
}
