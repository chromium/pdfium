// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_bitmapcomposer.h"

#include <string.h>

#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/cfx_cliprgn.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "third_party/base/check_op.h"

CFX_BitmapComposer::CFX_BitmapComposer() = default;

CFX_BitmapComposer::~CFX_BitmapComposer() = default;

void CFX_BitmapComposer::Compose(const RetainPtr<CFX_DIBitmap>& pDest,
                                 const CFX_ClipRgn* pClipRgn,
                                 int bitmap_alpha,
                                 uint32_t mask_color,
                                 const FX_RECT& dest_rect,
                                 bool bVertical,
                                 bool bFlipX,
                                 bool bFlipY,
                                 bool bRgbByteOrder,
                                 BlendMode blend_mode) {
  m_pBitmap = pDest;
  m_pClipRgn = pClipRgn;
  m_DestLeft = dest_rect.left;
  m_DestTop = dest_rect.top;
  m_DestWidth = dest_rect.Width();
  m_DestHeight = dest_rect.Height();
  m_BitmapAlpha = bitmap_alpha;
  m_MaskColor = mask_color;
  m_pClipMask = nullptr;
  if (pClipRgn && pClipRgn->GetType() != CFX_ClipRgn::kRectI)
    m_pClipMask = pClipRgn->GetMask();
  m_bVertical = bVertical;
  m_bFlipX = bFlipX;
  m_bFlipY = bFlipY;
  m_bRgbByteOrder = bRgbByteOrder;
  m_BlendMode = blend_mode;
}

bool CFX_BitmapComposer::SetInfo(int width,
                                 int height,
                                 FXDIB_Format src_format,
                                 pdfium::span<const uint32_t> src_palette) {
  DCHECK_NE(src_format, FXDIB_Format::k1bppMask);
  DCHECK_NE(src_format, FXDIB_Format::k1bppRgb);
  m_SrcFormat = src_format;
  if (!m_Compositor.Init(m_pBitmap->GetFormat(), src_format, src_palette,
                         m_MaskColor, m_BlendMode,
                         m_pClipMask != nullptr || (m_BitmapAlpha < 255),
                         m_bRgbByteOrder)) {
    return false;
  }
  if (m_bVertical) {
    m_pScanlineV.resize(m_pBitmap->GetBPP() / 8 * width + 4);
    m_pClipScanV.resize(m_pBitmap->GetHeight());
  }
  if (m_BitmapAlpha < 255) {
    m_pAddClipScan.resize(m_bVertical ? m_pBitmap->GetHeight()
                                      : m_pBitmap->GetWidth());
  }
  return true;
}

void CFX_BitmapComposer::DoCompose(pdfium::span<uint8_t> dest_scan,
                                   pdfium::span<const uint8_t> src_scan,
                                   int dest_width,
                                   pdfium::span<const uint8_t> clip_scan) {
  if (m_BitmapAlpha < 255) {
    if (!clip_scan.empty()) {
      for (int i = 0; i < dest_width; ++i)
        m_pAddClipScan[i] = clip_scan[i] * m_BitmapAlpha / 255;
    } else {
      fxcrt::spanset(pdfium::make_span(m_pAddClipScan).first(dest_width),
                     m_BitmapAlpha);
    }
    clip_scan = m_pAddClipScan;
  }
  if (m_SrcFormat == FXDIB_Format::k8bppMask) {
    m_Compositor.CompositeByteMaskLine(dest_scan, src_scan, dest_width,
                                       clip_scan);
  } else if (m_SrcFormat == FXDIB_Format::k8bppRgb) {
    m_Compositor.CompositePalBitmapLine(dest_scan, src_scan, 0, dest_width,
                                        clip_scan);
  } else {
    m_Compositor.CompositeRgbBitmapLine(dest_scan, src_scan, dest_width,
                                        clip_scan);
  }
}

void CFX_BitmapComposer::ComposeScanline(int line,
                                         pdfium::span<const uint8_t> scanline) {
  if (m_bVertical) {
    ComposeScanlineV(line, scanline);
    return;
  }
  pdfium::span<const uint8_t> clip_scan;
  if (m_pClipMask) {
    clip_scan =
        m_pClipMask
            ->GetWritableScanline(m_DestTop + line - m_pClipRgn->GetBox().top)
            .subspan(m_DestLeft - m_pClipRgn->GetBox().left);
  }
  pdfium::span<uint8_t> dest_scan =
      m_pBitmap->GetWritableScanline(line + m_DestTop);
  if (!dest_scan.empty()) {
    FX_SAFE_UINT32 offset = m_DestLeft;
    offset *= m_pBitmap->GetBPP();
    offset /= 8;
    if (!offset.IsValid())
      return;

    dest_scan = dest_scan.subspan(offset.ValueOrDie());
  }
  DoCompose(dest_scan, scanline, m_DestWidth, clip_scan);
}

void CFX_BitmapComposer::ComposeScanlineV(
    int line,
    pdfium::span<const uint8_t> scanline) {
  int Bpp = m_pBitmap->GetBPP() / 8;
  int dest_pitch = m_pBitmap->GetPitch();
  int dest_x = m_DestLeft + (m_bFlipX ? (m_DestWidth - line - 1) : line);
  pdfium::span<uint8_t> dest_span = m_pBitmap->GetBuffer();
  if (!dest_span.empty()) {
    const size_t dest_x_offset = Fx2DSizeOrDie(dest_x, Bpp);
    const size_t dest_y_offset = Fx2DSizeOrDie(m_DestTop, dest_pitch);
    dest_span = dest_span.subspan(dest_y_offset).subspan(dest_x_offset);
    if (m_bFlipY) {
      const size_t dest_flip_offset =
          Fx2DSizeOrDie(dest_pitch, m_DestHeight - 1);
      dest_span = dest_span.subspan(dest_flip_offset);
    }
  }
  uint8_t* dest_buf = dest_span.data();
  const int y_step = m_bFlipY ? -dest_pitch : dest_pitch;
  uint8_t* src_scan = m_pScanlineV.data();
  uint8_t* dest_scan = dest_buf;
  for (int i = 0; i < m_DestHeight; ++i) {
    for (int j = 0; j < Bpp; ++j)
      *src_scan++ = dest_scan[j];
    dest_scan += y_step;
  }
  pdfium::span<uint8_t> clip_scan;
  if (m_pClipMask) {
    clip_scan = m_pClipScanV;
    int clip_pitch = m_pClipMask->GetPitch();
    const uint8_t* src_clip =
        m_pClipMask->GetScanline(m_DestTop - m_pClipRgn->GetBox().top)
            .subspan(dest_x - m_pClipRgn->GetBox().left)
            .data();
    if (m_bFlipY) {
      src_clip += Fx2DSizeOrDie(clip_pitch, m_DestHeight - 1);
      clip_pitch = -clip_pitch;
    }
    for (int i = 0; i < m_DestHeight; ++i) {
      clip_scan[i] = *src_clip;
      src_clip += clip_pitch;
    }
  }
  DoCompose(m_pScanlineV, scanline, m_DestHeight, clip_scan);
  src_scan = m_pScanlineV.data();
  dest_scan = dest_buf;
  for (int i = 0; i < m_DestHeight; ++i) {
    for (int j = 0; j < Bpp; ++j)
      dest_scan[j] = *src_scan++;
    dest_scan += y_step;
  }
}
