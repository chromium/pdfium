// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/fx_dib.h"

#include <limits.h>

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcodec/fx_codec.h"
#include "core/fxcrt/cfx_maybe_owned.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/dib/cfx_filtereddib.h"
#include "core/fxge/dib/dib_int.h"
#include "core/fxge/ge/cfx_cliprgn.h"
#include "third_party/base/ptr_util.h"

const int16_t SDP_Table[513] = {
    256, 256, 256, 256, 256, 256, 256, 256, 256, 255, 255, 255, 255, 255, 255,
    254, 254, 254, 254, 253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250,
    249, 249, 249, 248, 248, 247, 247, 246, 246, 245, 244, 244, 243, 243, 242,
    242, 241, 240, 240, 239, 238, 238, 237, 236, 236, 235, 234, 233, 233, 232,
    231, 230, 230, 229, 228, 227, 226, 226, 225, 224, 223, 222, 221, 220, 219,
    218, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205,
    204, 203, 202, 201, 200, 199, 198, 196, 195, 194, 193, 192, 191, 190, 189,
    188, 186, 185, 184, 183, 182, 181, 179, 178, 177, 176, 175, 173, 172, 171,
    170, 169, 167, 166, 165, 164, 162, 161, 160, 159, 157, 156, 155, 154, 152,
    151, 150, 149, 147, 146, 145, 143, 142, 141, 140, 138, 137, 136, 134, 133,
    132, 130, 129, 128, 126, 125, 124, 122, 121, 120, 119, 117, 116, 115, 113,
    112, 111, 109, 108, 107, 105, 104, 103, 101, 100, 99,  97,  96,  95,  93,
    92,  91,  89,  88,  87,  85,  84,  83,  81,  80,  79,  77,  76,  75,  73,
    72,  71,  69,  68,  67,  66,  64,  63,  62,  60,  59,  58,  57,  55,  54,
    53,  52,  50,  49,  48,  47,  45,  44,  43,  42,  40,  39,  38,  37,  36,
    34,  33,  32,  31,  30,  28,  27,  26,  25,  24,  23,  21,  20,  19,  18,
    17,  16,  15,  14,  13,  11,  10,  9,   8,   7,   6,   5,   4,   3,   2,
    1,   0,   0,   -1,  -2,  -3,  -4,  -5,  -6,  -7,  -7,  -8,  -9,  -10, -11,
    -12, -12, -13, -14, -15, -15, -16, -17, -17, -18, -19, -19, -20, -21, -21,
    -22, -22, -23, -24, -24, -25, -25, -26, -26, -27, -27, -27, -28, -28, -29,
    -29, -30, -30, -30, -31, -31, -31, -32, -32, -32, -33, -33, -33, -33, -34,
    -34, -34, -34, -35, -35, -35, -35, -35, -36, -36, -36, -36, -36, -36, -36,
    -36, -36, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37,
    -37, -37, -37, -37, -37, -37, -37, -37, -36, -36, -36, -36, -36, -36, -36,
    -36, -36, -35, -35, -35, -35, -35, -35, -34, -34, -34, -34, -34, -33, -33,
    -33, -33, -33, -32, -32, -32, -32, -31, -31, -31, -31, -30, -30, -30, -30,
    -29, -29, -29, -29, -28, -28, -28, -27, -27, -27, -27, -26, -26, -26, -25,
    -25, -25, -24, -24, -24, -23, -23, -23, -22, -22, -22, -22, -21, -21, -21,
    -20, -20, -20, -19, -19, -19, -18, -18, -18, -17, -17, -17, -16, -16, -16,
    -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -12, -11, -11, -11,
    -10, -10, -10, -9,  -9,  -9,  -9,  -8,  -8,  -8,  -7,  -7,  -7,  -7,  -6,
    -6,  -6,  -6,  -5,  -5,  -5,  -5,  -4,  -4,  -4,  -4,  -3,  -3,  -3,  -3,
    -3,  -2,  -2,  -2,  -2,  -2,  -1,  -1,  -1,  -1,  -1,  -1,  0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,
};

FX_RECT FXDIB_SwapClipBox(FX_RECT& clip,
                          int width,
                          int height,
                          bool bFlipX,
                          bool bFlipY) {
  FX_RECT rect;
  if (bFlipY) {
    rect.left = height - clip.top;
    rect.right = height - clip.bottom;
  } else {
    rect.left = clip.top;
    rect.right = clip.bottom;
  }
  if (bFlipX) {
    rect.top = width - clip.left;
    rect.bottom = width - clip.right;
  } else {
    rect.top = clip.left;
    rect.bottom = clip.right;
  }
  rect.Normalize();
  return rect;
}

void CmykDecode(uint32_t cmyk, int& c, int& m, int& y, int& k) {
  c = FXSYS_GetCValue(cmyk);
  m = FXSYS_GetMValue(cmyk);
  y = FXSYS_GetYValue(cmyk);
  k = FXSYS_GetKValue(cmyk);
}

void ArgbDecode(uint32_t argb, int& a, int& r, int& g, int& b) {
  a = FXARGB_A(argb);
  r = FXARGB_R(argb);
  g = FXARGB_G(argb);
  b = FXARGB_B(argb);
}

void ArgbDecode(uint32_t argb, int& a, FX_COLORREF& rgb) {
  a = FXARGB_A(argb);
  rgb = FXSYS_RGB(FXARGB_R(argb), FXARGB_G(argb), FXARGB_B(argb));
}

uint32_t ArgbEncode(int a, FX_COLORREF rgb) {
  return FXARGB_MAKE(a, FXSYS_GetRValue(rgb), FXSYS_GetGValue(rgb),
                     FXSYS_GetBValue(rgb));
}

CFX_DIBitmap::CFX_DIBitmap() {
  m_bExtBuf = false;
  m_pBuffer = nullptr;
  m_pPalette = nullptr;
#ifdef _SKIA_SUPPORT_PATHS_
  m_nFormat = Format::kCleared;
#endif
}

#define _MAX_OOM_LIMIT_ 12000000
bool CFX_DIBitmap::Create(int width,
                          int height,
                          FXDIB_Format format,
                          uint8_t* pBuffer,
                          int pitch) {
  m_pBuffer = nullptr;
  m_bpp = (uint8_t)format;
  m_AlphaFlag = (uint8_t)(format >> 8);
  m_Width = m_Height = m_Pitch = 0;
  if (width <= 0 || height <= 0 || pitch < 0) {
    return false;
  }
  if ((INT_MAX - 31) / width < (format & 0xff)) {
    return false;
  }
  if (!pitch) {
    pitch = (width * (format & 0xff) + 31) / 32 * 4;
  }
  if ((1 << 30) / pitch < height) {
    return false;
  }
  if (pBuffer) {
    m_pBuffer = pBuffer;
    m_bExtBuf = true;
  } else {
    int size = pitch * height + 4;
    int oomlimit = _MAX_OOM_LIMIT_;
    if (oomlimit >= 0 && size >= oomlimit) {
      m_pBuffer = FX_TryAlloc(uint8_t, size);
      if (!m_pBuffer) {
        return false;
      }
    } else {
      m_pBuffer = FX_Alloc(uint8_t, size);
    }
  }
  m_Width = width;
  m_Height = height;
  m_Pitch = pitch;
  if (HasAlpha() && format != FXDIB_Argb) {
    bool ret = true;
    ret = BuildAlphaMask();
    if (!ret) {
      if (!m_bExtBuf) {
        FX_Free(m_pBuffer);
        m_pBuffer = nullptr;
        m_Width = m_Height = m_Pitch = 0;
        return false;
      }
    }
  }
  return true;
}

bool CFX_DIBitmap::Copy(const CFX_RetainPtr<CFX_DIBSource>& pSrc) {
  if (m_pBuffer)
    return false;

  if (!Create(pSrc->GetWidth(), pSrc->GetHeight(), pSrc->GetFormat()))
    return false;

  SetPalette(pSrc->GetPalette());
  SetAlphaMask(pSrc->m_pAlphaMask);
  for (int row = 0; row < pSrc->GetHeight(); row++)
    FXSYS_memcpy(m_pBuffer + row * m_Pitch, pSrc->GetScanline(row), m_Pitch);

  return true;
}

CFX_DIBitmap::~CFX_DIBitmap() {
  if (!m_bExtBuf)
    FX_Free(m_pBuffer);

  m_pBuffer = nullptr;
}

uint8_t* CFX_DIBitmap::GetBuffer() const {
  return m_pBuffer;
}

const uint8_t* CFX_DIBitmap::GetScanline(int line) const {
  return m_pBuffer ? m_pBuffer + line * m_Pitch : nullptr;
}

void CFX_DIBitmap::TakeOver(CFX_RetainPtr<CFX_DIBitmap>&& pSrcBitmap) {
  if (!m_bExtBuf)
    FX_Free(m_pBuffer);

  m_pBuffer = pSrcBitmap->m_pBuffer;
  m_pPalette = std::move(pSrcBitmap->m_pPalette);
  m_pAlphaMask = pSrcBitmap->m_pAlphaMask;
  pSrcBitmap->m_pBuffer = nullptr;
  pSrcBitmap->m_pAlphaMask = nullptr;
  m_bpp = pSrcBitmap->m_bpp;
  m_bExtBuf = pSrcBitmap->m_bExtBuf;
  m_AlphaFlag = pSrcBitmap->m_AlphaFlag;
  m_Width = pSrcBitmap->m_Width;
  m_Height = pSrcBitmap->m_Height;
  m_Pitch = pSrcBitmap->m_Pitch;
}

void CFX_DIBitmap::Clear(uint32_t color) {
  if (!m_pBuffer) {
    return;
  }
  switch (GetFormat()) {
    case FXDIB_1bppMask:
      FXSYS_memset(m_pBuffer, (color & 0xff000000) ? 0xff : 0,
                   m_Pitch * m_Height);
      break;
    case FXDIB_1bppRgb: {
      int index = FindPalette(color);
      FXSYS_memset(m_pBuffer, index ? 0xff : 0, m_Pitch * m_Height);
      break;
    }
    case FXDIB_8bppMask:
      FXSYS_memset(m_pBuffer, color >> 24, m_Pitch * m_Height);
      break;
    case FXDIB_8bppRgb: {
      int index = FindPalette(color);
      FXSYS_memset(m_pBuffer, index, m_Pitch * m_Height);
      break;
    }
    case FXDIB_Rgb:
    case FXDIB_Rgba: {
      int a, r, g, b;
      ArgbDecode(color, a, r, g, b);
      if (r == g && g == b) {
        FXSYS_memset(m_pBuffer, r, m_Pitch * m_Height);
      } else {
        int byte_pos = 0;
        for (int col = 0; col < m_Width; col++) {
          m_pBuffer[byte_pos++] = b;
          m_pBuffer[byte_pos++] = g;
          m_pBuffer[byte_pos++] = r;
        }
        for (int row = 1; row < m_Height; row++) {
          FXSYS_memcpy(m_pBuffer + row * m_Pitch, m_pBuffer, m_Pitch);
        }
      }
      break;
    }
    case FXDIB_Rgb32:
    case FXDIB_Argb: {
      color = IsCmykImage() ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
#ifdef _SKIA_SUPPORT_
      if (FXDIB_Rgb32 == GetFormat() && !IsCmykImage()) {
        color |= 0xFF000000;
      }
#endif
      for (int i = 0; i < m_Width; i++) {
        ((uint32_t*)m_pBuffer)[i] = color;
      }
      for (int row = 1; row < m_Height; row++) {
        FXSYS_memcpy(m_pBuffer + row * m_Pitch, m_pBuffer, m_Pitch);
      }
      break;
    }
    default:
      break;
  }
}

bool CFX_DIBitmap::TransferBitmap(
    int dest_left,
    int dest_top,
    int width,
    int height,
    const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap,
    int src_left,
    int src_top) {
  if (!m_pBuffer)
    return false;

  GetOverlapRect(dest_left, dest_top, width, height, pSrcBitmap->GetWidth(),
                 pSrcBitmap->GetHeight(), src_left, src_top, nullptr);
  if (width == 0 || height == 0)
    return true;

  FXDIB_Format dest_format = GetFormat();
  FXDIB_Format src_format = pSrcBitmap->GetFormat();
  if (dest_format == src_format) {
    if (GetBPP() == 1) {
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan = m_pBuffer + (dest_top + row) * m_Pitch;
        const uint8_t* src_scan = pSrcBitmap->GetScanline(src_top + row);
        for (int col = 0; col < width; col++) {
          if (src_scan[(src_left + col) / 8] &
              (1 << (7 - (src_left + col) % 8))) {
            dest_scan[(dest_left + col) / 8] |= 1
                                                << (7 - (dest_left + col) % 8);
          } else {
            dest_scan[(dest_left + col) / 8] &=
                ~(1 << (7 - (dest_left + col) % 8));
          }
        }
      }
    } else {
      int Bpp = GetBPP() / 8;
      for (int row = 0; row < height; row++) {
        uint8_t* dest_scan =
            m_pBuffer + (dest_top + row) * m_Pitch + dest_left * Bpp;
        const uint8_t* src_scan =
            pSrcBitmap->GetScanline(src_top + row) + src_left * Bpp;
        FXSYS_memcpy(dest_scan, src_scan, width * Bpp);
      }
    }
  } else {
    if (m_pPalette)
      return false;

    if (m_bpp == 8)
      dest_format = FXDIB_8bppMask;

    uint8_t* dest_buf =
        m_pBuffer + dest_top * m_Pitch + dest_left * GetBPP() / 8;
    std::unique_ptr<uint32_t, FxFreeDeleter> d_plt;
    if (!ConvertBuffer(dest_format, dest_buf, m_Pitch, width, height,
                       pSrcBitmap, src_left, src_top, &d_plt)) {
      return false;
    }
  }
  return true;
}

bool CFX_DIBitmap::TransferMask(int dest_left,
                                int dest_top,
                                int width,
                                int height,
                                const CFX_RetainPtr<CFX_DIBSource>& pMask,
                                uint32_t color,
                                int src_left,
                                int src_top,
                                int alpha_flag,
                                void* pIccTransform) {
  if (!m_pBuffer) {
    return false;
  }
  ASSERT(HasAlpha() && (m_bpp >= 24));
  ASSERT(pMask->IsAlphaMask());
  if (!HasAlpha() || !pMask->IsAlphaMask() || m_bpp < 24) {
    return false;
  }
  GetOverlapRect(dest_left, dest_top, width, height, pMask->GetWidth(),
                 pMask->GetHeight(), src_left, src_top, nullptr);
  if (width == 0 || height == 0) {
    return true;
  }
  int src_bpp = pMask->GetBPP();
  int alpha;
  uint32_t dst_color;
  if (alpha_flag >> 8) {
    alpha = alpha_flag & 0xff;
    dst_color = FXCMYK_TODIB(color);
  } else {
    alpha = FXARGB_A(color);
    dst_color = FXARGB_TODIB(color);
  }
  uint8_t* color_p = (uint8_t*)&dst_color;
  if (pIccTransform && CFX_GEModule::Get()->GetCodecModule() &&
      CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
    CCodec_IccModule* pIccModule =
        CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
    pIccModule->TranslateScanline(pIccTransform, color_p, color_p, 1);
  } else {
    if (alpha_flag >> 8 && !IsCmykImage()) {
      AdobeCMYK_to_sRGB1(FXSYS_GetCValue(color), FXSYS_GetMValue(color),
                         FXSYS_GetYValue(color), FXSYS_GetKValue(color),
                         color_p[2], color_p[1], color_p[0]);
    } else if (!(alpha_flag >> 8) && IsCmykImage()) {
      return false;
    }
  }
  if (!IsCmykImage()) {
    color_p[3] = (uint8_t)alpha;
  }
  if (GetFormat() == FXDIB_Argb) {
    for (int row = 0; row < height; row++) {
      uint32_t* dest_pos =
          (uint32_t*)(m_pBuffer + (dest_top + row) * m_Pitch + dest_left * 4);
      const uint8_t* src_scan = pMask->GetScanline(src_top + row);
      if (src_bpp == 1) {
        for (int col = 0; col < width; col++) {
          int src_bitpos = src_left + col;
          if (src_scan[src_bitpos / 8] & (1 << (7 - src_bitpos % 8))) {
            *dest_pos = dst_color;
          } else {
            *dest_pos = 0;
          }
          dest_pos++;
        }
      } else {
        src_scan += src_left;
        dst_color = FXARGB_TODIB(dst_color);
        dst_color &= 0xffffff;
        for (int col = 0; col < width; col++) {
          FXARGB_SETDIB(dest_pos++,
                        dst_color | ((alpha * (*src_scan++) / 255) << 24));
        }
      }
    }
  } else {
    int comps = m_bpp / 8;
    for (int row = 0; row < height; row++) {
      uint8_t* dest_color_pos =
          m_pBuffer + (dest_top + row) * m_Pitch + dest_left * comps;
      uint8_t* dest_alpha_pos =
          (uint8_t*)m_pAlphaMask->GetScanline(dest_top + row) + dest_left;
      const uint8_t* src_scan = pMask->GetScanline(src_top + row);
      if (src_bpp == 1) {
        for (int col = 0; col < width; col++) {
          int src_bitpos = src_left + col;
          if (src_scan[src_bitpos / 8] & (1 << (7 - src_bitpos % 8))) {
            FXSYS_memcpy(dest_color_pos, color_p, comps);
            *dest_alpha_pos = 0xff;
          } else {
            FXSYS_memset(dest_color_pos, 0, comps);
            *dest_alpha_pos = 0;
          }
          dest_color_pos += comps;
          dest_alpha_pos++;
        }
      } else {
        src_scan += src_left;
        for (int col = 0; col < width; col++) {
          FXSYS_memcpy(dest_color_pos, color_p, comps);
          dest_color_pos += comps;
          *dest_alpha_pos++ = (alpha * (*src_scan++) / 255);
        }
      }
    }
  }
  return true;
}

const int g_ChannelOffset[] = {0, 2, 1, 0, 0, 1, 2, 3, 3};
bool CFX_DIBitmap::LoadChannel(FXDIB_Channel destChannel,
                               const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap,
                               FXDIB_Channel srcChannel) {
  if (!m_pBuffer)
    return false;

  CFX_RetainPtr<CFX_DIBSource> pSrcClone = pSrcBitmap;
  int srcOffset;
  if (srcChannel == FXDIB_Alpha) {
    if (!pSrcBitmap->HasAlpha() && !pSrcBitmap->IsAlphaMask())
      return false;

    if (pSrcBitmap->GetBPP() == 1) {
      pSrcClone = pSrcBitmap->CloneConvert(FXDIB_8bppMask);
      if (!pSrcClone)
        return false;
    }
    srcOffset = pSrcBitmap->GetFormat() == FXDIB_Argb ? 3 : 0;
  } else {
    if (pSrcBitmap->IsAlphaMask())
      return false;

    if (pSrcBitmap->GetBPP() < 24) {
      if (pSrcBitmap->IsCmykImage()) {
        pSrcClone = pSrcBitmap->CloneConvert(static_cast<FXDIB_Format>(
            (pSrcBitmap->GetFormat() & 0xff00) | 0x20));
      } else {
        pSrcClone = pSrcBitmap->CloneConvert(static_cast<FXDIB_Format>(
            (pSrcBitmap->GetFormat() & 0xff00) | 0x18));
      }
      if (!pSrcClone)
        return false;
    }
    srcOffset = g_ChannelOffset[srcChannel];
  }
  int destOffset = 0;
  if (destChannel == FXDIB_Alpha) {
    if (IsAlphaMask()) {
      if (!ConvertFormat(FXDIB_8bppMask))
        return false;
    } else {
      if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb))
        return false;

      if (GetFormat() == FXDIB_Argb)
        destOffset = 3;
    }
  } else {
    if (IsAlphaMask())
      return false;

    if (GetBPP() < 24) {
      if (HasAlpha()) {
        if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb))
          return false;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      } else if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb32)) {
#else
      } else if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb)) {
#endif
        return false;
      }
    }
    destOffset = g_ChannelOffset[destChannel];
  }
  if (srcChannel == FXDIB_Alpha && pSrcClone->m_pAlphaMask) {
    CFX_RetainPtr<CFX_DIBSource> pAlphaMask = pSrcClone->m_pAlphaMask;
    if (pSrcClone->GetWidth() != m_Width ||
        pSrcClone->GetHeight() != m_Height) {
      if (pAlphaMask) {
        pAlphaMask = pAlphaMask->StretchTo(m_Width, m_Height);
        if (!pAlphaMask)
          return false;
      }
    }
    pSrcClone = std::move(pAlphaMask);
    srcOffset = 0;
  } else if (pSrcClone->GetWidth() != m_Width ||
             pSrcClone->GetHeight() != m_Height) {
    CFX_RetainPtr<CFX_DIBitmap> pSrcMatched =
        pSrcClone->StretchTo(m_Width, m_Height);
    if (!pSrcMatched)
      return false;

    pSrcClone = std::move(pSrcMatched);
  }
  CFX_RetainPtr<CFX_DIBitmap> pDst(this);
  if (destChannel == FXDIB_Alpha && m_pAlphaMask) {
    pDst = m_pAlphaMask;
    destOffset = 0;
  }
  int srcBytes = pSrcClone->GetBPP() / 8;
  int destBytes = pDst->GetBPP() / 8;
  for (int row = 0; row < m_Height; row++) {
    uint8_t* dest_pos = (uint8_t*)pDst->GetScanline(row) + destOffset;
    const uint8_t* src_pos = pSrcClone->GetScanline(row) + srcOffset;
    for (int col = 0; col < m_Width; col++) {
      *dest_pos = *src_pos;
      dest_pos += destBytes;
      src_pos += srcBytes;
    }
  }
  return true;
}

bool CFX_DIBitmap::LoadChannel(FXDIB_Channel destChannel, int value) {
  if (!m_pBuffer) {
    return false;
  }
  int destOffset;
  if (destChannel == FXDIB_Alpha) {
    if (IsAlphaMask()) {
      if (!ConvertFormat(FXDIB_8bppMask)) {
        return false;
      }
      destOffset = 0;
    } else {
      destOffset = 0;
      if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
        return false;
      }
      if (GetFormat() == FXDIB_Argb) {
        destOffset = 3;
      }
    }
  } else {
    if (IsAlphaMask()) {
      return false;
    }
    if (GetBPP() < 24) {
      if (HasAlpha()) {
        if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyka : FXDIB_Argb)) {
          return false;
        }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
      } else if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb)) {
#else
      } else if (!ConvertFormat(IsCmykImage() ? FXDIB_Cmyk : FXDIB_Rgb32)) {
#endif
        return false;
      }
    }
    destOffset = g_ChannelOffset[destChannel];
  }
  int Bpp = GetBPP() / 8;
  if (Bpp == 1) {
    FXSYS_memset(m_pBuffer, value, m_Height * m_Pitch);
    return true;
  }
  if (destChannel == FXDIB_Alpha && m_pAlphaMask) {
    FXSYS_memset(m_pAlphaMask->GetBuffer(), value,
                 m_pAlphaMask->GetHeight() * m_pAlphaMask->GetPitch());
    return true;
  }
  for (int row = 0; row < m_Height; row++) {
    uint8_t* scan_line = m_pBuffer + row * m_Pitch + destOffset;
    for (int col = 0; col < m_Width; col++) {
      *scan_line = value;
      scan_line += Bpp;
    }
  }
  return true;
}

bool CFX_DIBitmap::MultiplyAlpha(
    const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap) {
  if (!m_pBuffer)
    return false;

  ASSERT(pSrcBitmap->IsAlphaMask());
  if (!pSrcBitmap->IsAlphaMask())
    return false;

  if (!IsAlphaMask() && !HasAlpha())
    return LoadChannel(FXDIB_Alpha, pSrcBitmap, FXDIB_Alpha);

  CFX_RetainPtr<CFX_DIBitmap> pSrcClone = pSrcBitmap.As<CFX_DIBitmap>();
  if (pSrcBitmap->GetWidth() != m_Width ||
      pSrcBitmap->GetHeight() != m_Height) {
    pSrcClone = pSrcBitmap->StretchTo(m_Width, m_Height);
    if (!pSrcClone)
      return false;
  }
  if (IsAlphaMask()) {
    if (!ConvertFormat(FXDIB_8bppMask))
      return false;

    for (int row = 0; row < m_Height; row++) {
      uint8_t* dest_scan = m_pBuffer + m_Pitch * row;
      uint8_t* src_scan = pSrcClone->m_pBuffer + pSrcClone->m_Pitch * row;
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
    if (GetFormat() == FXDIB_Argb) {
      if (pSrcClone->GetBPP() == 1)
        return false;

      for (int row = 0; row < m_Height; row++) {
        uint8_t* dest_scan = m_pBuffer + m_Pitch * row + 3;
        uint8_t* src_scan = pSrcClone->m_pBuffer + pSrcClone->m_Pitch * row;
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

bool CFX_DIBitmap::GetGrayData(void* pIccTransform) {
  if (!m_pBuffer) {
    return false;
  }
  switch (GetFormat()) {
    case FXDIB_1bppRgb: {
      if (!m_pPalette)
        return false;

      uint8_t gray[2];
      for (int i = 0; i < 2; i++) {
        int r = static_cast<uint8_t>(m_pPalette.get()[i] >> 16);
        int g = static_cast<uint8_t>(m_pPalette.get()[i] >> 8);
        int b = static_cast<uint8_t>(m_pPalette.get()[i]);
        gray[i] = static_cast<uint8_t>(FXRGB2GRAY(r, g, b));
      }
      auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
      if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask))
        return false;

      FXSYS_memset(pMask->GetBuffer(), gray[0], pMask->GetPitch() * m_Height);
      for (int row = 0; row < m_Height; row++) {
        uint8_t* src_pos = m_pBuffer + row * m_Pitch;
        uint8_t* dest_pos = (uint8_t*)pMask->GetScanline(row);
        for (int col = 0; col < m_Width; col++) {
          if (src_pos[col / 8] & (1 << (7 - col % 8))) {
            *dest_pos = gray[1];
          }
          dest_pos++;
        }
      }
      TakeOver(std::move(pMask));
      break;
    }
    case FXDIB_8bppRgb: {
      if (!m_pPalette)
        return false;

      uint8_t gray[256];
      for (int i = 0; i < 256; i++) {
        int r = static_cast<uint8_t>(m_pPalette.get()[i] >> 16);
        int g = static_cast<uint8_t>(m_pPalette.get()[i] >> 8);
        int b = static_cast<uint8_t>(m_pPalette.get()[i]);
        gray[i] = static_cast<uint8_t>(FXRGB2GRAY(r, g, b));
      }
      auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
      if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask))
        return false;

      for (int row = 0; row < m_Height; row++) {
        uint8_t* dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
        uint8_t* src_pos = m_pBuffer + row * m_Pitch;
        for (int col = 0; col < m_Width; col++) {
          *dest_pos++ = gray[*src_pos++];
        }
      }
      TakeOver(std::move(pMask));
      break;
    }
    case FXDIB_Rgb: {
      auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
      if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask))
        return false;

      for (int row = 0; row < m_Height; row++) {
        uint8_t* src_pos = m_pBuffer + row * m_Pitch;
        uint8_t* dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
        for (int col = 0; col < m_Width; col++) {
          *dest_pos++ = FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos);
          src_pos += 3;
        }
      }
      TakeOver(std::move(pMask));
      break;
    }
    case FXDIB_Rgb32: {
      auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
      if (!pMask->Create(m_Width, m_Height, FXDIB_8bppMask))
        return false;

      for (int row = 0; row < m_Height; row++) {
        uint8_t* src_pos = m_pBuffer + row * m_Pitch;
        uint8_t* dest_pos = pMask->GetBuffer() + row * pMask->GetPitch();
        for (int col = 0; col < m_Width; col++) {
          *dest_pos++ = FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos);
          src_pos += 4;
        }
      }
      TakeOver(std::move(pMask));
      break;
    }
    default:
      return false;
  }
  return true;
}

bool CFX_DIBitmap::MultiplyAlpha(int alpha) {
  if (!m_pBuffer) {
    return false;
  }
  switch (GetFormat()) {
    case FXDIB_1bppMask:
      if (!ConvertFormat(FXDIB_8bppMask)) {
        return false;
      }
      MultiplyAlpha(alpha);
      break;
    case FXDIB_8bppMask: {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* scan_line = m_pBuffer + row * m_Pitch;
        for (int col = 0; col < m_Width; col++) {
          scan_line[col] = scan_line[col] * alpha / 255;
        }
      }
      break;
    }
    case FXDIB_Argb: {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* scan_line = m_pBuffer + row * m_Pitch + 3;
        for (int col = 0; col < m_Width; col++) {
          *scan_line = (*scan_line) * alpha / 255;
          scan_line += 4;
        }
      }
      break;
    }
    default:
      if (HasAlpha()) {
        m_pAlphaMask->MultiplyAlpha(alpha);
      } else if (IsCmykImage()) {
        if (!ConvertFormat((FXDIB_Format)(GetFormat() | 0x0200))) {
          return false;
        }
        m_pAlphaMask->MultiplyAlpha(alpha);
      } else {
        if (!ConvertFormat(FXDIB_Argb)) {
          return false;
        }
        MultiplyAlpha(alpha);
      }
      break;
  }
  return true;
}

uint32_t CFX_DIBitmap::GetPixel(int x, int y) const {
  if (!m_pBuffer) {
    return 0;
  }
  uint8_t* pos = m_pBuffer + y * m_Pitch + x * GetBPP() / 8;
  switch (GetFormat()) {
    case FXDIB_1bppMask: {
      if ((*pos) & (1 << (7 - x % 8))) {
        return 0xff000000;
      }
      return 0;
    }
    case FXDIB_1bppRgb: {
      if ((*pos) & (1 << (7 - x % 8))) {
        return m_pPalette ? m_pPalette.get()[1] : 0xffffffff;
      }
      return m_pPalette ? m_pPalette.get()[0] : 0xff000000;
    }
    case FXDIB_8bppMask:
      return (*pos) << 24;
    case FXDIB_8bppRgb:
      return m_pPalette ? m_pPalette.get()[*pos]
                        : (0xff000000 | ((*pos) * 0x10101));
    case FXDIB_Rgb:
    case FXDIB_Rgba:
    case FXDIB_Rgb32:
      return FXARGB_GETDIB(pos) | 0xff000000;
    case FXDIB_Argb:
      return FXARGB_GETDIB(pos);
    default:
      break;
  }
  return 0;
}

void CFX_DIBitmap::SetPixel(int x, int y, uint32_t color) {
  if (!m_pBuffer) {
    return;
  }
  if (x < 0 || x >= m_Width || y < 0 || y >= m_Height) {
    return;
  }
  uint8_t* pos = m_pBuffer + y * m_Pitch + x * GetBPP() / 8;
  switch (GetFormat()) {
    case FXDIB_1bppMask:
      if (color >> 24) {
        *pos |= 1 << (7 - x % 8);
      } else {
        *pos &= ~(1 << (7 - x % 8));
      }
      break;
    case FXDIB_1bppRgb:
      if (m_pPalette) {
        if (color == m_pPalette.get()[1]) {
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
    case FXDIB_8bppMask:
      *pos = (uint8_t)(color >> 24);
      break;
    case FXDIB_8bppRgb: {
      if (m_pPalette) {
        for (int i = 0; i < 256; i++) {
          if (m_pPalette.get()[i] == color) {
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
    case FXDIB_Rgb:
    case FXDIB_Rgb32: {
      int alpha = FXARGB_A(color);
      pos[0] = (FXARGB_B(color) * alpha + pos[0] * (255 - alpha)) / 255;
      pos[1] = (FXARGB_G(color) * alpha + pos[1] * (255 - alpha)) / 255;
      pos[2] = (FXARGB_R(color) * alpha + pos[2] * (255 - alpha)) / 255;
      break;
    }
    case FXDIB_Rgba: {
      pos[0] = FXARGB_B(color);
      pos[1] = FXARGB_G(color);
      pos[2] = FXARGB_R(color);
      break;
    }
    case FXDIB_Argb:
      FXARGB_SETDIB(pos, color);
      break;
    default:
      break;
  }
}

void CFX_DIBitmap::DownSampleScanline(int line,
                                      uint8_t* dest_scan,
                                      int dest_bpp,
                                      int dest_width,
                                      bool bFlipX,
                                      int clip_left,
                                      int clip_width) const {
  if (!m_pBuffer) {
    return;
  }
  int src_Bpp = m_bpp / 8;
  uint8_t* scanline = m_pBuffer + line * m_Pitch;
  if (src_Bpp == 0) {
    for (int i = 0; i < clip_width; i++) {
      uint32_t dest_x = clip_left + i;
      uint32_t src_x = dest_x * m_Width / dest_width;
      if (bFlipX) {
        src_x = m_Width - src_x - 1;
      }
      src_x %= m_Width;
      dest_scan[i] = (scanline[src_x / 8] & (1 << (7 - src_x % 8))) ? 255 : 0;
    }
  } else if (src_Bpp == 1) {
    for (int i = 0; i < clip_width; i++) {
      uint32_t dest_x = clip_left + i;
      uint32_t src_x = dest_x * m_Width / dest_width;
      if (bFlipX) {
        src_x = m_Width - src_x - 1;
      }
      src_x %= m_Width;
      int dest_pos = i;
      if (m_pPalette) {
        if (!IsCmykImage()) {
          dest_pos *= 3;
          FX_ARGB argb = m_pPalette.get()[scanline[src_x]];
          dest_scan[dest_pos] = FXARGB_B(argb);
          dest_scan[dest_pos + 1] = FXARGB_G(argb);
          dest_scan[dest_pos + 2] = FXARGB_R(argb);
        } else {
          dest_pos *= 4;
          FX_CMYK cmyk = m_pPalette.get()[scanline[src_x]];
          dest_scan[dest_pos] = FXSYS_GetCValue(cmyk);
          dest_scan[dest_pos + 1] = FXSYS_GetMValue(cmyk);
          dest_scan[dest_pos + 2] = FXSYS_GetYValue(cmyk);
          dest_scan[dest_pos + 3] = FXSYS_GetKValue(cmyk);
        }
      } else {
        dest_scan[dest_pos] = scanline[src_x];
      }
    }
  } else {
    for (int i = 0; i < clip_width; i++) {
      uint32_t dest_x = clip_left + i;
      uint32_t src_x =
          bFlipX ? (m_Width - dest_x * m_Width / dest_width - 1) * src_Bpp
                 : (dest_x * m_Width / dest_width) * src_Bpp;
      src_x %= m_Width * src_Bpp;
      int dest_pos = i * src_Bpp;
      for (int b = 0; b < src_Bpp; b++) {
        dest_scan[dest_pos + b] = scanline[src_x + b];
      }
    }
  }
}

// TODO(weili): Split this function into two for handling CMYK and RGB
// colors separately.
bool CFX_DIBitmap::ConvertColorScale(uint32_t forecolor, uint32_t backcolor) {
  ASSERT(!IsAlphaMask());
  if (!m_pBuffer || IsAlphaMask()) {
    return false;
  }
  // Values used for CMYK colors.
  int fc = 0;
  int fm = 0;
  int fy = 0;
  int fk = 0;
  int bc = 0;
  int bm = 0;
  int by = 0;
  int bk = 0;
  // Values used for RGB colors.
  int fr = 0;
  int fg = 0;
  int fb = 0;
  int br = 0;
  int bg = 0;
  int bb = 0;
  bool isCmykImage = IsCmykImage();
  if (isCmykImage) {
    fc = FXSYS_GetCValue(forecolor);
    fm = FXSYS_GetMValue(forecolor);
    fy = FXSYS_GetYValue(forecolor);
    fk = FXSYS_GetKValue(forecolor);
    bc = FXSYS_GetCValue(backcolor);
    bm = FXSYS_GetMValue(backcolor);
    by = FXSYS_GetYValue(backcolor);
    bk = FXSYS_GetKValue(backcolor);
  } else {
    fr = FXSYS_GetRValue(forecolor);
    fg = FXSYS_GetGValue(forecolor);
    fb = FXSYS_GetBValue(forecolor);
    br = FXSYS_GetRValue(backcolor);
    bg = FXSYS_GetGValue(backcolor);
    bb = FXSYS_GetBValue(backcolor);
  }
  if (m_bpp <= 8) {
    if (isCmykImage) {
      if (forecolor == 0xff && backcolor == 0 && !m_pPalette) {
        return true;
      }
    } else if (forecolor == 0 && backcolor == 0xffffff && !m_pPalette) {
      return true;
    }
    if (!m_pPalette) {
      BuildPalette();
    }
    int size = 1 << m_bpp;
    if (isCmykImage) {
      for (int i = 0; i < size; i++) {
        uint8_t b, g, r;
        AdobeCMYK_to_sRGB1(FXSYS_GetCValue(m_pPalette.get()[i]),
                           FXSYS_GetMValue(m_pPalette.get()[i]),
                           FXSYS_GetYValue(m_pPalette.get()[i]),
                           FXSYS_GetKValue(m_pPalette.get()[i]), r, g, b);
        int gray = 255 - FXRGB2GRAY(r, g, b);
        m_pPalette.get()[i] = CmykEncode(
            bc + (fc - bc) * gray / 255, bm + (fm - bm) * gray / 255,
            by + (fy - by) * gray / 255, bk + (fk - bk) * gray / 255);
      }
    } else {
      for (int i = 0; i < size; i++) {
        int gray = FXRGB2GRAY(FXARGB_R(m_pPalette.get()[i]),
                              FXARGB_G(m_pPalette.get()[i]),
                              FXARGB_B(m_pPalette.get()[i]));
        m_pPalette.get()[i] = FXARGB_MAKE(0xff, br + (fr - br) * gray / 255,
                                          bg + (fg - bg) * gray / 255,
                                          bb + (fb - bb) * gray / 255);
      }
    }
    return true;
  }
  if (isCmykImage) {
    if (forecolor == 0xff && backcolor == 0x00) {
      for (int row = 0; row < m_Height; row++) {
        uint8_t* scanline = m_pBuffer + row * m_Pitch;
        for (int col = 0; col < m_Width; col++) {
          uint8_t b, g, r;
          AdobeCMYK_to_sRGB1(scanline[0], scanline[1], scanline[2], scanline[3],
                             r, g, b);
          *scanline++ = 0;
          *scanline++ = 0;
          *scanline++ = 0;
          *scanline++ = 255 - FXRGB2GRAY(r, g, b);
        }
      }
      return true;
    }
  } else if (forecolor == 0 && backcolor == 0xffffff) {
    for (int row = 0; row < m_Height; row++) {
      uint8_t* scanline = m_pBuffer + row * m_Pitch;
      int gap = m_bpp / 8 - 2;
      for (int col = 0; col < m_Width; col++) {
        int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
        *scanline++ = gray;
        *scanline++ = gray;
        *scanline = gray;
        scanline += gap;
      }
    }
    return true;
  }
  if (isCmykImage) {
    for (int row = 0; row < m_Height; row++) {
      uint8_t* scanline = m_pBuffer + row * m_Pitch;
      for (int col = 0; col < m_Width; col++) {
        uint8_t b, g, r;
        AdobeCMYK_to_sRGB1(scanline[0], scanline[1], scanline[2], scanline[3],
                           r, g, b);
        int gray = 255 - FXRGB2GRAY(r, g, b);
        *scanline++ = bc + (fc - bc) * gray / 255;
        *scanline++ = bm + (fm - bm) * gray / 255;
        *scanline++ = by + (fy - by) * gray / 255;
        *scanline++ = bk + (fk - bk) * gray / 255;
      }
    }
  } else {
    for (int row = 0; row < m_Height; row++) {
      uint8_t* scanline = m_pBuffer + row * m_Pitch;
      int gap = m_bpp / 8 - 2;
      for (int col = 0; col < m_Width; col++) {
        int gray = FXRGB2GRAY(scanline[2], scanline[1], scanline[0]);
        *scanline++ = bb + (fb - bb) * gray / 255;
        *scanline++ = bg + (fg - bg) * gray / 255;
        *scanline = br + (fr - br) * gray / 255;
        scanline += gap;
      }
    }
  }
  return true;
}

CFX_DIBExtractor::CFX_DIBExtractor(const CFX_RetainPtr<CFX_DIBSource>& pSrc) {
  if (pSrc->GetBuffer()) {
    CFX_RetainPtr<CFX_DIBSource> pOldSrc(pSrc);
    m_pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!m_pBitmap->Create(pOldSrc->GetWidth(), pOldSrc->GetHeight(),
                           pOldSrc->GetFormat(), pOldSrc->GetBuffer())) {
      m_pBitmap.Reset();
      return;
    }
    m_pBitmap->SetPalette(pOldSrc->GetPalette());
    m_pBitmap->SetAlphaMask(pOldSrc->m_pAlphaMask);
  } else {
    m_pBitmap = pSrc->Clone();
  }
}

CFX_DIBExtractor::~CFX_DIBExtractor() {}

CFX_BitmapStorer::CFX_BitmapStorer() {
}

CFX_BitmapStorer::~CFX_BitmapStorer() {
}

CFX_RetainPtr<CFX_DIBitmap> CFX_BitmapStorer::Detach() {
  return std::move(m_pBitmap);
}

void CFX_BitmapStorer::Replace(CFX_RetainPtr<CFX_DIBitmap>&& pBitmap) {
  m_pBitmap = std::move(pBitmap);
}

void CFX_BitmapStorer::ComposeScanline(int line,
                                       const uint8_t* scanline,
                                       const uint8_t* scan_extra_alpha) {
  uint8_t* dest_buf = const_cast<uint8_t*>(m_pBitmap->GetScanline(line));
  uint8_t* dest_alpha_buf =
      m_pBitmap->m_pAlphaMask
          ? const_cast<uint8_t*>(m_pBitmap->m_pAlphaMask->GetScanline(line))
          : nullptr;
  if (dest_buf)
    FXSYS_memcpy(dest_buf, scanline, m_pBitmap->GetPitch());

  if (dest_alpha_buf) {
    FXSYS_memcpy(dest_alpha_buf, scan_extra_alpha,
                 m_pBitmap->m_pAlphaMask->GetPitch());
  }
}

bool CFX_BitmapStorer::SetInfo(int width,
                               int height,
                               FXDIB_Format src_format,
                               uint32_t* pSrcPalette) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height, src_format))
    return false;

  if (pSrcPalette)
    pBitmap->SetPalette(pSrcPalette);

  m_pBitmap = std::move(pBitmap);
  return true;
}
