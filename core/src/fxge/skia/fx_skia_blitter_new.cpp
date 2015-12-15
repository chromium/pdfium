// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/include/fxge/fx_ge.h"

//#define _SKIA_SUPPORT_
#if defined(_SKIA_SUPPORT_)
#include "SkBlitter.h"
#include "core/include/fxcodec/fx_codec.h"
#include "fx_skia_blitter_new.h"

// We use our own renderer here to make it simple
void CFX_SkiaRenderer::blitAntiH(int x,
                                 int y,
                                 const SkAlpha antialias[],
                                 const int16_t runs[]) {
  FXSYS_assert(m_Alpha);
  if (!m_pOriDevice && !composite_span)
    return;
  if (y < m_ClipBox.top || y >= m_ClipBox.bottom)
    return;
  while (1) {
    int width = runs[0];
    SkASSERT(width >= 0);
    if (width <= 0)
      return;
    unsigned aa = antialias[0];
    if (aa)
      (*composite_span)(m_pDestScan, m_pOriScan, 0, x, width, y, aa,
                        m_ClipBox.top, m_ClipBox.left, m_ClipBox.right,
                        m_pClipScan, m_pDestExtraAlphaScan);
    runs += width;
    antialias += width;
    x += width;
  }
}

void CFX_SkiaRenderer::blitH(int x, int y, int width) {
  FXSYS_assert(m_Alpha && width);
  if (y < m_ClipBox.top || y >= m_ClipBox.bottom)
    return;
  (*composite_span)(m_pDestScan, m_pOriScan, 0, x, width, y, 255, m_ClipBox.top,
                    m_ClipBox.left, m_ClipBox.right, m_pClipScan,
                    m_pDestExtraAlphaScan);
}

void CFX_SkiaRenderer::blitV(int x, int y, int height, SkAlpha alpha) {
  FXSYS_assert(m_Alpha && alpha);
  if (alpha == 255) {
    blitRect(x, y, 1, height);
  } else {
    int16_t runs[2];
    runs[0] = 1;
    runs[1] = 0;
    while (--height >= 0) {
      if (y >= m_ClipBox.bottom)
        return;
      blitAntiH(x, y++, &alpha, runs);
    }
  }
}
void CFX_SkiaRenderer::blitRect(int x, int y, int width, int height) {
  FXSYS_assert(m_Alpha && width);
  while (--height >= 0) {
    if (y >= m_ClipBox.bottom)
      return;
    blitH(x, y++, width);
  }
}

void CFX_SkiaRenderer::blitAntiRect(int x,
                                    int y,
                                    int width,
                                    int height,
                                    SkAlpha leftAlpha,
                                    SkAlpha rightAlpha) {
  blitV(x++, y, height, leftAlpha);
  if (width > 0) {
    blitRect(x, y, width, height);
    x += width;
  }
  blitV(x, y, height, rightAlpha);
}
/*---------------------------------------------------------------------------------------------------*/
void CFX_SkiaRenderer::CompositeSpan1bpp_0(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_bRgbByteOrder);
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left / 8;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start / 8;

  int index = 0;
  if (m_pDevice->GetPalette()) {
    for (int i = 0; i < 2; i++) {
      if (FXARGB_TODIB(m_pDevice->GetPalette()[i]) == m_Color)
        index = i;
    }
  } else {
    index = ((uint8_t)m_Color == 0xff) ? 1 : 0;
  }
  uint8_t* dest_scan1 = dest_scan;
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    if (src_alpha) {
      if (!index)
        *dest_scan1 &= ~(1 << (7 - (col + span_left) % 8));
      else
        *dest_scan1 |= 1 << (7 - (col + span_left) % 8);
    }
    dest_scan1 = dest_scan + (span_left % 8 + col - col_start + 1) / 8;
  }
}
void CFX_SkiaRenderer::CompositeSpan1bpp_4(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_bRgbByteOrder);
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left / 8;
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start / 8;

  int index = 0;
  if (m_pDevice->GetPalette()) {
    for (int i = 0; i < 2; i++) {
      if (FXARGB_TODIB(m_pDevice->GetPalette()[i]) == m_Color)
        index = i;
    }
  } else {
    index = ((uint8_t)m_Color == 0xff) ? 1 : 0;
  }
  uint8_t* dest_scan1 = dest_scan;
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (src_alpha1) {
      if (!index)
        *dest_scan1 &= ~(1 << (7 - (col + span_left) % 8));
      else
        *dest_scan1 |= 1 << (7 - (col + span_left) % 8);
    }
    dest_scan1 = dest_scan + (span_left % 8 + col - col_start + 1) / 8;
  }
}
/*-----------------------------------------------------------------------------------------------------*/
void CFX_SkiaRenderer::CompositeSpanGray_2(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  ASSERT(!m_bRgbByteOrder);
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start;
  if (cover_scan == 255 && m_Alpha == 255) {
    FXSYS_memset(dest_scan, FXARGB_MAKE(m_Gray, m_Gray, m_Gray, m_Gray),
                 col_end - col_start);
    return;
  }
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Gray, src_alpha);
    dest_scan++;
  }
}
void CFX_SkiaRenderer::CompositeSpanGray_3(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  ASSERT(!m_bRgbByteOrder);
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left;
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start;
  ori_scan += col_start;
  if (m_Alpha == 255 && cover_scan == 255) {
    FXSYS_memset(dest_scan, FXARGB_MAKE(m_Gray, m_Gray, m_Gray, m_Gray),
                 col_end - col_start);
  } else {
    int src_alpha = m_Alpha;
#if 1
    for (int col = col_start; col < col_end; col++) {
      int gray = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, cover_scan);
      dest_scan++;
    }
#else
    if (m_bFullCover) {
      if (src_alpha == 255) {
        FXSYS_memset(dest_scan, FXARGB_MAKE(m_Gray, m_Gray, m_Gray, m_Gray),
                     col_end - col_start);
        return;
      }
      for (int col = col_start; col < col_end; col++)
        *dest_scan = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
    } else {
      for (int col = col_start; col < col_end; col++) {
        int gray = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, cover_scan);
        dest_scan++;
      }
    }
#endif
  }
}

void CFX_SkiaRenderer::CompositeSpanGray_6(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_bRgbByteOrder);
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left;
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start;
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (!src_alpha1) {
      dest_scan++;
      continue;
    }
    if (src_alpha1 == 255)
      *dest_scan++ = m_Gray;
    else {
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Gray, src_alpha1);
      dest_scan++;
    }
  }
}

void CFX_SkiaRenderer::CompositeSpanGray_7(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + span_left;
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + span_left;
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start;
  ori_scan += col_start;
#if 1
  for (int col = col_start; col < col_end; col++) {
    int src_alpha = m_Alpha * clip_scan[col] / 255;
    if (src_alpha == 255 && cover_scan == 255) {
      *dest_scan++ = m_Gray;
      ori_scan++;
      continue;
    }
    int gray = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, cover_scan);
    dest_scan++;
  }

#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        dest_scan++;
        ori_scan++;
        continue;
      }
      if (src_alpha == 255) {
        *dest_scan++ = m_Gray;
        ori_scan++;
        continue;
      }
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
    }
  } else {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (src_alpha == 255 && cover_scan == 255) {
        *dest_scan++ = m_Gray;
        ori_scan++;
        continue;
      }
      int gray = FXDIB_ALPHA_MERGE(*ori_scan++, m_Gray, src_alpha);
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, gray, cover_scan);
      dest_scan++;
    }
  }
#endif
}
/*--------------------------------------------------------------------------------------------------*/

void CFX_SkiaRenderer::CompositeSpanARGB_2(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
  if (m_Alpha == 255 && cover_scan == 255) {
    FXSYS_memset(dest_scan, m_Color, (col_end - col_start) << 2);
    return;
  }
  int src_alpha;
#if 0
        if (m_bFullCover) {
            if (m_Alpha == 255) {
                FXSYS_memset(dest_scan, m_Color, (col_end - col_start)<<2);
                return;
            }
        }
        else
#endif
  src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    // Dest format: Argb
    // calculate destination alpha (it's union of source and dest alpha)
    if (dest_scan[3] == 0) {
      dest_scan[3] = src_alpha;
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan = m_Red;
      dest_scan += 2;
      continue;
    }
    uint8_t dest_alpha =
        dest_scan[3] + src_alpha - dest_scan[3] * src_alpha / 255;
    dest_scan[3] = dest_alpha;
    int alpha_ratio = src_alpha * 255 / dest_alpha;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
    dest_scan += 2;
  }
}

void CFX_SkiaRenderer::CompositeSpanARGB_3(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  // ori_scan  = (uint8_t*)m_pOriDevice->GetScanline(span_top) + (span_left<<2);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
  // ori_scan += col_start << 2;

  if (m_Alpha == 255 && cover_scan == 255) {
    FXSYS_memset(dest_scan, m_Color, (col_end - col_start) << 2);
    return;
  }
  if (cover_scan == 255) {
    int dst_color = (0x00ffffff & m_Color) | (m_Alpha << 24);
    FXSYS_memset(dest_scan, dst_color, (col_end - col_start) << 2);
    return;
  }
  // Do not need origin bitmap, because of merge in pure transparent background
  int src_alpha_covered = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    // shortcut
    if (dest_scan[3] == 0) {
      dest_scan[3] = src_alpha_covered;
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan = m_Red;
      dest_scan += 2;
      continue;
    }
    // We should do alpha transition and color transition
    // alpha fg          color fg
    // alpha bg          color bg
    // alpha cover       color cover
    dest_scan[3] = FXDIB_ALPHA_MERGE(dest_scan[3], m_Alpha, cover_scan);
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, cover_scan);
    dest_scan += 2;
  }
}
void CFX_SkiaRenderer::CompositeSpanARGB_6(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
#if 1
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (!src_alpha1) {
      dest_scan += 4;
      continue;
    }
    if (src_alpha1 == 255) {
      *(FX_DWORD*)dest_scan = m_Color;
      dest_scan += 4;
    } else {
      // Dest format: Argb
      // calculate destination alpha (it's union of source and dest alpha)
      if (dest_scan[3] == 0) {
        dest_scan[3] = src_alpha1;
        *dest_scan++ = m_Blue;
        *dest_scan++ = m_Green;
        *dest_scan = m_Red;
        dest_scan += 2;
        continue;
      }
      uint8_t dest_alpha =
          dest_scan[3] + src_alpha1 - dest_scan[3] * src_alpha1 / 255;
      dest_scan[3] = dest_alpha;
      int alpha_ratio = src_alpha1 * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
      dest_scan += 2;
    }
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        dest_scan += 4;
        continue;
      }
      if (src_alpha == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
        continue;
      } else {
        // Dest format: Argb
        // calculate destination alpha (it's union of source and dest alpha)
        if (dest_scan[3] == 0) {
          dest_scan[3] = src_alpha;
          *dest_scan++ = m_Blue;
          *dest_scan++ = m_Green;
          *dest_scan = m_Red;
          dest_scan += 2;
          continue;
        }
        uint8_t dest_alpha =
            dest_scan[3] + src_alpha - dest_scan[3] * src_alpha / 255;
        dest_scan[3] = dest_alpha;
        int alpha_ratio = src_alpha * 255 / dest_alpha;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
        dest_scan += 2;
      }
    }
  } else {
    int src_alpha = m_Alpha * cover_scan / 255;
    for (int col = col_start; col < col_end; col++) {
      int src_alpha1 = src_alpha * clip_scan[col] / 255;
      if (!src_alpha1) {
        dest_scan += 4;
        continue;
      }
      if (src_alpha1 == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
      } else {
        // Dest format: Argb
        // calculate destination alpha (it's union of source and dest alpha)
        if (dest_scan[3] == 0) {
          dest_scan[3] = src_alpha1;
          *dest_scan++ = m_Blue;
          *dest_scan++ = m_Green;
          *dest_scan = m_Red;
          dest_scan += 2;
          continue;
        }
        uint8_t dest_alpha =
            dest_scan[3] + src_alpha1 - dest_scan[3] * src_alpha1 / 255;
        dest_scan[3] = dest_alpha;
        int alpha_ratio = src_alpha1 * 255 / dest_alpha;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
        dest_scan += 2;
      }
    }
  }
#endif
}

void CFX_SkiaRenderer::CompositeSpanARGB_7(uint8_t* dest_scan,
                                           uint8_t* ori_scan,
                                           int Bpp,
                                           int span_left,
                                           int span_len,
                                           int span_top,
                                           uint8_t cover_scan,
                                           int clip_top,
                                           int clip_left,
                                           int clip_right,
                                           uint8_t* clip_scan,
                                           uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  // ori_scan  = (uint8_t*)m_pOriDevice->GetScanline(span_top) + (span_left<<2);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
  // ori_scan += col_start << 2;
  // Do not need origin bitmap, because of merge in pure transparent background
  for (int col = col_start; col < col_end; col++) {
    int src_alpha = m_Alpha * clip_scan[col] / 255;
    int src_alpha_covered = src_alpha * cover_scan / 255;
    // shortcut
    if (src_alpha_covered == 0) {
      dest_scan += 4;
      continue;
    }
    // shortcut
    if (cover_scan == 255 || dest_scan[3] == 0) {
      // origin alpha always zero, just get src alpha
      dest_scan[3] = src_alpha_covered;
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan = m_Red;
      dest_scan += 2;
      continue;
    }
    // We should do alpha transition and color transition
    // alpha fg          color fg
    // alpha bg          color bg
    // alpha cover       color cover
    dest_scan[3] = FXDIB_ALPHA_MERGE(dest_scan[3], src_alpha, cover_scan);
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, cover_scan);
    dest_scan += 2;
  }
}

/*-----------------------------------------------------------------------------------------------------------*/
void CFX_SkiaRenderer::CompositeSpanRGB32_2(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += (col_start << 2);
  if (m_Alpha == 255 && cover_scan == 255) {
    FXSYS_memset(dest_scan, m_Color, (col_end - col_start) << 2);
    return;
  }
  int src_alpha;
#if 0
        if (m_bFullCover)
            src_alpha = m_Alpha;
        else
#endif
  src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    // Dest format:  Rgb32
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha);
    dest_scan += 2;
  }
}
void CFX_SkiaRenderer::CompositeSpanRGB32_3(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + (span_left << 2);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
  ori_scan += col_start << 2;
  if (m_Alpha == 255 && cover_scan == 255) {
    FXSYS_memset(dest_scan, m_Color, (col_end - col_start) << 2);
    return;
  }
  int src_alpha = m_Alpha;
  for (int col = col_start; col < col_end; col++) {
#if 0
            if (m_bFullCover) {
                *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
                *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
                *dest_scan = FXDIB_ALPHA_MERGE(*ori_scan, m_Red, src_alpha);
                dest_scan += 2; ori_scan += 2;
                continue;
            }
#endif
    int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
    int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
    int r = FXDIB_ALPHA_MERGE(*ori_scan, m_Red, src_alpha);
    ori_scan += 2;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
    dest_scan += 2;
  }
}
void CFX_SkiaRenderer::CompositeSpanRGB32_6(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
#if 1
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (!src_alpha1) {
      dest_scan += 4;
      continue;
    }
    if (src_alpha1 == 255) {
      *(FX_DWORD*)dest_scan = m_Color;
      dest_scan += 4;
    } else {
      // Dest format: Rgb or Rgb32
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha1);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha1);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha1);
      dest_scan += 2;
    }
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        dest_scan += 4;
        continue;
      }
      if (src_alpha == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
      } else {
        // Dest format: Rgb or Rgb32
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha);
        dest_scan += 2;
      }
    }
  } else {
    // Rgb32
    int src_alpha = m_Alpha * cover_scan / 255;
    for (int col = col_start; col < col_end; col++) {
      int src_alpha1 = src_alpha * clip_scan[col] / 255;
      if (!src_alpha1) {
        dest_scan += 4;
        continue;
      }
      if (src_alpha1 == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
      } else {
        // Dest format: Rgb or Rgb32
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha1);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha1);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha1);
        dest_scan += 2;
      }
    }
  }
#endif
}
void CFX_SkiaRenderer::CompositeSpanRGB32_7(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan = (uint8_t*)m_pDevice->GetScanline(span_top) + (span_left << 2);
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + (span_left << 2);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start << 2;
  ori_scan += col_start << 2;
#if 1
  for (int col = col_start; col < col_end; col++) {
    int src_alpha = m_Alpha * clip_scan[col] / 255;
    if (src_alpha == 255 && cover_scan == 255) {
      *(FX_DWORD*)dest_scan = m_Color;
      dest_scan += 4;
      ori_scan += 4;
      continue;
    }
    int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
    int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
    int r = FXDIB_ALPHA_MERGE(*ori_scan, m_Red, src_alpha);
    ori_scan += 2;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
    dest_scan += 2;
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        *(FX_DWORD*)dest_scan = *(FX_DWORD*)ori_scan;
        dest_scan += 4;
        ori_scan += 4;
        continue;
      }
      if (src_alpha == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
        ori_scan += 4;
        continue;
      }
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
      *dest_scan = FXDIB_ALPHA_MERGE(*ori_scan, m_Red, src_alpha);
      dest_scan += 2;
      ori_scan += 2;
    }
  } else {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (src_alpha == 255 && cover_scan == 255) {
        *(FX_DWORD*)dest_scan = m_Color;
        dest_scan += 4;
        ori_scan += 4;
        continue;
      }
      int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
      int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
      int r = FXDIB_ALPHA_MERGE(*ori_scan, m_Red, src_alpha);
      ori_scan += 2;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
      dest_scan += 2;
    }
  }
#endif
}
/*-----------------------------------------------------------------------------------------------------*/
void CFX_SkiaRenderer::CompositeSpanRGB24_2(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += (col_start << 1) + col_start;
  int src_alpha;
#if 0
        if (m_bFullCover)
            src_alpha = m_Alpha;
        else
#endif
  src_alpha = m_Alpha * cover_scan / 255;
  if (src_alpha == 255) {
    for (int col = col_start; col < col_end; col++) {
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan++ = m_Red;
    }
    return;
  }
  for (int col = col_start; col < col_end; col++) {
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha);
    dest_scan++;
  }
}
void CFX_SkiaRenderer::CompositeSpanRGB24_3(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + span_left +
             (span_left << 1);
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += (col_start << 1) + col_start;
  ori_scan += (col_start << 1) + col_start;
  if (m_Alpha == 255 && cover_scan == 255) {
    for (int col = col_start; col < col_end; col++) {
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan++ = m_Red;
    }
    return;
  }
  for (int col = col_start; col < col_end; col++) {
#if 0
            if (m_bFullCover) {
                *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, m_Alpha);
                *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, m_Alpha);
                *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Red, m_Alpha);
                continue;
            }
#endif
    int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, m_Alpha);
    int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, m_Alpha);
    int r = FXDIB_ALPHA_MERGE(*ori_scan++, m_Red, m_Alpha);
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
    dest_scan++;
  }
}
void CFX_SkiaRenderer::CompositeSpanRGB24_6(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start + (col_start << 1);
#if 1
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (!src_alpha1) {
      dest_scan += 3;
      continue;
    }
    if (src_alpha1 == 255) {
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan++ = m_Red;
    } else {
      // Dest format: Rgb
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha1);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha1);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha1);
      dest_scan++;
    }
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        dest_scan += 3;
        continue;
      }
      if (src_alpha == 255) {
        *dest_scan++ = m_Blue;
        *dest_scan++ = m_Green;
        *dest_scan++ = m_Red;
      } else {
        // Dest format: Rgb
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha);
        dest_scan++;
      }
    }
  } else {
    int src_alpha = m_Alpha * cover_scan / 255;
    for (int col = col_start; col < col_end; col++) {
      int src_alpha1 = src_alpha * clip_scan[col] / 255;
      if (!src_alpha1) {
        dest_scan += 3;
        continue;
      }
      if (src_alpha1 == 255) {
        *dest_scan++ = m_Blue;
        *dest_scan++ = m_Green;
        *dest_scan++ = m_Red;
      } else {
        // Dest format: Rgb
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha1);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha1);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha1);
        dest_scan++;
      }
    }
  }
#endif
}
void CFX_SkiaRenderer::CompositeSpanRGB24_7(uint8_t* dest_scan,
                                            uint8_t* ori_scan,
                                            int Bpp,
                                            int span_left,
                                            int span_len,
                                            int span_top,
                                            uint8_t cover_scan,
                                            int clip_top,
                                            int clip_left,
                                            int clip_right,
                                            uint8_t* clip_scan,
                                            uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_pDevice->IsCmykImage());
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  ori_scan = (uint8_t*)m_pOriDevice->GetScanline(span_top) + span_left +
             (span_left << 1);
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start + (col_start << 1);
  ori_scan += col_start + (col_start << 1);
#if 1
  for (int col = col_start; col < col_end; col++) {
    int src_alpha = m_Alpha * clip_scan[col] / 255;
    if (src_alpha == 255 && cover_scan == 255) {
      *dest_scan++ = m_Blue;
      *dest_scan++ = m_Green;
      *dest_scan++ = m_Red;
      ori_scan += 3;
      continue;
    }
    int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
    int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
    int r = FXDIB_ALPHA_MERGE(*ori_scan++, m_Red, src_alpha);
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
    dest_scan++;
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        *dest_scan++ = *ori_scan++;
        *dest_scan++ = *ori_scan++;
        *dest_scan++ = *ori_scan++;
        continue;
      }
      if (src_alpha == 255) {
        *dest_scan++ = m_Blue;
        *dest_scan++ = m_Green;
        *dest_scan++ = m_Red;
        ori_scan += 3;
        continue;
      }
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
      *dest_scan++ = FXDIB_ALPHA_MERGE(*ori_scan++, m_Red, src_alpha);
    }
  } else {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (src_alpha == 255 && cover_scan == 255) {
        *dest_scan++ = m_Blue;
        *dest_scan++ = m_Green;
        *dest_scan++ = m_Red;
        ori_scan += 3;
        continue;
      }
      int b = FXDIB_ALPHA_MERGE(*ori_scan++, m_Blue, src_alpha);
      int g = FXDIB_ALPHA_MERGE(*ori_scan++, m_Green, src_alpha);
      int r = FXDIB_ALPHA_MERGE(*ori_scan++, m_Red, src_alpha);
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, b, cover_scan);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, g, cover_scan);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, r, cover_scan);
      dest_scan++;
    }
  }
#endif
}
void CFX_SkiaRenderer::CompositeSpanRGB24_10(uint8_t* dest_scan,
                                             uint8_t* ori_scan,
                                             int Bpp,
                                             int span_left,
                                             int span_len,
                                             int span_top,
                                             uint8_t cover_scan,
                                             int clip_top,
                                             int clip_left,
                                             int clip_right,
                                             uint8_t* clip_scan,
                                             uint8_t* dest_extra_alpha_scan) {
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  dest_extra_alpha_scan =
      (uint8_t*)m_pDevice->m_pAlphaMask->GetScanline(span_top) + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start + (col_start << 1);
#if 1
  if (m_Alpha == 255 && cover_scan == 255) {
    for (int col = col_start; col < col_end; col++) {
      *dest_scan++ = (uint8_t)m_Blue;
      *dest_scan++ = (uint8_t)m_Green;
      *dest_scan++ = (uint8_t)m_Red;
      *dest_extra_alpha_scan++ = 255;
    }
    return;
  }
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    // Dest format: Rgba
    // calculate destination alpha (it's union of source and dest alpha)
    uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha -
                         (*dest_extra_alpha_scan) * src_alpha / 255;
    *dest_extra_alpha_scan++ = dest_alpha;
    int alpha_ratio = src_alpha * 255 / dest_alpha;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
    dest_scan++;
    *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
    dest_scan++;
  }
#else
  if (m_bFullCover) {
    if (m_Alpha == 255) {
      for (int col = col_start; col < col_end; col++) {
        *dest_scan++ = (uint8_t)m_Blue;
        *dest_scan++ = (uint8_t)m_Green;
        *dest_scan++ = (uint8_t)m_Red;
        *dest_extra_alpha_scan++ = 255;
      }
      return;
    }
    for (int col = col_start; col < col_end; col++) {
      // Dest format: Rgba
      // calculate destination alpha (it's union of source and dest alpha)
      uint8_t dest_alpha = (*dest_extra_alpha_scan) + m_Alpha -
                           (*dest_extra_alpha_scan) * m_Alpha / 255;
      *dest_extra_alpha_scan++ = dest_alpha;
      int alpha_ratio = m_Alpha * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
      dest_scan++;
    }
  } else {
    if (m_Alpha == 255 && cover_scan == 255) {
      for (int col = col_start; col < col_end; col++) {
        *dest_scan++ = (uint8_t)m_Blue;
        *dest_scan++ = (uint8_t)m_Green;
        *dest_scan++ = (uint8_t)m_Red;
        *dest_extra_alpha_scan++ = 255;
      }
      return;
    }
    int src_alpha = m_Alpha * cover_scan / 255;
    for (int col = col_start; col < col_end; col++) {
      // Dest format: Rgba
      // calculate destination alpha (it's union of source and dest alpha)
      uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha -
                           (*dest_extra_alpha_scan) * src_alpha / 255;
      *dest_extra_alpha_scan++ = dest_alpha;
      int alpha_ratio = src_alpha * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
      dest_scan++;
    }
  }
#endif
}
void CFX_SkiaRenderer::CompositeSpanRGB24_14(uint8_t* dest_scan,
                                             uint8_t* ori_scan,
                                             int Bpp,
                                             int span_left,
                                             int span_len,
                                             int span_top,
                                             uint8_t cover_scan,
                                             int clip_top,
                                             int clip_left,
                                             int clip_right,
                                             uint8_t* clip_scan,
                                             uint8_t* dest_extra_alpha_scan) {
  dest_scan =
      (uint8_t*)m_pDevice->GetScanline(span_top) + span_left + (span_left << 1);
  dest_extra_alpha_scan =
      (uint8_t*)m_pDevice->m_pAlphaMask->GetScanline(span_top) + span_left;
  clip_scan = (uint8_t*)m_pClipMask->GetScanline(span_top - clip_top) -
              clip_left + span_left;
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start + (col_start << 1);
#if 1
  int src_alpha = m_Alpha * cover_scan / 255;
  for (int col = col_start; col < col_end; col++) {
    int src_alpha1 = src_alpha * clip_scan[col] / 255;
    if (!src_alpha1) {
      dest_extra_alpha_scan++;
      dest_scan += 3;
      continue;
    }
    if (src_alpha1 == 255) {
      *dest_scan++ = (uint8_t)m_Blue;
      *dest_scan++ = (uint8_t)m_Green;
      *dest_scan++ = (uint8_t)m_Red;
      *dest_extra_alpha_scan++ = (uint8_t)m_Alpha;
    } else {
      // Dest format: Rgba
      // calculate destination alpha (it's union of source and dest alpha)
      uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha1 -
                           (*dest_extra_alpha_scan) * src_alpha1 / 255;
      *dest_extra_alpha_scan++ = dest_alpha;
      int alpha_ratio = src_alpha1 * 255 / dest_alpha;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
      dest_scan++;
      *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
      dest_scan++;
    }
  }
#else
  if (m_bFullCover) {
    for (int col = col_start; col < col_end; col++) {
      int src_alpha = m_Alpha * clip_scan[col] / 255;
      if (!src_alpha) {
        dest_extra_alpha_scan++;
        dest_scan += 3;
        continue;
      }
      if (src_alpha == 255) {
        *dest_scan++ = (uint8_t)m_Blue;
        *dest_scan++ = (uint8_t)m_Green;
        *dest_scan++ = (uint8_t)m_Red;
        *dest_extra_alpha_scan++ = (uint8_t)m_Alpha;
      } else {
        // Dest format: Rgba
        // calculate destination alpha (it's union of source and dest alpha)
        uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha -
                             (*dest_extra_alpha_scan) * src_alpha / 255;
        *dest_extra_alpha_scan++ = dest_alpha;
        int alpha_ratio = src_alpha * 255 / dest_alpha;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
        dest_scan++;
      }
    }
  } else {
    int src_alpha = m_Alpha * cover_scan / 255;
    for (int col = col_start; col < col_end; col++) {
      int src_alpha1 = m_Alpha * cover_scan * clip_scan[col] / 255;
      if (!src_alpha1) {
        dest_extra_alpha_scan++;
        dest_scan += 3;
        continue;
      }
      if (src_alpha1 == 255) {
        *dest_scan++ = (uint8_t)m_Blue;
        *dest_scan++ = (uint8_t)m_Green;
        *dest_scan++ = (uint8_t)m_Red;
        *dest_extra_alpha_scan++ = (uint8_t)m_Alpha;
      } else {
        // Dest format: Rgba
        // calculate destination alpha (it's union of source and dest alpha)
        uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha1 -
                             (*dest_extra_alpha_scan) * src_alpha1 / 255;
        *dest_extra_alpha_scan++ = dest_alpha;
        int alpha_ratio = src_alpha1 * 255 / dest_alpha;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
        dest_scan++;
        *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
        dest_scan++;
      }
    }
  }
#endif
}
/*-----------------------------------------------------------------------------------------------------*/

// A general alpha merge function (with clipping mask). Cmyka/Cmyk device.
void CFX_SkiaRenderer::CompositeSpanCMYK(uint8_t* dest_scan,
                                         uint8_t* ori_scan,
                                         int Bpp,
                                         int span_left,
                                         int span_len,
                                         int span_top,
                                         uint8_t cover_scan,
                                         int clip_top,
                                         int clip_left,
                                         int clip_right,
                                         uint8_t* clip_scan,
                                         uint8_t* dest_extra_alpha_scan) {
  ASSERT(!m_bRgbByteOrder);
  // Cmyk(a)
  int col_start = span_left < clip_left ? clip_left - span_left : 0;
  int col_end =
      (span_left + span_len) < clip_right ? span_len : (clip_right - span_left);
  if (col_end < col_start)
    return;  // do nothing.
  dest_scan += col_start * 4;
  Bpp;  // for avoid compile warning.

  if (dest_extra_alpha_scan) {
    // CMYKa
    for (int col = col_start; col < col_end; col++) {
      int src_alpha;
      if (m_bFullCover) {
        if (clip_scan)
          src_alpha = m_Alpha * clip_scan[col] / 255;
        else
          src_alpha = m_Alpha;
      } else {
        if (clip_scan)
          src_alpha = m_Alpha * cover_scan * clip_scan[col] / 255 / 255;
        else
          src_alpha = m_Alpha * cover_scan / 255;
      }

      if (src_alpha) {
        if (src_alpha == 255) {
          *(FX_CMYK*)dest_scan = m_Color;
          *dest_extra_alpha_scan = (uint8_t)m_Alpha;
        } else {
          // Dest format: Cmyka
          // calculate destination alpha (it's union of source and dest alpha)
          uint8_t dest_alpha = (*dest_extra_alpha_scan) + src_alpha -
                               (*dest_extra_alpha_scan) * src_alpha / 255;
          *dest_extra_alpha_scan++ = dest_alpha;
          int alpha_ratio = src_alpha * 255 / dest_alpha;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, alpha_ratio);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, alpha_ratio);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, alpha_ratio);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Gray, alpha_ratio);
          dest_scan++;
          continue;
        }
      }
      dest_extra_alpha_scan++;
      dest_scan += 4;
    }
  } else {
    // CMYK
    for (int col = col_start; col < col_end; col++) {
      int src_alpha;
      if (clip_scan)
        src_alpha = m_Alpha * cover_scan * clip_scan[col] / 255 / 255;
      else
        src_alpha = m_Alpha * cover_scan / 255;

      if (src_alpha) {
        if (src_alpha == 255) {
          *(FX_CMYK*)dest_scan = m_Color;
        } else {
          // Dest format: cmyk
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Red, src_alpha);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Green, src_alpha);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Blue, src_alpha);
          dest_scan++;
          *dest_scan = FXDIB_ALPHA_MERGE(*dest_scan, m_Gray, src_alpha);
          dest_scan++;
          continue;
        }
      }
      dest_scan += 4;
    }
  }
}

//--------------------------------------------------------------------
FX_BOOL CFX_SkiaRenderer::Init(
    CFX_DIBitmap* pDevice,
    CFX_DIBitmap* pOriDevice,
    const CFX_ClipRgn* pClipRgn,
    FX_DWORD color,
    FX_BOOL bFullCover,
    FX_BOOL bRgbByteOrder,
    int alpha_flag,
    void* pIccTransform)  // The alpha flag must be fill_flag if exist.
{
  m_pDevice = pDevice;
  m_pClipRgn = pClipRgn;
  m_bRgbByteOrder = bRgbByteOrder;
  m_pOriDevice = pOriDevice;
  m_pDestScan = NULL;
  m_pDestExtraAlphaScan = NULL;
  m_pOriScan = NULL;
  m_pClipScan = NULL;
  composite_span = NULL;
  if (m_pClipRgn)
    m_ClipBox = m_pClipRgn->GetBox();
  else {
    m_ClipBox.left = m_ClipBox.top = 0;
    m_ClipBox.right = m_pDevice->GetWidth();
    m_ClipBox.bottom = m_pDevice->GetHeight();
  }
  m_pClipMask = NULL;
  if (m_pClipRgn && m_pClipRgn->GetType() == CFX_ClipRgn::MaskF) {
    m_pClipMask = m_pClipRgn->GetMask();
    m_pClipScan = m_pClipMask->GetBuffer();
  }
  if (m_pDevice->m_pAlphaMask)
    m_pDestExtraAlphaScan = m_pDevice->m_pAlphaMask->GetBuffer();
  if (m_pOriDevice)
    m_pOriScan = m_pOriDevice->GetBuffer();
  m_pDestScan = m_pDevice->GetBuffer();

  m_bFullCover = bFullCover;

  FX_BOOL bObjectCMYK = FXGETFLAG_COLORTYPE(alpha_flag);
  FX_BOOL bDeviceCMYK = pDevice->IsCmykImage();

  m_Alpha = bObjectCMYK ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(color);

  ICodec_IccModule* pIccModule = NULL;
  // No lcms engine, we skip the transform
  if (!CFX_GEModule::Get()->GetCodecModule() ||
      !CFX_GEModule::Get()->GetCodecModule()->GetIccModule())
    pIccTransform = NULL;
  else
    pIccModule = CFX_GEModule::Get()->GetCodecModule()->GetIccModule();

  if (m_pDevice->GetBPP() == 8) {  // Gray(a) device
    ASSERT(!m_bRgbByteOrder);
    if (m_pDevice->IsAlphaMask()) {
      // Alpha Mask
      m_Gray = 255;
    } else {
      // Gray(a) device
      if (pIccTransform) {
        uint8_t gray;
        color = bObjectCMYK ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
        pIccModule->TranslateScanline(pIccTransform, &gray,
                                      (const uint8_t*)&color, 1);
        m_Gray = gray;
      } else {
        if (bObjectCMYK) {
          uint8_t r, g, b;
          AdobeCMYK_to_sRGB1(FXSYS_GetCValue(color), FXSYS_GetMValue(color),
                             FXSYS_GetYValue(color), FXSYS_GetKValue(color), r,
                             g, b);
          m_Gray = FXRGB2GRAY(r, g, b);
        } else {
          m_Gray =
              FXRGB2GRAY(FXARGB_R(color), FXARGB_G(color), FXARGB_B(color));
        }
      }
    }
  } else {
    if (bDeviceCMYK) {  // Cmyk(a) Device
      ASSERT(!m_bRgbByteOrder);
      // TODO... opt for cmyk
      composite_span = &CFX_SkiaRenderer::CompositeSpanCMYK;
      if (bObjectCMYK) {
        m_Color = FXCMYK_TODIB(color);
        if (pIccTransform)
          pIccModule->TranslateScanline(pIccTransform, (uint8_t*)&m_Color,
                                        (const uint8_t*)&m_Color, 1);
      } else {  // Object RGB
        if (!pIccTransform)
          return FALSE;
        color = FXARGB_TODIB(color);
        pIccModule->TranslateScanline(pIccTransform, (uint8_t*)&m_Color,
                                      (const uint8_t*)&color, 1);
      }
      m_Red = ((uint8_t*)&m_Color)[0];
      m_Green = ((uint8_t*)&m_Color)[1];
      m_Blue = ((uint8_t*)&m_Color)[2];
      m_Gray = ((uint8_t*)&m_Color)[3];
      return TRUE;
    }
    if (pIccTransform) {
      color = bObjectCMYK ? FXCMYK_TODIB(color) : FXARGB_TODIB(color);
      pIccModule->TranslateScanline(pIccTransform, (uint8_t*)&m_Color,
                                    (const uint8_t*)&color, 1);
      ((uint8_t*)&m_Color)[3] = m_Alpha;
      m_Red = ((uint8_t*)&m_Color)[2];
      m_Green = ((uint8_t*)&m_Color)[1];
      m_Blue = ((uint8_t*)&m_Color)[0];
      // Need Johnson to improvement it.
      if (m_bRgbByteOrder) {
        // swap
        m_Red = ((uint8_t*)&m_Color)[0];
        m_Blue = ((uint8_t*)&m_Color)[2];
        m_Color = FXARGB_TODIB(m_Color);
        m_Color = FXARGB_TOBGRORDERDIB(m_Color);
      }
    } else {
      if (bObjectCMYK) {
        uint8_t r, g, b;
        AdobeCMYK_to_sRGB1(FXSYS_GetCValue(color), FXSYS_GetMValue(color),
                           FXSYS_GetYValue(color), FXSYS_GetKValue(color), r, g,
                           b);
        m_Color = FXARGB_MAKE(m_Alpha, r, g, b);
        if (m_bRgbByteOrder) {
          m_Color = FXARGB_TOBGRORDERDIB(m_Color);
          m_Red = b;
          m_Green = g;
          m_Blue = r;  //
        } else {
          m_Color = FXARGB_TODIB(m_Color);
          m_Red = r;
          m_Green = g;
          m_Blue = b;  //
        }
      } else {
        if (m_bRgbByteOrder) {
          m_Color = FXARGB_TOBGRORDERDIB(color);
          ArgbDecode(color, m_Alpha, m_Blue, m_Green, m_Red);  //
        } else {
          m_Color = FXARGB_TODIB(color);
          ArgbDecode(color, m_Alpha, m_Red, m_Green, m_Blue);
        }
      }
    }
  }
  // Get palette transparency selector
  m_ProcessFilter =
      (m_pOriDevice ? 1 : 0)               /* has Ori Device flag */
      + (m_pDevice->GetBPP() >= 8 ? 2 : 0) /* bpp flag */
      + (m_pClipMask ? 4 : 0)              /* has clip region flag */
      + (m_pDevice->m_pAlphaMask ? 8 : 0); /* has Alpha Mask chanel flag */
  switch (m_ProcessFilter) {
    case 0:
      composite_span = &CFX_SkiaRenderer::CompositeSpan1bpp_0;
      break;
    case 2: {
      if (m_pDevice->GetBPP() == 8)
        composite_span = &CFX_SkiaRenderer::CompositeSpanGray_2;
      else if (m_pDevice->GetBPP() == 24)
        composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_2;
      else
        composite_span = m_pDevice->HasAlpha()
                             ? &CFX_SkiaRenderer::CompositeSpanARGB_2
                             : &CFX_SkiaRenderer::CompositeSpanRGB32_2;
    } break;
    case 3: {
      if (m_pDevice->GetBPP() == 8)
        composite_span = &CFX_SkiaRenderer::CompositeSpanGray_3;
      else if (m_pDevice->GetBPP() == 24)
        composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_3;
      else
        composite_span = m_pDevice->HasAlpha()
                             ? &CFX_SkiaRenderer::CompositeSpanARGB_3
                             : &CFX_SkiaRenderer::CompositeSpanRGB32_3;
    } break;
    case 4:
      composite_span = &CFX_SkiaRenderer::CompositeSpan1bpp_4;
      break;
    case 6: {
      if (m_pDevice->GetBPP() == 8)
        composite_span = &CFX_SkiaRenderer::CompositeSpanGray_6;
      else if (m_pDevice->GetBPP() == 24)
        composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_6;
      else
        composite_span = m_pDevice->HasAlpha()
                             ? &CFX_SkiaRenderer::CompositeSpanARGB_6
                             : &CFX_SkiaRenderer::CompositeSpanRGB32_6;
    } break;
    case 7: {
      if (m_pDevice->GetBPP() == 8)
        composite_span = &CFX_SkiaRenderer::CompositeSpanGray_7;
      else if (m_pDevice->GetBPP() == 24)
        composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_7;
      else
        composite_span = m_pDevice->HasAlpha()
                             ? &CFX_SkiaRenderer::CompositeSpanARGB_7
                             : &CFX_SkiaRenderer::CompositeSpanRGB32_7;
    } break;
    case 1:
    case 5:
    case 8:
    case 9:
    case 11:
    case 12:
    case 13:
    case 15:
      // TODO...
      break;
    case 10:
      composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_10;
      break;
    case 14:
      composite_span = &CFX_SkiaRenderer::CompositeSpanRGB24_14;
      break;
  }
  return !!composite_span;
}

/*----------------------------------------------------------------------------------------------------*/
void CFX_SkiaA8Renderer::blitAntiH(int x,
                                   int y,
                                   const SkAlpha antialias[],
                                   const int16_t runs[]) {
  FXSYS_assert(m_pDevice);
  int dst_y = y - m_Top;
  if (dst_y < 0 || dst_y >= m_pDevice->GetHeight())
    return;

  uint8_t* dest_scan = m_pDevice->GetBuffer() + m_pDevice->GetPitch() * dst_y;
  uint8_t* dest_pos = dest_scan;
  while (1) {
    if (x >= m_dstWidth)
      return;
    int width = runs[0];
    SkASSERT(width >= 0);
    if (width <= 0)
      return;
    unsigned aa = antialias[0];
    if (aa) {
      int col_start = x < m_Left ? 0 : x - m_Left;
      int col_end = x + width;
      col_end = col_end < m_dstWidth ? col_end - m_Left : m_pDevice->GetWidth();
      int result = col_end - col_start;
      if (result > 0) {
        dest_pos = dest_scan + col_start;
        if (result >= 4)
          FXSYS_memset(dest_pos, FXARGB_MAKE(aa, aa, aa, aa), result);
        else
          FXSYS_memset(dest_pos, aa, result);
      }
    }
    runs += width;
    antialias += width;
    x += width;
  }
}
void CFX_SkiaA8Renderer::blitH(int x, int y, int width) {
  FXSYS_assert(m_pDevice);
  int dst_y = y - m_Top;
  if (dst_y < 0 || dst_y >= m_pDevice->GetHeight())
    return;
  if (x >= m_dstWidth)
    return;
  uint8_t* dest_scan = m_pDevice->GetBuffer() + m_pDevice->GetPitch() * dst_y;
  int col_start = x < m_Left ? 0 : x - m_Left;
  int col_end = x + width;
  col_end = col_end < m_dstWidth ? col_end - m_Left : m_pDevice->GetWidth();
  int result = col_end - col_start;
  if (result > 0) {
    uint8_t* dest_pos = dest_scan + col_start;
    if (result >= 4)
      FXSYS_memset(dest_pos, 0xffffffff, result);
    else
      FXSYS_memset(dest_pos, 255, result);
  }
}
void CFX_SkiaA8Renderer::blitV(int x, int y, int height, SkAlpha alpha) {
  FXSYS_assert(alpha);
  if (alpha == 255) {
    blitRect(x, y, 1, height);
  } else {
    int16_t runs[2];
    runs[0] = 1;
    runs[1] = 0;
    while (--height >= 0) {
      if (y >= m_dstHeight)
        return;
      blitAntiH(x, y++, &alpha, runs);
    }
  }
}
void CFX_SkiaA8Renderer::blitRect(int x, int y, int width, int height) {
  FXSYS_assert(m_pDevice);
  while (--height >= 0) {
    if (y >= m_dstHeight)
      return;
    blitH(x, y++, width);
  }
}

void CFX_SkiaA8Renderer::blitAntiRect(int x,
                                      int y,
                                      int width,
                                      int height,
                                      SkAlpha leftAlpha,
                                      SkAlpha rightAlpha) {
  blitV(x++, y, height, leftAlpha);
  if (width > 0) {
    blitRect(x, y, width, height);
    x += width;
  }
  blitV(x, y, height, rightAlpha);
}

FX_BOOL CFX_SkiaA8Renderer::Init(CFX_DIBitmap* pDevice, int Left, int Top) {
  m_pDevice = pDevice;
  m_Left = Left;
  m_Top = Top;
  if (pDevice) {
    m_dstWidth = m_Left + pDevice->GetWidth();
    m_dstHeight = m_Top + pDevice->GetHeight();
  }
  return TRUE;
}
#endif
