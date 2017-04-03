// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "core/fxcodec/fx_codec.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/ge/cfx_cliprgn.h"

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
