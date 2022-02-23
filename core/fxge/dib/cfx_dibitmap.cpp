// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_dibitmap.h"

#include <string.h>

#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/calculate_pitch.h"
#include "core/fxge/cfx_cliprgn.h"
#include "core/fxge/dib/cfx_scanlinecompositor.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"

CFX_DIBitmap::CFX_DIBitmap() = default;

bool CFX_DIBitmap::Create(int width, int height, FXDIB_Format format) {
  return Create(width, height, format, nullptr, 0);
}

bool CFX_DIBitmap::Create(int width,
                          int height,
                          FXDIB_Format format,
                          uint8_t* pBuffer,
                          uint32_t pitch) {
  m_pBuffer = nullptr;
  m_Format = format;
  m_Width = 0;
  m_Height = 0;
  m_Pitch = 0;

  absl::optional<PitchAndSize> pitch_size =
      CalculatePitchAndSize(width, height, format, pitch);
  if (!pitch_size.has_value())
    return false;

  if (pBuffer) {
    m_pBuffer.Reset(pBuffer);
  } else {
    FX_SAFE_SIZE_T safe_buffer_size = pitch_size.value().size;
    safe_buffer_size += 4;
    if (!safe_buffer_size.IsValid())
      return false;

    m_pBuffer = std::unique_ptr<uint8_t, FxFreeDeleter>(
        FX_TryAlloc(uint8_t, safe_buffer_size.ValueOrDie()));
    if (!m_pBuffer)
      return false;
  }
  m_Width = width;
  m_Height = height;
  m_Pitch = pitch_size.value().pitch;
  if (!IsAlphaFormat() || format == FXDIB_Format::kArgb)
    return true;

  if (BuildAlphaMask())
    return true;

  if (pBuffer)
    return true;

  m_pBuffer = nullptr;
  m_Width = 0;
  m_Height = 0;
  m_Pitch = 0;
  return false;
}

bool CFX_DIBitmap::Copy(const RetainPtr<CFX_DIBBase>& pSrc) {
  if (m_pBuffer)
    return false;

  if (!Create(pSrc->GetWidth(), pSrc->GetHeight(), pSrc->GetFormat()))
    return false;

  SetPalette(pSrc->GetPaletteSpan());
  SetAlphaMask(pSrc->GetAlphaMask(), nullptr);
  for (int row = 0; row < pSrc->GetHeight(); row++) {
    memcpy(m_pBuffer.Get() + row * m_Pitch, pSrc->GetScanline(row).data(),
           m_Pitch);
  }
  return true;
}

CFX_DIBitmap::~CFX_DIBitmap() = default;

uint8_t* CFX_DIBitmap::GetBuffer() const {
  return m_pBuffer.Get();
}

pdfium::span<const uint8_t> CFX_DIBitmap::GetScanline(int line) const {
  if (!m_pBuffer)
    return pdfium::span<const uint8_t>();

  return {m_pBuffer.Get() + line * m_Pitch, m_Pitch};
}

size_t CFX_DIBitmap::GetEstimatedImageMemoryBurden() const {
  size_t result = CFX_DIBBase::GetEstimatedImageMemoryBurden();
  if (GetBuffer()) {
    int height = GetHeight();
    CHECK(pdfium::base::IsValueInRangeForNumericType<size_t>(height));
    result += static_cast<size_t>(height) * GetPitch();
  }
  return result;
}

void CFX_DIBitmap::TakeOver(RetainPtr<CFX_DIBitmap>&& pSrcBitmap) {
  m_pBuffer = std::move(pSrcBitmap->m_pBuffer);
  m_palette = std::move(pSrcBitmap->m_palette);
  m_pAlphaMask = pSrcBitmap->m_pAlphaMask;
  pSrcBitmap->m_pBuffer = nullptr;
  pSrcBitmap->m_pAlphaMask = nullptr;
  m_Format = pSrcBitmap->m_Format;
  m_Width = pSrcBitmap->m_Width;
  m_Height = pSrcBitmap->m_Height;
  m_Pitch = pSrcBitmap->m_Pitch;
}

void CFX_DIBitmap::Clear(uint32_t color) {
  if (!m_pBuffer)
    return;

  uint8_t* pBuffer = m_pBuffer.Get();
  switch (GetFormat()) {
    case FXDIB_Format::k1bppMask:
      memset(pBuffer, (color & 0xff000000) ? 0xff : 0, m_Pitch * m_Height);
      break;
    case FXDIB_Format::k1bppRgb: {
      int index = FindPalette(color);
      memset(pBuffer, index ? 0xff : 0, m_Pitch * m_Height);
      break;
    }
    case FXDIB_Format::k8bppMask:
      memset(pBuffer, color >> 24, m_Pitch * m_Height);
      break;
    case FXDIB_Format::k8bppRgb: {
      int index = FindPalette(color);
      memset(pBuffer, index, m_Pitch * m_Height);
      break;
    }
    case FXDIB_Format::kRgb: {
      int a;
      int r;
      int g;
      int b;
      std::tie(a, r, g, b) = ArgbDecode(color);
      if (r == g && g == b) {
        memset(pBuffer, r, m_Pitch * m_Height);
      } else {
        int byte_pos = 0;
        for (int col = 0; col < m_Width; col++) {
          pBuffer[byte_pos++] = b;
          pBuffer[byte_pos++] = g;
          pBuffer[byte_pos++] = r;
        }
        for (int row = 1; row < m_Height; row++) {
          memcpy(pBuffer + row * m_Pitch, pBuffer, m_Pitch);
        }
      }
      break;
    }
    case FXDIB_Format::kRgb32:
    case FXDIB_Format::kArgb: {
#if defined(_SKIA_SUPPORT_)
      if (FXDIB_Format::kRgb32 == GetFormat())
        color |= 0xFF000000;
#endif
      for (int i = 0; i < m_Width; i++)
        reinterpret_cast<uint32_t*>(pBuffer)[i] = color;
      for (int row = 1; row < m_Height; row++)
        memcpy(pBuffer + row * m_Pitch, pBuffer, m_Pitch);
      break;
    }
    default:
      break;
  }
}

bool CFX_DIBitmap::TransferBitmap(int dest_left,
                                  int dest_top,
                                  int width,
                                  int height,
                                  const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                  int src_left,
                                  int src_top) {
  if (!m_pBuffer)
    return false;

  if (!GetOverlapRect(dest_left, dest_top, width, height,
                      pSrcBitmap->GetWidth(), pSrcBitmap->GetHeight(), src_left,
                      src_top, nullptr)) {
    return true;
  }

  FXDIB_Format dest_format = GetFormat();
  FXDIB_Format src_format = pSrcBitmap->GetFormat();
  if (dest_format != src_format) {
    return TransferWithUnequalFormats(dest_format, dest_left, dest_top, width,
                                      height, pSrcBitmap, src_left, src_top);
  }

  if (GetBPP() != 1) {
    TransferWithMultipleBPP(dest_left, dest_top, width, height, pSrcBitmap,
                            src_left, src_top);
    return true;
  }

  TransferEqualFormatsOneBPP(dest_left, dest_top, width, height, pSrcBitmap,
                             src_left, src_top);
  return true;
}

bool CFX_DIBitmap::TransferWithUnequalFormats(
    FXDIB_Format dest_format,
    int dest_left,
    int dest_top,
    int width,
    int height,
    const RetainPtr<CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top) {
  if (HasPalette())
    return false;

  if (GetBppFromFormat(m_Format) == 8)
    dest_format = FXDIB_Format::k8bppMask;

  FX_SAFE_UINT32 offset = dest_left;
  offset *= GetBPP();
  offset /= 8;
  if (!offset.IsValid())
    return false;

  uint8_t* dest_buf =
      m_pBuffer.Get() + dest_top * m_Pitch + offset.ValueOrDie();
  std::vector<uint32_t, FxAllocAllocator<uint32_t>> d_plt;
  return ConvertBuffer(dest_format, dest_buf, m_Pitch, width, height,
                       pSrcBitmap, src_left, src_top, &d_plt);
}

void CFX_DIBitmap::TransferWithMultipleBPP(
    int dest_left,
    int dest_top,
    int width,
    int height,
    const RetainPtr<CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top) {
  int Bpp = GetBPP() / 8;
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan =
        m_pBuffer.Get() + (dest_top + row) * m_Pitch + dest_left * Bpp;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left * Bpp).data();
    memcpy(dest_scan, src_scan, width * Bpp);
  }
}

void CFX_DIBitmap::TransferEqualFormatsOneBPP(
    int dest_left,
    int dest_top,
    int width,
    int height,
    const RetainPtr<CFX_DIBBase>& pSrcBitmap,
    int src_left,
    int src_top) {
  for (int row = 0; row < height; ++row) {
    uint8_t* dest_scan = m_pBuffer.Get() + (dest_top + row) * m_Pitch;
    const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row).data();
    for (int col = 0; col < width; ++col) {
      int src_idx = src_left + col;
      int dest_idx = dest_left + col;
      if (src_scan[(src_idx) / 8] & (1 << (7 - (src_idx) % 8)))
        dest_scan[(dest_idx) / 8] |= 1 << (7 - (dest_idx) % 8);
      else
        dest_scan[(dest_idx) / 8] &= ~(1 << (7 - (dest_idx) % 8));
    }
  }
}

bool CFX_DIBitmap::SetChannelFromBitmap(
    Channel destChannel,
    const RetainPtr<CFX_DIBBase>& pSrcBitmap) {
  if (!m_pBuffer)
    return false;

  RetainPtr<CFX_DIBBase> pSrcClone = pSrcBitmap;
  if (!pSrcBitmap->IsAlphaFormat() && !pSrcBitmap->IsMaskFormat())
    return false;

  if (pSrcBitmap->GetBPP() == 1) {
    pSrcClone = pSrcBitmap->ConvertTo(FXDIB_Format::k8bppMask);
    if (!pSrcClone)
      return false;
  }
  int srcOffset = pSrcBitmap->GetFormat() == FXDIB_Format::kArgb ? 3 : 0;
  int destOffset = 0;
  if (destChannel == Channel::kAlpha) {
    if (IsMaskFormat()) {
      if (!ConvertFormat(FXDIB_Format::k8bppMask))
        return false;
    } else {
      if (!ConvertFormat(FXDIB_Format::kArgb))
        return false;

      destOffset = 3;
    }
  } else {
    DCHECK_EQ(destChannel, Channel::kRed);
    if (IsMaskFormat())
      return false;

    if (GetBPP() < 24) {
      if (IsAlphaFormat()) {
        if (!ConvertFormat(FXDIB_Format::kArgb))
          return false;
      } else {
        if (!ConvertFormat(kPlatformRGBFormat))
          return false;
      }
    }
    destOffset = 2;
  }
  if (pSrcClone->HasAlphaMask()) {
    RetainPtr<CFX_DIBBase> pAlphaMask = pSrcClone->GetAlphaMask();
    if (pSrcClone->GetWidth() != m_Width ||
        pSrcClone->GetHeight() != m_Height) {
      if (pAlphaMask) {
        pAlphaMask = pAlphaMask->StretchTo(m_Width, m_Height,
                                           FXDIB_ResampleOptions(), nullptr);
        if (!pAlphaMask)
          return false;
      }
    }
    pSrcClone = std::move(pAlphaMask);
    srcOffset = 0;
  } else if (pSrcClone->GetWidth() != m_Width ||
             pSrcClone->GetHeight() != m_Height) {
    RetainPtr<CFX_DIBitmap> pSrcMatched = pSrcClone->StretchTo(
        m_Width, m_Height, FXDIB_ResampleOptions(), nullptr);
    if (!pSrcMatched)
      return false;

    pSrcClone = pSrcMatched;
  }
  RetainPtr<CFX_DIBitmap> pDst(this);
  if (destChannel == Channel::kAlpha && m_pAlphaMask) {
    pDst = m_pAlphaMask;
    destOffset = 0;
  }
  int srcBytes = pSrcClone->GetBPP() / 8;
  int destBytes = pDst->GetBPP() / 8;
  for (int row = 0; row < m_Height; row++) {
    uint8_t* dest_pos =
        pDst->GetWritableScanline(row).subspan(destOffset).data();
    const uint8_t* src_pos =
        pSrcClone->GetScanline(row).subspan(srcOffset).data();
    for (int col = 0; col < m_Width; col++) {
      *dest_pos = *src_pos;
      dest_pos += destBytes;
      src_pos += srcBytes;
    }
  }
  return true;
}

bool CFX_DIBitmap::SetAlphaFromBitmap(
    const RetainPtr<CFX_DIBBase>& pSrcBitmap) {
  return SetChannelFromBitmap(Channel::kAlpha, pSrcBitmap);
}

bool CFX_DIBitmap::SetRedFromBitmap(const RetainPtr<CFX_DIBBase>& pSrcBitmap) {
  return SetChannelFromBitmap(Channel::kRed, pSrcBitmap);
}

bool CFX_DIBitmap::SetUniformOpaqueAlpha() {
  if (!m_pBuffer)
    return false;

  if (IsMaskFormat()) {
    if (!ConvertFormat(FXDIB_Format::k8bppMask))
      return false;
  } else {
    if (!ConvertFormat(FXDIB_Format::kArgb))
      return false;
  }
  const int Bpp = GetBPP() / 8;
  if (Bpp == 1) {
    memset(m_pBuffer.Get(), 0xff, m_Height * m_Pitch);
    return true;
  }
  if (m_pAlphaMask) {
    memset(m_pAlphaMask->GetBuffer(), 0xff,
           m_pAlphaMask->GetHeight() * m_pAlphaMask->GetPitch());
    return true;
  }
  const int destOffset = GetFormat() == FXDIB_Format::kArgb ? 3 : 0;
  for (int row = 0; row < m_Height; row++) {
    uint8_t* scan_line = m_pBuffer.Get() + row * m_Pitch + destOffset;
    for (int col = 0; col < m_Width; col++) {
      *scan_line = 0xff;
      scan_line += Bpp;
    }
  }
  return true;
}

bool CFX_DIBitmap::MultiplyAlpha(const RetainPtr<CFX_DIBBase>& pSrcBitmap) {
  if (!m_pBuffer)
    return false;

  if (!pSrcBitmap->IsMaskFormat()) {
    NOTREACHED();
    return false;
  }

  if (IsOpaqueImage())
    return SetAlphaFromBitmap(pSrcBitmap);

  RetainPtr<CFX_DIBitmap> pSrcClone = pSrcBitmap.As<CFX_DIBitmap>();
  if (pSrcBitmap->GetWidth() != m_Width ||
      pSrcBitmap->GetHeight() != m_Height) {
    pSrcClone = pSrcBitmap->StretchTo(m_Width, m_Height,
                                      FXDIB_ResampleOptions(), nullptr);
    if (!pSrcClone)
      return false;
  }
  if (IsMaskFormat()) {
    if (!ConvertFormat(FXDIB_Format::k8bppMask))
      return false;

    for (int row = 0; row < m_Height; row++) {
      uint8_t* dest_scan = m_pBuffer.Get() + m_Pitch * row;
      uint8_t* src_scan = pSrcClone->m_pBuffer.Get() + pSrcClone->m_Pitch * row;
      if (pSrcClone->GetBPP() == 1) {
        for (int col = 0; col < m_Width; col++) {
          if (!((1 << (7 - col % 8)) & src_scan[col / 8]))
            dest_scan[col] = 0;
        }
      } else {
        for (int col = 0; col < m_Width; col++) {
          *dest_scan = (*dest_scan) * src_scan[col] / 255;
          dest_scan++;
        }
      }
    }
  } else {
    if (GetFormat() == FXDIB_Format::kArgb) {
      if (pSrcClone->GetBPP() == 1)
        return false;

      for (int row = 0; row < m_Height; row++) {
        uint8_t* dest_scan = m_pBuffer.Get() + m_Pitch * row + 3;
        uint8_t* src_scan =
            pSrcClone->m_pBuffer.Get() + pSrcClone->m_Pitch * row;
        for (int col = 0; col < m_Width; col++) {
          *dest_scan = (*dest_scan) * src_scan[col] / 255;
          dest_scan += 4;
        }
      }
    } else {
      m_pAlphaMask->MultiplyAlpha(pSrcClone);
    }
  }
  return true;
}

bool CFX_DIBitmap::MultiplyAlpha(int alpha) {
  if (!m_pBuffer)
    return false;

  switch (GetFormat()) {
    case FXDIB_Format::k1bppMask:
      if (!ConvertFormat(FXDIB_Format::k8bppMask)) {
        return false;
      }
      MultiplyAlpha(alpha);
      break;
    case FXDIB_Format::k8bppMask: {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* scan_line = m_pBuffer.Get() + row * m_Pitch;
        for (int col = 0; col < m_Width; col++) {
          scan_line[col] = scan_line[col] * alpha / 255;
        }
      }
      break;
    }
    case FXDIB_Format::kArgb: {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* scan_line = m_pBuffer.Get() + row * m_Pitch + 3;
        for (int col = 0; col < m_Width; col++) {
          *scan_line = (*scan_line) * alpha / 255;
          scan_line += 4;
        }
      }
      break;
    }
    default:
      if (IsAlphaFormat()) {
        m_pAlphaMask->MultiplyAlpha(alpha);
      } else {
        if (!ConvertFormat(FXDIB_Format::kArgb)) {
          return false;
        }
        MultiplyAlpha(alpha);
      }
      break;
  }
  return true;
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
uint32_t CFX_DIBitmap::GetPixel(int x, int y) const {
  if (!m_pBuffer)
    return 0;

  FX_SAFE_UINT32 offset = x;
  offset *= GetBPP();
  offset /= 8;
  if (!offset.IsValid())
    return 0;

  uint8_t* pos = m_pBuffer.Get() + y * m_Pitch + offset.ValueOrDie();
  switch (GetFormat()) {
    case FXDIB_Format::k1bppMask: {
      if ((*pos) & (1 << (7 - x % 8))) {
        return 0xff000000;
      }
      return 0;
    }
    case FXDIB_Format::k1bppRgb: {
      if ((*pos) & (1 << (7 - x % 8))) {
        return HasPalette() ? GetPaletteSpan()[1] : 0xffffffff;
      }
      return HasPalette() ? GetPaletteSpan()[0] : 0xff000000;
    }
    case FXDIB_Format::k8bppMask:
      return (*pos) << 24;
    case FXDIB_Format::k8bppRgb:
      return HasPalette() ? GetPaletteSpan()[*pos]
                          : ArgbEncode(0xff, *pos, *pos, *pos);
    case FXDIB_Format::kRgb:
    case FXDIB_Format::kRgb32:
      return FXARGB_GETDIB(pos) | 0xff000000;
    case FXDIB_Format::kArgb:
      return FXARGB_GETDIB(pos);
    default:
      break;
  }
  return 0;
}
#endif  // defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)

#if defined(_SKIA_SUPPORT_)
void CFX_DIBitmap::SetPixel(int x, int y, uint32_t color) {
  if (!m_pBuffer)
    return;

  if (x < 0 || x >= m_Width || y < 0 || y >= m_Height)
    return;

  FX_SAFE_UINT32 offset = x;
  offset *= GetBPP();
  offset /= 8;
  if (!offset.IsValid())
    return;

  uint8_t* pos = m_pBuffer.Get() + y * m_Pitch + offset.ValueOrDie();
  switch (GetFormat()) {
    case FXDIB_Format::k1bppMask:
      if (color >> 24) {
        *pos |= 1 << (7 - x % 8);
      } else {
        *pos &= ~(1 << (7 - x % 8));
      }
      break;
    case FXDIB_Format::k1bppRgb:
      if (HasPalette()) {
        if (color == GetPaletteSpan()[1]) {
          *pos |= 1 << (7 - x % 8);
        } else {
          *pos &= ~(1 << (7 - x % 8));
        }
      } else {
        if (color == 0xffffffff) {
          *pos |= 1 << (7 - x % 8);
        } else {
          *pos &= ~(1 << (7 - x % 8));
        }
      }
      break;
    case FXDIB_Format::k8bppMask:
      *pos = (uint8_t)(color >> 24);
      break;
    case FXDIB_Format::k8bppRgb: {
      if (HasPalette()) {
        pdfium::span<const uint32_t> palette = GetPaletteSpan();
        for (int i = 0; i < 256; i++) {
          if (palette[i] == color) {
            *pos = (uint8_t)i;
            return;
          }
        }
        *pos = 0;
      } else {
        *pos = FXRGB2GRAY(FXARGB_R(color), FXARGB_G(color), FXARGB_B(color));
      }
      break;
    }
    case FXDIB_Format::kRgb:
    case FXDIB_Format::kRgb32: {
      int alpha = FXARGB_A(color);
      pos[0] = (FXARGB_B(color) * alpha + pos[0] * (255 - alpha)) / 255;
      pos[1] = (FXARGB_G(color) * alpha + pos[1] * (255 - alpha)) / 255;
      pos[2] = (FXARGB_R(color) * alpha + pos[2] * (255 - alpha)) / 255;
      break;
    }
    case FXDIB_Format::kArgb:
      FXARGB_SETDIB(pos, color);
      break;
    default:
      break;
  }
}
#endif  // defined(_SKIA_SUPPORT_)

void CFX_DIBitmap::ConvertBGRColorScale(uint32_t forecolor,
                                        uint32_t backcolor) {
  int fr = FXSYS_GetRValue(forecolor);
  int fg = FXSYS_GetGValue(forecolor);
  int fb = FXSYS_GetBValue(forecolor);
  int br = FXSYS_GetRValue(backcolor);
  int bg = FXSYS_GetGValue(backcolor);
  int bb = FXSYS_GetBValue(backcolor);
  if (GetBppFromFormat(m_Format) <= 8) {
    if (forecolor == 0 && backcolor == 0xffffff && !HasPalette())
      return;

    BuildPalette();
    int size = 1 << GetBppFromFormat(m_Format);
    for (int i = 0; i < size; ++i) {
      int gray = FXRGB2GRAY(FXARGB_R(m_palette[i]), FXARGB_G(m_palette[i]),
                            FXARGB_B(m_palette[i]));
      m_palette[i] =
          ArgbEncode(0xff, br + (fr - br) * gray / 255,
                     bg + (fg - bg) * gray / 255, bb + (fb - bb) * gray / 255);
    }
    return;
  }
  if (forecolor == 0 && backcolor == 0xffffff) {
    for (int row = 0; row < m_Height; ++row) {
      uint8_t* scanline = m_pBuffer.Get() + row * m_Pitch;
      int gap = GetBppFromFormat(m_Format) / 8 - 2;
      for (int col = 0; col < m_Width; ++col) {
        int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
        *scanline++ = gray;
        *scanline++ = gray;
        *scanline = gray;
        scanline += gap;
      }
    }
    return;
  }
  for (int row = 0; row < m_Height; ++row) {
    uint8_t* scanline = m_pBuffer.Get() + row * m_Pitch;
    int gap = GetBppFromFormat(m_Format) / 8 - 2;
    for (int col = 0; col < m_Width; ++col) {
      int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
      *scanline++ = bb + (fb - bb) * gray / 255;
      *scanline++ = bg + (fg - bg) * gray / 255;
      *scanline = br + (fr - br) * gray / 255;
      scanline += gap;
    }
  }
}

bool CFX_DIBitmap::ConvertColorScale(uint32_t forecolor, uint32_t backcolor) {
  if (!m_pBuffer || IsMaskFormat())
    return false;

  ConvertBGRColorScale(forecolor, backcolor);
  return true;
}

// static
absl::optional<CFX_DIBitmap::PitchAndSize> CFX_DIBitmap::CalculatePitchAndSize(
    int width,
    int height,
    FXDIB_Format format,
    uint32_t pitch) {
  if (width <= 0 || height <= 0)
    return absl::nullopt;

  int bpp = GetBppFromFormat(format);
  if (!bpp)
    return absl::nullopt;

  uint32_t actual_pitch = pitch;
  if (actual_pitch == 0) {
    FX_SAFE_UINT32 safe_pitch = width;
    safe_pitch *= bpp;
    safe_pitch += 31;
    // Note: This is not the same as /8 due to truncation.
    safe_pitch /= 32;
    safe_pitch *= 4;
    if (!safe_pitch.IsValid())
      return absl::nullopt;

    actual_pitch = safe_pitch.ValueOrDie();
  }

  FX_SAFE_UINT32 safe_size = actual_pitch;
  safe_size *= height;
  if (!safe_size.IsValid())
    return absl::nullopt;

  return PitchAndSize{actual_pitch, safe_size.ValueOrDie()};
}

bool CFX_DIBitmap::CompositeBitmap(int dest_left,
                                   int dest_top,
                                   int width,
                                   int height,
                                   const RetainPtr<CFX_DIBBase>& pSrcBitmap,
                                   int src_left,
                                   int src_top,
                                   BlendMode blend_type,
                                   const CFX_ClipRgn* pClipRgn,
                                   bool bRgbByteOrder) {
  if (pSrcBitmap->IsMaskFormat()) {
    // Should have called CompositeMask().
    NOTREACHED();
    return false;
  }

  if (!m_pBuffer)
    return false;

  if (GetBppFromFormat(m_Format) < 8)
    return false;

  if (!GetOverlapRect(dest_left, dest_top, width, height,
                      pSrcBitmap->GetWidth(), pSrcBitmap->GetHeight(), src_left,
                      src_top, pClipRgn)) {
    return true;
  }

  RetainPtr<CFX_DIBitmap> pClipMask;
  FX_RECT clip_box;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::kRectI) {
    pClipMask = pClipRgn->GetMask();
    clip_box = pClipRgn->GetBox();
  }
  CFX_ScanlineCompositor compositor;
  if (!compositor.Init(GetFormat(), pSrcBitmap->GetFormat(), width,
                       pSrcBitmap->GetPaletteSpan(), 0, blend_type,
                       pClipMask != nullptr, bRgbByteOrder)) {
    return false;
  }
  const int dest_Bpp = GetBppFromFormat(m_Format) / 8;
  const int src_Bpp = pSrcBitmap->GetBPP() / 8;
  const bool bRgb = src_Bpp > 1;
  if (!bRgb && !pSrcBitmap->HasPalette())
    return false;

  RetainPtr<CFX_DIBitmap> pSrcAlphaMask = pSrcBitmap->GetAlphaMask();
  for (int row = 0; row < height; row++) {
    pdfium::span<uint8_t> dest_scan =
        GetWritableScanline(dest_top + row).subspan(dest_left * dest_Bpp);
    pdfium::span<const uint8_t> src_scan =
        pSrcBitmap->GetScanline(src_top + row).subspan(src_left * src_Bpp);
    pdfium::span<const uint8_t> src_scan_extra_alpha =
        pSrcAlphaMask
            ? pSrcAlphaMask->GetScanline(src_top + row).subspan(src_left)
            : pdfium::span<const uint8_t>();
    pdfium::span<uint8_t> dst_scan_extra_alpha =
        m_pAlphaMask ? m_pAlphaMask->GetWritableScanline(dest_top + row)
                           .subspan(dest_left)
                     : pdfium::span<uint8_t>();
    pdfium::span<const uint8_t> clip_scan;
    if (pClipMask) {
      clip_scan = pClipMask->GetWritableScanline(dest_top + row - clip_box.top)
                      .subspan(dest_left - clip_box.left);
    }
    if (bRgb) {
      compositor.CompositeRgbBitmapLine(dest_scan, src_scan, width, clip_scan,
                                        src_scan_extra_alpha,
                                        dst_scan_extra_alpha);
    } else {
      compositor.CompositePalBitmapLine(dest_scan, src_scan, src_left, width,
                                        clip_scan, src_scan_extra_alpha,
                                        dst_scan_extra_alpha);
    }
  }
  return true;
}

bool CFX_DIBitmap::CompositeMask(int dest_left,
                                 int dest_top,
                                 int width,
                                 int height,
                                 const RetainPtr<CFX_DIBBase>& pMask,
                                 uint32_t color,
                                 int src_left,
                                 int src_top,
                                 BlendMode blend_type,
                                 const CFX_ClipRgn* pClipRgn,
                                 bool bRgbByteOrder) {
  if (!pMask->IsMaskFormat()) {
    // Should have called CompositeBitmap().
    NOTREACHED();
    return false;
  }

  if (!m_pBuffer)
    return false;

  if (GetBppFromFormat(m_Format) < 8)
    return false;

  if (!GetOverlapRect(dest_left, dest_top, width, height, pMask->GetWidth(),
                      pMask->GetHeight(), src_left, src_top, pClipRgn)) {
    return true;
  }

  int src_alpha = FXARGB_A(color);
  if (src_alpha == 0)
    return true;

  RetainPtr<CFX_DIBitmap> pClipMask;
  FX_RECT clip_box;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::kRectI) {
    pClipMask = pClipRgn->GetMask();
    clip_box = pClipRgn->GetBox();
  }
  int src_bpp = pMask->GetBPP();
  int Bpp = GetBPP() / 8;
  CFX_ScanlineCompositor compositor;
  if (!compositor.Init(GetFormat(), pMask->GetFormat(), width, {}, color,
                       blend_type, pClipMask != nullptr, bRgbByteOrder)) {
    return false;
  }
  for (int row = 0; row < height; row++) {
    pdfium::span<uint8_t> dest_scan =
        GetWritableScanline(dest_top + row).subspan(dest_left * Bpp);
    pdfium::span<const uint8_t> src_scan = pMask->GetScanline(src_top + row);
    pdfium::span<uint8_t> dst_scan_extra_alpha =
        m_pAlphaMask ? m_pAlphaMask->GetWritableScanline(dest_top + row)
                           .subspan(dest_left)
                     : pdfium::span<uint8_t>();
    pdfium::span<const uint8_t> clip_scan;
    if (pClipMask) {
      clip_scan = pClipMask->GetScanline(dest_top + row - clip_box.top)
                      .subspan(dest_left - clip_box.left);
    }
    if (src_bpp == 1) {
      compositor.CompositeBitMaskLine(dest_scan, src_scan, src_left, width,
                                      clip_scan, dst_scan_extra_alpha);
    } else {
      compositor.CompositeByteMaskLine(dest_scan, src_scan.subspan(src_left),
                                       width, clip_scan, dst_scan_extra_alpha);
    }
  }
  return true;
}

bool CFX_DIBitmap::CompositeRect(int left,
                                 int top,
                                 int width,
                                 int height,
                                 uint32_t color) {
  if (!m_pBuffer)
    return false;

  int src_alpha = FXARGB_A(color);
  if (src_alpha == 0)
    return true;

  FX_RECT rect(left, top, left + width, top + height);
  rect.Intersect(0, 0, m_Width, m_Height);
  if (rect.IsEmpty())
    return true;

  width = rect.Width();
  uint32_t dst_color = color;
  uint8_t* color_p = reinterpret_cast<uint8_t*>(&dst_color);
  if (GetBppFromFormat(m_Format) == 8) {
    uint8_t gray = IsMaskFormat() ? 255
                                  : (uint8_t)FXRGB2GRAY((int)color_p[2],
                                                        color_p[1], color_p[0]);
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan = m_pBuffer.Get() + row * m_Pitch + rect.left;
      if (src_alpha == 255) {
        memset(dest_scan, gray, width);
      } else {
        for (int col = 0; col < width; col++) {
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, src_alpha);
          dest_scan++;
        }
      }
    }
    return true;
  }
  if (GetBppFromFormat(m_Format) == 1) {
    int left_shift = rect.left % 8;
    int right_shift = rect.right % 8;
    int new_width = rect.right / 8 - rect.left / 8;
    int index = 0;
    if (HasPalette()) {
      for (int i = 0; i < 2; i++) {
        if (GetPaletteSpan()[i] == color)
          index = i;
      }
    } else {
      index = (static_cast<uint8_t>(color) == 0xff) ? 1 : 0;
    }
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan_top =
          GetWritableScanline(row).subspan(rect.left / 8).data();
      uint8_t* dest_scan_top_r =
          GetWritableScanline(row).subspan(rect.right / 8).data();
      uint8_t left_flag = *dest_scan_top & (255 << (8 - left_shift));
      uint8_t right_flag = *dest_scan_top_r & (255 >> right_shift);
      if (new_width) {
        memset(dest_scan_top + 1, index ? 255 : 0, new_width - 1);
        if (!index) {
          *dest_scan_top &= left_flag;
          *dest_scan_top_r &= right_flag;
        } else {
          *dest_scan_top |= ~left_flag;
          *dest_scan_top_r |= ~right_flag;
        }
      } else {
        if (!index) {
          *dest_scan_top &= left_flag | right_flag;
        } else {
          *dest_scan_top |= ~(left_flag | right_flag);
        }
      }
    }
    return true;
  }

  if (GetBppFromFormat(m_Format) < 24) {
    NOTREACHED();
    return false;
  }

  color_p[3] = static_cast<uint8_t>(src_alpha);
  int Bpp = GetBppFromFormat(m_Format) / 8;
  bool bAlpha = IsAlphaFormat();
  bool bArgb = GetFormat() == FXDIB_Format::kArgb;
  if (src_alpha == 255) {
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan = m_pBuffer.Get() + row * m_Pitch + rect.left * Bpp;
      uint8_t* dest_scan_alpha =
          m_pAlphaMask
              ? m_pAlphaMask->GetWritableScanline(row).subspan(rect.left).data()
              : nullptr;
      if (dest_scan_alpha)
        memset(dest_scan_alpha, 0xff, width);

      if (Bpp == 4) {
        uint32_t* scan = reinterpret_cast<uint32_t*>(dest_scan);
        for (int col = 0; col < width; col++)
          *scan++ = dst_color;
      } else {
        for (int col = 0; col < width; col++) {
          *dest_scan++ = color_p[0];
          *dest_scan++ = color_p[1];
          *dest_scan++ = color_p[2];
        }
      }
    }
    return true;
  }
  for (int row = rect.top; row < rect.bottom; row++) {
    uint8_t* dest_scan = m_pBuffer.Get() + row * m_Pitch + rect.left * Bpp;
    if (bAlpha) {
      if (bArgb) {
        for (int col = 0; col < width; col++) {
          uint8_t back_alpha = dest_scan[3];
          if (back_alpha == 0) {
            FXARGB_SETDIB(dest_scan, ArgbEncode(src_alpha, color_p[2],
                                                color_p[1], color_p[0]));
            dest_scan += 4;
            continue;
          }
          uint8_t dest_alpha =
              back_alpha + src_alpha - back_alpha * src_alpha / 255;
          int alpha_ratio = src_alpha * 255 / dest_alpha;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, color_p[0], alpha_ratio);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, color_p[1], alpha_ratio);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, color_p[2], alpha_ratio);
          dest_scan++;
          *dest_scan++ = dest_alpha;
        }
      } else {
        uint8_t* dest_scan_alpha =
            m_pAlphaMask->GetWritableScanline(row).subspan(rect.left).data();
        for (int col = 0; col < width; col++) {
          uint8_t back_alpha = *dest_scan_alpha;
          if (back_alpha == 0) {
            *dest_scan_alpha++ = src_alpha;
            memcpy(dest_scan, color_p, Bpp);
            dest_scan += Bpp;
            continue;
          }
          uint8_t dest_alpha =
              back_alpha + src_alpha - back_alpha * src_alpha / 255;
          *dest_scan_alpha++ = dest_alpha;
          int alpha_ratio = src_alpha * 255 / dest_alpha;
          for (int comps = 0; comps < Bpp; comps++) {
            *dest_scan =
                FXDIB_ALPHA_MERGE(*dest_scan, color_p[comps], alpha_ratio);
            dest_scan++;
          }
        }
      }
    } else {
      for (int col = 0; col < width; col++) {
        for (int comps = 0; comps < Bpp; comps++) {
          if (comps == 3) {
            *dest_scan++ = 255;
            continue;
          }
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, color_p[comps], src_alpha);
          dest_scan++;
        }
      }
    }
  }
  return true;
}

bool CFX_DIBitmap::ConvertFormat(FXDIB_Format dest_format) {
  if (dest_format == m_Format)
    return true;

  if (dest_format == FXDIB_Format::k8bppMask &&
      m_Format == FXDIB_Format::k8bppRgb && !HasPalette()) {
    m_Format = FXDIB_Format::k8bppMask;
    return true;
  }
  if (dest_format == FXDIB_Format::kArgb && m_Format == FXDIB_Format::kRgb32) {
    m_Format = FXDIB_Format::kArgb;
    for (int row = 0; row < m_Height; row++) {
      uint8_t* scanline = m_pBuffer.Get() + row * m_Pitch + 3;
      for (int col = 0; col < m_Width; col++) {
        *scanline = 0xff;
        scanline += 4;
      }
    }
    return true;
  }
  int dest_bpp = GetBppFromFormat(dest_format);
  int dest_pitch = fxge::CalculatePitch32OrDie(dest_bpp, m_Width);
  std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf(
      FX_TryAlloc(uint8_t, dest_pitch * m_Height + 4));
  if (!dest_buf)
    return false;

  RetainPtr<CFX_DIBitmap> pAlphaMask;
  if (dest_format == FXDIB_Format::kArgb) {
    memset(dest_buf.get(), 0xff, dest_pitch * m_Height + 4);
    if (m_pAlphaMask) {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* pDstScanline = dest_buf.get() + row * dest_pitch + 3;
        const uint8_t* pSrcScanline = m_pAlphaMask->GetScanline(row).data();
        for (int col = 0; col < m_Width; col++) {
          *pDstScanline = *pSrcScanline++;
          pDstScanline += 4;
        }
      }
    }
  } else if (GetIsAlphaFromFormat(dest_format)) {
    if (m_Format == FXDIB_Format::kArgb) {
      pAlphaMask = CloneAlphaMask();
      if (!pAlphaMask)
        return false;
    } else {
      if (!m_pAlphaMask) {
        if (!BuildAlphaMask())
          return false;
        pAlphaMask = std::move(m_pAlphaMask);
      } else {
        pAlphaMask = m_pAlphaMask;
      }
    }
  }
  bool ret = false;
  RetainPtr<CFX_DIBBase> holder(this);
  std::vector<uint32_t, FxAllocAllocator<uint32_t>> pal_8bpp;
  ret = ConvertBuffer(dest_format, dest_buf.get(), dest_pitch, m_Width,
                      m_Height, holder, 0, 0, &pal_8bpp);
  if (!ret)
    return false;

  m_pAlphaMask = pAlphaMask;
  m_palette = std::move(pal_8bpp);
  m_pBuffer = std::move(dest_buf);
  m_Format = dest_format;
  m_Pitch = dest_pitch;
  return true;
}
