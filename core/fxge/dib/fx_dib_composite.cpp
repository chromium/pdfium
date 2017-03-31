// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "core/fxcodec/fx_codec.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/ge/cfx_cliprgn.h"

bool CFX_DIBitmap::CompositeBitmap(
    int dest_left,
    int dest_top,
    int width,
    int height,
    const CFX_RetainPtr<CFX_DIBSource>& pSrcBitmap,
    int src_left,
    int src_top,
    int blend_type,
    const CFX_ClipRgn* pClipRgn,
    bool bRgbByteOrder,
    void* pIccTransform) {
  if (!m_pBuffer) {
    return false;
  }
  ASSERT(!pSrcBitmap->IsAlphaMask());
  ASSERT(m_bpp >= 8);
  if (pSrcBitmap->IsAlphaMask() || m_bpp < 8) {
    return false;
  }
  GetOverlapRect(dest_left, dest_top, width, height, pSrcBitmap->GetWidth(),
                 pSrcBitmap->GetHeight(), src_left, src_top, pClipRgn);
  if (width == 0 || height == 0) {
    return true;
  }
  CFX_RetainPtr<CFX_DIBitmap> pClipMask;
  FX_RECT clip_box;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::RectI) {
    ASSERT(pClipRgn->GetType() == CFX_ClipRgn::MaskF);
    pClipMask = pClipRgn->GetMask();
    clip_box = pClipRgn->GetBox();
  }
  CFX_ScanlineCompositor compositor;
  if (!compositor.Init(GetFormat(), pSrcBitmap->GetFormat(), width,
                       pSrcBitmap->GetPalette(), 0, blend_type,
                       pClipMask != nullptr, bRgbByteOrder, 0, pIccTransform)) {
    return false;
  }
  int dest_Bpp = m_bpp / 8;
  int src_Bpp = pSrcBitmap->GetBPP() / 8;
  bool bRgb = src_Bpp > 1 && !pSrcBitmap->IsCmykImage();
  CFX_RetainPtr<CFX_DIBitmap> pSrcAlphaMask = pSrcBitmap->m_pAlphaMask;
  for (int row = 0; row < height; row++) {
    uint8_t* dest_scan =
        m_pBuffer + (dest_top + row) * m_Pitch + dest_left * dest_Bpp;
    const uint8_t* src_scan =
        pSrcBitmap->GetScanline(src_top + row) + src_left * src_Bpp;
    const uint8_t* src_scan_extra_alpha =
        pSrcAlphaMask ? pSrcAlphaMask->GetScanline(src_top + row) + src_left
                      : nullptr;
    uint8_t* dst_scan_extra_alpha =
        m_pAlphaMask
            ? (uint8_t*)m_pAlphaMask->GetScanline(dest_top + row) + dest_left
            : nullptr;
    const uint8_t* clip_scan = nullptr;
    if (pClipMask) {
      clip_scan = pClipMask->m_pBuffer +
                  (dest_top + row - clip_box.top) * pClipMask->m_Pitch +
                  (dest_left - clip_box.left);
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
                                 const CFX_RetainPtr<CFX_DIBSource>& pMask,
                                 uint32_t color,
                                 int src_left,
                                 int src_top,
                                 int blend_type,
                                 const CFX_ClipRgn* pClipRgn,
                                 bool bRgbByteOrder,
                                 int alpha_flag,
                                 void* pIccTransform) {
  if (!m_pBuffer) {
    return false;
  }
  ASSERT(pMask->IsAlphaMask());
  ASSERT(m_bpp >= 8);
  if (!pMask->IsAlphaMask() || m_bpp < 8) {
    return false;
  }
  GetOverlapRect(dest_left, dest_top, width, height, pMask->GetWidth(),
                 pMask->GetHeight(), src_left, src_top, pClipRgn);
  if (width == 0 || height == 0) {
    return true;
  }
  int src_alpha =
      (uint8_t)(alpha_flag >> 8) ? (alpha_flag & 0xff) : FXARGB_A(color);
  if (src_alpha == 0) {
    return true;
  }
  CFX_RetainPtr<CFX_DIBitmap> pClipMask;
  FX_RECT clip_box;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::RectI) {
    ASSERT(pClipRgn->GetType() == CFX_ClipRgn::MaskF);
    pClipMask = pClipRgn->GetMask();
    clip_box = pClipRgn->GetBox();
  }
  int src_bpp = pMask->GetBPP();
  int Bpp = GetBPP() / 8;
  CFX_ScanlineCompositor compositor;
  if (!compositor.Init(GetFormat(), pMask->GetFormat(), width, nullptr, color,
                       blend_type, pClipMask != nullptr, bRgbByteOrder,
                       alpha_flag, pIccTransform)) {
    return false;
  }
  for (int row = 0; row < height; row++) {
    uint8_t* dest_scan =
        m_pBuffer + (dest_top + row) * m_Pitch + dest_left * Bpp;
    const uint8_t* src_scan = pMask->GetScanline(src_top + row);
    uint8_t* dst_scan_extra_alpha =
        m_pAlphaMask
            ? (uint8_t*)m_pAlphaMask->GetScanline(dest_top + row) + dest_left
            : nullptr;
    const uint8_t* clip_scan = nullptr;
    if (pClipMask) {
      clip_scan = pClipMask->m_pBuffer +
                  (dest_top + row - clip_box.top) * pClipMask->m_Pitch +
                  (dest_left - clip_box.left);
    }
    if (src_bpp == 1) {
      compositor.CompositeBitMaskLine(dest_scan, src_scan, src_left, width,
                                      clip_scan, dst_scan_extra_alpha);
    } else {
      compositor.CompositeByteMaskLine(dest_scan, src_scan + src_left, width,
                                       clip_scan, dst_scan_extra_alpha);
    }
  }
  return true;
}

bool CFX_DIBitmap::CompositeRect(int left,
                                 int top,
                                 int width,
                                 int height,
                                 uint32_t color,
                                 int alpha_flag,
                                 void* pIccTransform) {
  if (!m_pBuffer) {
    return false;
  }
  int src_alpha = (alpha_flag >> 8) ? (alpha_flag & 0xff) : FXARGB_A(color);
  if (src_alpha == 0) {
    return true;
  }
  FX_RECT rect(left, top, left + width, top + height);
  rect.Intersect(0, 0, m_Width, m_Height);
  if (rect.IsEmpty()) {
    return true;
  }
  width = rect.Width();
  uint32_t dst_color;
  if (alpha_flag >> 8) {
    dst_color = FXCMYK_TODIB(color);
  } else {
    dst_color = FXARGB_TODIB(color);
  }
  uint8_t* color_p = (uint8_t*)&dst_color;
  if (m_bpp == 8) {
    uint8_t gray = 255;
    if (!IsAlphaMask()) {
      if (pIccTransform && CFX_GEModule::Get()->GetCodecModule() &&
          CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        CCodec_IccModule* pIccModule =
            CFX_GEModule::Get()->GetCodecModule()->GetIccModule();
        pIccModule->TranslateScanline(pIccTransform, &gray, color_p, 1);
      } else {
        if (alpha_flag >> 8) {
          uint8_t r, g, b;
          AdobeCMYK_to_sRGB1(color_p[0], color_p[1], color_p[2], color_p[3], r,
                             g, b);
          gray = FXRGB2GRAY(r, g, b);
        } else {
          gray = (uint8_t)FXRGB2GRAY((int)color_p[2], color_p[1], color_p[0]);
        }
      }
      if (IsCmykImage()) {
        gray = ~gray;
      }
    }
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan = m_pBuffer + row * m_Pitch + rect.left;
      if (src_alpha == 255) {
        FXSYS_memset(dest_scan, gray, width);
      } else {
        for (int col = 0; col < width; col++) {
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, src_alpha);
          dest_scan++;
        }
      }
    }
    return true;
  }
  if (m_bpp == 1) {
    ASSERT(!IsCmykImage() && (uint8_t)(alpha_flag >> 8) == 0);
    int left_shift = rect.left % 8;
    int right_shift = rect.right % 8;
    int new_width = rect.right / 8 - rect.left / 8;
    int index = 0;
    if (m_pPalette) {
      for (int i = 0; i < 2; i++) {
        if (m_pPalette.get()[i] == color) {
          index = i;
        }
      }
    } else {
      index = ((uint8_t)color == 0xff) ? 1 : 0;
    }
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan_top = (uint8_t*)GetScanline(row) + rect.left / 8;
      uint8_t* dest_scan_top_r = (uint8_t*)GetScanline(row) + rect.right / 8;
      uint8_t left_flag = *dest_scan_top & (255 << (8 - left_shift));
      uint8_t right_flag = *dest_scan_top_r & (255 >> right_shift);
      if (new_width) {
        FXSYS_memset(dest_scan_top + 1, index ? 255 : 0, new_width - 1);
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
  ASSERT(m_bpp >= 24);
  if (m_bpp < 24) {
    return false;
  }
  if (pIccTransform && CFX_GEModule::Get()->GetCodecModule()) {
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
    color_p[3] = (uint8_t)src_alpha;
  }
  int Bpp = m_bpp / 8;
  bool bAlpha = HasAlpha();
  bool bArgb = GetFormat() == FXDIB_Argb;
  if (src_alpha == 255) {
    for (int row = rect.top; row < rect.bottom; row++) {
      uint8_t* dest_scan = m_pBuffer + row * m_Pitch + rect.left * Bpp;
      uint8_t* dest_scan_alpha =
          m_pAlphaMask ? (uint8_t*)m_pAlphaMask->GetScanline(row) + rect.left
                       : nullptr;
      if (dest_scan_alpha) {
        FXSYS_memset(dest_scan_alpha, 0xff, width);
      }
      if (Bpp == 4) {
        uint32_t* scan = (uint32_t*)dest_scan;
        for (int col = 0; col < width; col++) {
          *scan++ = dst_color;
        }
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
    uint8_t* dest_scan = m_pBuffer + row * m_Pitch + rect.left * Bpp;
    if (bAlpha) {
      if (bArgb) {
        for (int col = 0; col < width; col++) {
          uint8_t back_alpha = dest_scan[3];
          if (back_alpha == 0) {
            FXARGB_SETDIB(dest_scan, FXARGB_MAKE(src_alpha, color_p[2],
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
            (uint8_t*)m_pAlphaMask->GetScanline(row) + rect.left;
        for (int col = 0; col < width; col++) {
          uint8_t back_alpha = *dest_scan_alpha;
          if (back_alpha == 0) {
            *dest_scan_alpha++ = src_alpha;
            FXSYS_memcpy(dest_scan, color_p, Bpp);
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

CFX_BitmapComposer::CFX_BitmapComposer() {
  m_pScanlineV = nullptr;
  m_pScanlineAlphaV = nullptr;
  m_pClipScanV = nullptr;
  m_pAddClipScan = nullptr;
  m_bRgbByteOrder = false;
  m_BlendType = FXDIB_BLEND_NORMAL;
}

CFX_BitmapComposer::~CFX_BitmapComposer() {
  FX_Free(m_pScanlineV);
  FX_Free(m_pScanlineAlphaV);
  FX_Free(m_pClipScanV);
  FX_Free(m_pAddClipScan);
}

void CFX_BitmapComposer::Compose(const CFX_RetainPtr<CFX_DIBitmap>& pDest,
                                 const CFX_ClipRgn* pClipRgn,
                                 int bitmap_alpha,
                                 uint32_t mask_color,
                                 FX_RECT& dest_rect,
                                 bool bVertical,
                                 bool bFlipX,
                                 bool bFlipY,
                                 bool bRgbByteOrder,
                                 int alpha_flag,
                                 void* pIccTransform,
                                 int blend_type) {
  m_pBitmap = pDest;
  m_pClipRgn = pClipRgn;
  m_DestLeft = dest_rect.left;
  m_DestTop = dest_rect.top;
  m_DestWidth = dest_rect.Width();
  m_DestHeight = dest_rect.Height();
  m_BitmapAlpha = bitmap_alpha;
  m_MaskColor = mask_color;
  m_pClipMask = nullptr;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::RectI)
    m_pClipMask = pClipRgn->GetMask();
  m_bVertical = bVertical;
  m_bFlipX = bFlipX;
  m_bFlipY = bFlipY;
  m_AlphaFlag = alpha_flag;
  m_pIccTransform = pIccTransform;
  m_bRgbByteOrder = bRgbByteOrder;
  m_BlendType = blend_type;
}
bool CFX_BitmapComposer::SetInfo(int width,
                                 int height,
                                 FXDIB_Format src_format,
                                 uint32_t* pSrcPalette) {
  m_SrcFormat = src_format;
  if (!m_Compositor.Init(m_pBitmap->GetFormat(), src_format, width, pSrcPalette,
                         m_MaskColor, FXDIB_BLEND_NORMAL,
                         m_pClipMask != nullptr || (m_BitmapAlpha < 255),
                         m_bRgbByteOrder, m_AlphaFlag, m_pIccTransform)) {
    return false;
  }
  if (m_bVertical) {
    m_pScanlineV = FX_Alloc(uint8_t, m_pBitmap->GetBPP() / 8 * width + 4);
    m_pClipScanV = FX_Alloc(uint8_t, m_pBitmap->GetHeight());
    if (m_pBitmap->m_pAlphaMask) {
      m_pScanlineAlphaV = FX_Alloc(uint8_t, width + 4);
    }
  }
  if (m_BitmapAlpha < 255) {
    m_pAddClipScan = FX_Alloc(
        uint8_t, m_bVertical ? m_pBitmap->GetHeight() : m_pBitmap->GetWidth());
  }
  return true;
}

void CFX_BitmapComposer::DoCompose(uint8_t* dest_scan,
                                   const uint8_t* src_scan,
                                   int dest_width,
                                   const uint8_t* clip_scan,
                                   const uint8_t* src_extra_alpha,
                                   uint8_t* dst_extra_alpha) {
  if (m_BitmapAlpha < 255) {
    if (clip_scan) {
      for (int i = 0; i < dest_width; i++) {
        m_pAddClipScan[i] = clip_scan[i] * m_BitmapAlpha / 255;
      }
    } else {
      FXSYS_memset(m_pAddClipScan, m_BitmapAlpha, dest_width);
    }
    clip_scan = m_pAddClipScan;
  }
  if (m_SrcFormat == FXDIB_8bppMask) {
    m_Compositor.CompositeByteMaskLine(dest_scan, src_scan, dest_width,
                                       clip_scan, dst_extra_alpha);
  } else if ((m_SrcFormat & 0xff) == 8) {
    m_Compositor.CompositePalBitmapLine(dest_scan, src_scan, 0, dest_width,
                                        clip_scan, src_extra_alpha,
                                        dst_extra_alpha);
  } else {
    m_Compositor.CompositeRgbBitmapLine(dest_scan, src_scan, dest_width,
                                        clip_scan, src_extra_alpha,
                                        dst_extra_alpha);
  }
}

void CFX_BitmapComposer::ComposeScanline(int line,
                                         const uint8_t* scanline,
                                         const uint8_t* scan_extra_alpha) {
  if (m_bVertical) {
    ComposeScanlineV(line, scanline, scan_extra_alpha);
    return;
  }
  const uint8_t* clip_scan = nullptr;
  if (m_pClipMask)
    clip_scan = m_pClipMask->GetBuffer() +
                (m_DestTop + line - m_pClipRgn->GetBox().top) *
                    m_pClipMask->GetPitch() +
                (m_DestLeft - m_pClipRgn->GetBox().left);
  uint8_t* dest_scan = (uint8_t*)m_pBitmap->GetScanline(line + m_DestTop) +
                       m_DestLeft * m_pBitmap->GetBPP() / 8;
  uint8_t* dest_alpha_scan =
      m_pBitmap->m_pAlphaMask
          ? (uint8_t*)m_pBitmap->m_pAlphaMask->GetScanline(line + m_DestTop) +
                m_DestLeft
          : nullptr;
  DoCompose(dest_scan, scanline, m_DestWidth, clip_scan, scan_extra_alpha,
            dest_alpha_scan);
}

void CFX_BitmapComposer::ComposeScanlineV(int line,
                                          const uint8_t* scanline,
                                          const uint8_t* scan_extra_alpha) {
  int i;
  int Bpp = m_pBitmap->GetBPP() / 8;
  int dest_pitch = m_pBitmap->GetPitch();
  int dest_alpha_pitch =
      m_pBitmap->m_pAlphaMask ? m_pBitmap->m_pAlphaMask->GetPitch() : 0;
  int dest_x = m_DestLeft + (m_bFlipX ? (m_DestWidth - line - 1) : line);
  uint8_t* dest_buf =
      m_pBitmap->GetBuffer() + dest_x * Bpp + m_DestTop * dest_pitch;
  uint8_t* dest_alpha_buf = m_pBitmap->m_pAlphaMask
                                ? m_pBitmap->m_pAlphaMask->GetBuffer() +
                                      dest_x + m_DestTop * dest_alpha_pitch
                                : nullptr;
  if (m_bFlipY) {
    dest_buf += dest_pitch * (m_DestHeight - 1);
    dest_alpha_buf += dest_alpha_pitch * (m_DestHeight - 1);
  }
  int y_step = dest_pitch;
  int y_alpha_step = dest_alpha_pitch;
  if (m_bFlipY) {
    y_step = -y_step;
    y_alpha_step = -y_alpha_step;
  }
  uint8_t* src_scan = m_pScanlineV;
  uint8_t* dest_scan = dest_buf;
  for (i = 0; i < m_DestHeight; i++) {
    for (int j = 0; j < Bpp; j++) {
      *src_scan++ = dest_scan[j];
    }
    dest_scan += y_step;
  }
  uint8_t* src_alpha_scan = m_pScanlineAlphaV;
  uint8_t* dest_alpha_scan = dest_alpha_buf;
  if (dest_alpha_scan) {
    for (i = 0; i < m_DestHeight; i++) {
      *src_alpha_scan++ = *dest_alpha_scan;
      dest_alpha_scan += y_alpha_step;
    }
  }
  uint8_t* clip_scan = nullptr;
  if (m_pClipMask) {
    clip_scan = m_pClipScanV;
    int clip_pitch = m_pClipMask->GetPitch();
    const uint8_t* src_clip =
        m_pClipMask->GetBuffer() +
        (m_DestTop - m_pClipRgn->GetBox().top) * clip_pitch +
        (dest_x - m_pClipRgn->GetBox().left);
    if (m_bFlipY) {
      src_clip += clip_pitch * (m_DestHeight - 1);
      clip_pitch = -clip_pitch;
    }
    for (i = 0; i < m_DestHeight; i++) {
      clip_scan[i] = *src_clip;
      src_clip += clip_pitch;
    }
  }
  DoCompose(m_pScanlineV, scanline, m_DestHeight, clip_scan, scan_extra_alpha,
            m_pScanlineAlphaV);
  src_scan = m_pScanlineV;
  dest_scan = dest_buf;
  for (i = 0; i < m_DestHeight; i++) {
    for (int j = 0; j < Bpp; j++) {
      dest_scan[j] = *src_scan++;
    }
    dest_scan += y_step;
  }
  src_alpha_scan = m_pScanlineAlphaV;
  dest_alpha_scan = dest_alpha_buf;
  if (dest_alpha_scan) {
    for (i = 0; i < m_DestHeight; i++) {
      *dest_alpha_scan = *src_alpha_scan++;
      dest_alpha_scan += y_alpha_step;
    }
  }
}
