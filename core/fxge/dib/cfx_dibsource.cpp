// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/dib/cfx_dibsource.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/cfx_imagetransformer.h"
#include "core/fxge/ge/cfx_cliprgn.h"
#include "third_party/base/ptr_util.h"

CFX_DIBSource::CFX_DIBSource()
    : m_Width(0), m_Height(0), m_bpp(0), m_AlphaFlag(0), m_Pitch(0) {}

CFX_DIBSource::~CFX_DIBSource() {}

uint8_t* CFX_DIBSource::GetBuffer() const {
  return nullptr;
}

bool CFX_DIBSource::SkipToScanline(int line, IFX_Pause* pPause) const {
  return false;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::Clone(const FX_RECT* pClip) const {
  FX_RECT rect(0, 0, m_Width, m_Height);
  if (pClip) {
    rect.Intersect(*pClip);
    if (rect.IsEmpty())
      return nullptr;
  }
  auto pNewBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pNewBitmap->Create(rect.Width(), rect.Height(), GetFormat()))
    return nullptr;

  pNewBitmap->SetPalette(m_pPalette.get());
  pNewBitmap->SetAlphaMask(m_pAlphaMask, pClip);
  if (GetBPP() == 1 && rect.left % 8 != 0) {
    int left_shift = rect.left % 32;
    int right_shift = 32 - left_shift;
    int dword_count = pNewBitmap->m_Pitch / 4;
    for (int row = rect.top; row < rect.bottom; row++) {
      uint32_t* src_scan = (uint32_t*)GetScanline(row) + rect.left / 32;
      uint32_t* dest_scan = (uint32_t*)pNewBitmap->GetScanline(row - rect.top);
      for (int i = 0; i < dword_count; i++) {
        dest_scan[i] =
            (src_scan[i] << left_shift) | (src_scan[i + 1] >> right_shift);
      }
    }
  } else {
    int copy_len = (pNewBitmap->GetWidth() * pNewBitmap->GetBPP() + 7) / 8;
    if (m_Pitch < (uint32_t)copy_len)
      copy_len = m_Pitch;

    for (int row = rect.top; row < rect.bottom; row++) {
      const uint8_t* src_scan = GetScanline(row) + rect.left * m_bpp / 8;
      uint8_t* dest_scan = (uint8_t*)pNewBitmap->GetScanline(row - rect.top);
      FXSYS_memcpy(dest_scan, src_scan, copy_len);
    }
  }
  return pNewBitmap;
}

void CFX_DIBSource::BuildPalette() {
  if (m_pPalette)
    return;

  if (GetBPP() == 1) {
    m_pPalette.reset(FX_Alloc(uint32_t, 2));
    if (IsCmykImage()) {
      m_pPalette.get()[0] = 0xff;
      m_pPalette.get()[1] = 0;
    } else {
      m_pPalette.get()[0] = 0xff000000;
      m_pPalette.get()[1] = 0xffffffff;
    }
  } else if (GetBPP() == 8) {
    m_pPalette.reset(FX_Alloc(uint32_t, 256));
    if (IsCmykImage()) {
      for (int i = 0; i < 256; i++)
        m_pPalette.get()[i] = 0xff - i;
    } else {
      for (int i = 0; i < 256; i++)
        m_pPalette.get()[i] = 0xff000000 | (i * 0x10101);
    }
  }
}

bool CFX_DIBSource::BuildAlphaMask() {
  if (m_pAlphaMask)
    return true;

  m_pAlphaMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!m_pAlphaMask->Create(m_Width, m_Height, FXDIB_8bppMask)) {
    m_pAlphaMask = nullptr;
    return false;
  }
  FXSYS_memset(m_pAlphaMask->GetBuffer(), 0xff,
               m_pAlphaMask->GetHeight() * m_pAlphaMask->GetPitch());
  return true;
}

uint32_t CFX_DIBSource::GetPaletteEntry(int index) const {
  ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
  if (m_pPalette) {
    return m_pPalette.get()[index];
  }
  if (IsCmykImage()) {
    if (GetBPP() == 1) {
      return index ? 0 : 0xff;
    }
    return 0xff - index;
  }
  if (GetBPP() == 1) {
    return index ? 0xffffffff : 0xff000000;
  }
  return index * 0x10101 | 0xff000000;
}

void CFX_DIBSource::SetPaletteEntry(int index, uint32_t color) {
  ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
  if (!m_pPalette) {
    BuildPalette();
  }
  m_pPalette.get()[index] = color;
}

int CFX_DIBSource::FindPalette(uint32_t color) const {
  ASSERT((GetBPP() == 1 || GetBPP() == 8) && !IsAlphaMask());
  if (!m_pPalette) {
    if (IsCmykImage()) {
      if (GetBPP() == 1) {
        return ((uint8_t)color == 0xff) ? 0 : 1;
      }
      return 0xff - (uint8_t)color;
    }
    if (GetBPP() == 1) {
      return ((uint8_t)color == 0xff) ? 1 : 0;
    }
    return (uint8_t)color;
  }
  int palsize = (1 << GetBPP());
  for (int i = 0; i < palsize; i++)
    if (m_pPalette.get()[i] == color) {
      return i;
    }
  return -1;
}

void CFX_DIBSource::GetOverlapRect(int& dest_left,
                                   int& dest_top,
                                   int& width,
                                   int& height,
                                   int src_width,
                                   int src_height,
                                   int& src_left,
                                   int& src_top,
                                   const CFX_ClipRgn* pClipRgn) {
  if (width == 0 || height == 0) {
    return;
  }
  ASSERT(width > 0 && height > 0);
  if (dest_left > m_Width || dest_top > m_Height) {
    width = 0;
    height = 0;
    return;
  }
  int x_offset = dest_left - src_left;
  int y_offset = dest_top - src_top;
  FX_RECT src_rect(src_left, src_top, src_left + width, src_top + height);
  FX_RECT src_bound(0, 0, src_width, src_height);
  src_rect.Intersect(src_bound);
  FX_RECT dest_rect(src_rect.left + x_offset, src_rect.top + y_offset,
                    src_rect.right + x_offset, src_rect.bottom + y_offset);
  FX_RECT dest_bound(0, 0, m_Width, m_Height);
  dest_rect.Intersect(dest_bound);
  if (pClipRgn) {
    dest_rect.Intersect(pClipRgn->GetBox());
  }
  dest_left = dest_rect.left;
  dest_top = dest_rect.top;
  src_left = dest_left - x_offset;
  src_top = dest_top - y_offset;
  width = dest_rect.right - dest_rect.left;
  height = dest_rect.bottom - dest_rect.top;
}

void CFX_DIBSource::SetPalette(const uint32_t* pSrc) {
  static const uint32_t kPaletteSize = 256;
  if (!pSrc || GetBPP() > 8) {
    m_pPalette.reset();
    return;
  }
  uint32_t pal_size = 1 << GetBPP();
  if (!m_pPalette)
    m_pPalette.reset(FX_Alloc(uint32_t, pal_size));
  pal_size = std::min(pal_size, kPaletteSize);
  FXSYS_memcpy(m_pPalette.get(), pSrc, pal_size * sizeof(uint32_t));
}

void CFX_DIBSource::GetPalette(uint32_t* pal, int alpha) const {
  ASSERT(GetBPP() <= 8 && !IsCmykImage());
  if (GetBPP() == 1) {
    pal[0] = ((m_pPalette ? m_pPalette.get()[0] : 0xff000000) & 0xffffff) |
             (alpha << 24);
    pal[1] = ((m_pPalette ? m_pPalette.get()[1] : 0xffffffff) & 0xffffff) |
             (alpha << 24);
    return;
  }
  if (m_pPalette) {
    for (int i = 0; i < 256; i++) {
      pal[i] = (m_pPalette.get()[i] & 0x00ffffff) | (alpha << 24);
    }
  } else {
    for (int i = 0; i < 256; i++) {
      pal[i] = (i * 0x10101) | (alpha << 24);
    }
  }
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::CloneAlphaMask(
    const FX_RECT* pClip) const {
  ASSERT(GetFormat() == FXDIB_Argb);
  FX_RECT rect(0, 0, m_Width, m_Height);
  if (pClip) {
    rect.Intersect(*pClip);
    if (rect.IsEmpty())
      return nullptr;
  }
  auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pMask->Create(rect.Width(), rect.Height(), FXDIB_8bppMask))
    return nullptr;

  for (int row = rect.top; row < rect.bottom; row++) {
    const uint8_t* src_scan = GetScanline(row) + rect.left * 4 + 3;
    uint8_t* dest_scan =
        const_cast<uint8_t*>(pMask->GetScanline(row - rect.top));
    for (int col = rect.left; col < rect.right; col++) {
      *dest_scan++ = *src_scan;
      src_scan += 4;
    }
  }
  return pMask;
}

bool CFX_DIBSource::SetAlphaMask(const CFX_RetainPtr<CFX_DIBSource>& pAlphaMask,
                                 const FX_RECT* pClip) {
  if (!HasAlpha() || GetFormat() == FXDIB_Argb)
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
  for (int row = 0; row < m_Height; row++) {
    FXSYS_memcpy(const_cast<uint8_t*>(m_pAlphaMask->GetScanline(row)),
                 pAlphaMask->GetScanline(row + rect.top) + rect.left,
                 m_pAlphaMask->m_Pitch);
  }
  return true;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::FlipImage(bool bXFlip,
                                                     bool bYFlip) const {
  auto pFlipped = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pFlipped->Create(m_Width, m_Height, GetFormat()))
    return nullptr;

  pFlipped->SetPalette(m_pPalette.get());
  uint8_t* pDestBuffer = pFlipped->GetBuffer();
  int Bpp = m_bpp / 8;
  for (int row = 0; row < m_Height; row++) {
    const uint8_t* src_scan = GetScanline(row);
    uint8_t* dest_scan =
        pDestBuffer + m_Pitch * (bYFlip ? (m_Height - row - 1) : row);
    if (!bXFlip) {
      FXSYS_memcpy(dest_scan, src_scan, m_Pitch);
      continue;
    }
    if (m_bpp == 1) {
      FXSYS_memset(dest_scan, 0, m_Pitch);
      for (int col = 0; col < m_Width; col++)
        if (src_scan[col / 8] & (1 << (7 - col % 8))) {
          int dest_col = m_Width - col - 1;
          dest_scan[dest_col / 8] |= (1 << (7 - dest_col % 8));
        }
    } else {
      dest_scan += (m_Width - 1) * Bpp;
      if (Bpp == 1) {
        for (int col = 0; col < m_Width; col++) {
          *dest_scan = *src_scan;
          dest_scan--;
          src_scan++;
        }
      } else if (Bpp == 3) {
        for (int col = 0; col < m_Width; col++) {
          dest_scan[0] = src_scan[0];
          dest_scan[1] = src_scan[1];
          dest_scan[2] = src_scan[2];
          dest_scan -= 3;
          src_scan += 3;
        }
      } else {
        ASSERT(Bpp == 4);
        for (int col = 0; col < m_Width; col++) {
          *(uint32_t*)dest_scan = *(uint32_t*)src_scan;
          dest_scan -= 4;
          src_scan += 4;
        }
      }
    }
  }
  if (m_pAlphaMask) {
    pDestBuffer = pFlipped->m_pAlphaMask->GetBuffer();
    uint32_t dest_pitch = pFlipped->m_pAlphaMask->GetPitch();
    for (int row = 0; row < m_Height; row++) {
      const uint8_t* src_scan = m_pAlphaMask->GetScanline(row);
      uint8_t* dest_scan =
          pDestBuffer + dest_pitch * (bYFlip ? (m_Height - row - 1) : row);
      if (!bXFlip) {
        FXSYS_memcpy(dest_scan, src_scan, dest_pitch);
        continue;
      }
      dest_scan += (m_Width - 1);
      for (int col = 0; col < m_Width; col++) {
        *dest_scan = *src_scan;
        dest_scan--;
        src_scan++;
      }
    }
  }
  return pFlipped;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::CloneConvert(
    FXDIB_Format dest_format) {
  if (dest_format == GetFormat())
    return Clone(nullptr);

  auto pClone = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pClone->Create(m_Width, m_Height, dest_format))
    return nullptr;

  CFX_RetainPtr<CFX_DIBitmap> pSrcAlpha;
  if (HasAlpha()) {
    if (GetFormat() == FXDIB_Argb)
      pSrcAlpha = CloneAlphaMask();
    else
      pSrcAlpha = m_pAlphaMask;

    if (!pSrcAlpha)
      return nullptr;
  }
  bool ret = true;
  if (dest_format & 0x0200) {
    if (dest_format == FXDIB_Argb) {
      ret = pSrcAlpha ? pClone->LoadChannel(FXDIB_Alpha, pSrcAlpha, FXDIB_Alpha)
                      : pClone->LoadChannel(FXDIB_Alpha, 0xff);
    } else {
      ret = pClone->SetAlphaMask(pSrcAlpha);
    }
  }
  if (!ret)
    return nullptr;

  CFX_RetainPtr<CFX_DIBSource> holder(this);
  std::unique_ptr<uint32_t, FxFreeDeleter> pal_8bpp;
  if (!ConvertBuffer(dest_format, pClone->GetBuffer(), pClone->GetPitch(),
                     m_Width, m_Height, holder, 0, 0, &pal_8bpp)) {
    return nullptr;
  }
  if (pal_8bpp)
    pClone->SetPalette(pal_8bpp.get());

  return pClone;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::SwapXY(
    bool bXFlip,
    bool bYFlip,
    const FX_RECT* pDestClip) const {
  FX_RECT dest_clip(0, 0, m_Height, m_Width);
  if (pDestClip)
    dest_clip.Intersect(*pDestClip);
  if (dest_clip.IsEmpty())
    return nullptr;

  auto pTransBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  int result_height = dest_clip.Height();
  int result_width = dest_clip.Width();
  if (!pTransBitmap->Create(result_width, result_height, GetFormat()))
    return nullptr;

  pTransBitmap->SetPalette(m_pPalette.get());
  int dest_pitch = pTransBitmap->GetPitch();
  uint8_t* dest_buf = pTransBitmap->GetBuffer();
  int row_start = bXFlip ? m_Height - dest_clip.right : dest_clip.left;
  int row_end = bXFlip ? m_Height - dest_clip.left : dest_clip.right;
  int col_start = bYFlip ? m_Width - dest_clip.bottom : dest_clip.top;
  int col_end = bYFlip ? m_Width - dest_clip.top : dest_clip.bottom;
  if (GetBPP() == 1) {
    FXSYS_memset(dest_buf, 0xff, dest_pitch * result_height);
    for (int row = row_start; row < row_end; row++) {
      const uint8_t* src_scan = GetScanline(row);
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf;
      if (bYFlip) {
        dest_scan += (result_height - 1) * dest_pitch;
      }
      int dest_step = bYFlip ? -dest_pitch : dest_pitch;
      for (int col = col_start; col < col_end; col++) {
        if (!(src_scan[col / 8] & (1 << (7 - col % 8)))) {
          dest_scan[dest_col / 8] &= ~(1 << (7 - dest_col % 8));
        }
        dest_scan += dest_step;
      }
    }
  } else {
    int nBytes = GetBPP() / 8;
    int dest_step = bYFlip ? -dest_pitch : dest_pitch;
    if (nBytes == 3) {
      dest_step -= 2;
    }
    for (int row = row_start; row < row_end; row++) {
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf + dest_col * nBytes;
      if (bYFlip) {
        dest_scan += (result_height - 1) * dest_pitch;
      }
      if (nBytes == 4) {
        uint32_t* src_scan = (uint32_t*)GetScanline(row) + col_start;
        for (int col = col_start; col < col_end; col++) {
          *(uint32_t*)dest_scan = *src_scan++;
          dest_scan += dest_step;
        }
      } else {
        const uint8_t* src_scan = GetScanline(row) + col_start * nBytes;
        if (nBytes == 1) {
          for (int col = col_start; col < col_end; col++) {
            *dest_scan = *src_scan++;
            dest_scan += dest_step;
          }
        } else {
          for (int col = col_start; col < col_end; col++) {
            *dest_scan++ = *src_scan++;
            *dest_scan++ = *src_scan++;
            *dest_scan = *src_scan++;
            dest_scan += dest_step;
          }
        }
      }
    }
  }
  if (m_pAlphaMask) {
    dest_pitch = pTransBitmap->m_pAlphaMask->GetPitch();
    dest_buf = pTransBitmap->m_pAlphaMask->GetBuffer();
    int dest_step = bYFlip ? -dest_pitch : dest_pitch;
    for (int row = row_start; row < row_end; row++) {
      int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) -
                     dest_clip.left;
      uint8_t* dest_scan = dest_buf + dest_col;
      if (bYFlip) {
        dest_scan += (result_height - 1) * dest_pitch;
      }
      const uint8_t* src_scan = m_pAlphaMask->GetScanline(row) + col_start;
      for (int col = col_start; col < col_end; col++) {
        *dest_scan = *src_scan++;
        dest_scan += dest_step;
      }
    }
  }
  return pTransBitmap;
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::TransformTo(
    const CFX_Matrix* pDestMatrix,
    int& result_left,
    int& result_top,
    uint32_t flags,
    const FX_RECT* pDestClip) {
  CFX_RetainPtr<CFX_DIBSource> holder(this);
  CFX_ImageTransformer transformer(holder, pDestMatrix, flags, pDestClip);
  transformer.Start();
  transformer.Continue(nullptr);
  result_left = transformer.result().left;
  result_top = transformer.result().top;
  return transformer.DetachBitmap();
}

CFX_RetainPtr<CFX_DIBitmap> CFX_DIBSource::StretchTo(int dest_width,
                                                     int dest_height,
                                                     uint32_t flags,
                                                     const FX_RECT* pClip) {
  CFX_RetainPtr<CFX_DIBSource> holder(this);
  FX_RECT clip_rect(0, 0, FXSYS_abs(dest_width), FXSYS_abs(dest_height));
  if (pClip)
    clip_rect.Intersect(*pClip);

  if (clip_rect.IsEmpty())
    return nullptr;

  if (dest_width == m_Width && dest_height == m_Height)
    return Clone(&clip_rect);

  CFX_BitmapStorer storer;
  CFX_ImageStretcher stretcher(&storer, holder, dest_width, dest_height,
                               clip_rect, flags);
  if (stretcher.Start())
    stretcher.Continue(nullptr);

  return storer.Detach();
}
