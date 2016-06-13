// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/include/fx_ge.h"

#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_DESKTOP_

#include <windows.h>

#include "core/fxge/dib/dib_int.h"
#include "core/fxge/ge/fx_text_int.h"
#include "core/fxge/include/fx_freetype.h"
#include "core/fxge/include/fx_ge_win32.h"
#include "core/fxge/win32/win32_int.h"

namespace {

CFX_DIBitmap* Transform1bppBitmap(const CFX_DIBSource* pSrc,
                                  const CFX_Matrix* pDestMatrix) {
  ASSERT(pSrc->GetFormat() == FXDIB_1bppRgb ||
         pSrc->GetFormat() == FXDIB_1bppMask ||
         pSrc->GetFormat() == FXDIB_1bppCmyk);
  CFX_DIBExtractor src_bitmap(pSrc);
  CFX_DIBitmap* pSrcBitmap = src_bitmap;
  if (!pSrcBitmap)
    return nullptr;

  int src_width = pSrcBitmap->GetWidth(), src_height = pSrcBitmap->GetHeight();
  uint8_t* src_buf = pSrcBitmap->GetBuffer();
  uint32_t src_pitch = pSrcBitmap->GetPitch();
  FX_FLOAT dest_area = pDestMatrix->GetUnitArea();
  FX_FLOAT area_scale = ((FX_FLOAT)(src_width * src_height)) / dest_area;
  FX_FLOAT size_scale = FXSYS_sqrt(area_scale);
  CFX_Matrix adjusted_matrix(*pDestMatrix);
  adjusted_matrix.Scale(size_scale, size_scale);
  CFX_FloatRect result_rect_f = adjusted_matrix.GetUnitRect();
  FX_RECT result_rect = result_rect_f.GetOutterRect();
  CFX_Matrix src2result;
  src2result.e = adjusted_matrix.c + adjusted_matrix.e;
  src2result.f = adjusted_matrix.d + adjusted_matrix.f;
  src2result.a = adjusted_matrix.a / pSrcBitmap->GetWidth();
  src2result.b = adjusted_matrix.b / pSrcBitmap->GetWidth();
  src2result.c = -adjusted_matrix.c / pSrcBitmap->GetHeight();
  src2result.d = -adjusted_matrix.d / pSrcBitmap->GetHeight();
  src2result.TranslateI(-result_rect.left, -result_rect.top);
  CFX_Matrix result2src;
  result2src.SetReverse(src2result);
  CPDF_FixedMatrix result2src_fix(result2src, 8);
  int result_width = result_rect.Width();
  int result_height = result_rect.Height();
  std::unique_ptr<CFX_DIBitmap> pTempBitmap(new CFX_DIBitmap);
  if (!pTempBitmap->Create(result_width, result_height, pSrc->GetFormat())) {
    if (pSrcBitmap != src_bitmap)
      delete pSrcBitmap;
    return nullptr;
  }

  pTempBitmap->CopyPalette(pSrc->GetPalette());
  uint8_t* dest_buf = pTempBitmap->GetBuffer();
  int dest_pitch = pTempBitmap->GetPitch();
  FXSYS_memset(dest_buf, pSrc->IsAlphaMask() ? 0 : 0xff,
               dest_pitch * result_height);
  if (pSrcBitmap->IsAlphaMask()) {
    for (int dest_y = 0; dest_y < result_height; dest_y++) {
      uint8_t* dest_scan = dest_buf + dest_y * dest_pitch;
      for (int dest_x = 0; dest_x < result_width; dest_x++) {
        int src_x, src_y;
        result2src_fix.Transform(dest_x, dest_y, src_x, src_y);
        if (src_x < 0 || src_x >= src_width || src_y < 0 ||
            src_y >= src_height) {
          continue;
        }
        if (!((src_buf + src_pitch * src_y)[src_x / 8] &
              (1 << (7 - src_x % 8)))) {
          continue;
        }
        dest_scan[dest_x / 8] |= 1 << (7 - dest_x % 8);
      }
    }
  } else {
    for (int dest_y = 0; dest_y < result_height; dest_y++) {
      uint8_t* dest_scan = dest_buf + dest_y * dest_pitch;
      for (int dest_x = 0; dest_x < result_width; dest_x++) {
        int src_x, src_y;
        result2src_fix.Transform(dest_x, dest_y, src_x, src_y);
        if (src_x < 0 || src_x >= src_width || src_y < 0 ||
            src_y >= src_height) {
          continue;
        }
        if ((src_buf + src_pitch * src_y)[src_x / 8] & (1 << (7 - src_x % 8))) {
          continue;
        }
        dest_scan[dest_x / 8] &= ~(1 << (7 - dest_x % 8));
      }
    }
  }
  if (pSrcBitmap != src_bitmap)
    delete pSrcBitmap;

  return pTempBitmap.release();
}

}  // namespace

CGdiPrinterDriver::CGdiPrinterDriver(HDC hDC)
    : CGdiDeviceDriver(hDC, FXDC_PRINTER),
      m_HorzSize(::GetDeviceCaps(m_hDC, HORZSIZE)),
      m_VertSize(::GetDeviceCaps(m_hDC, VERTSIZE)) {}

CGdiPrinterDriver::~CGdiPrinterDriver() {}

int CGdiPrinterDriver::GetDeviceCaps(int caps_id) {
  if (caps_id == FXDC_HORZ_SIZE)
    return m_HorzSize;
  if (caps_id == FXDC_VERT_SIZE)
    return m_VertSize;
  return CGdiDeviceDriver::GetDeviceCaps(caps_id);
}

FX_BOOL CGdiPrinterDriver::SetDIBits(const CFX_DIBSource* pSource,
                                     uint32_t color,
                                     const FX_RECT* pSrcRect,
                                     int left,
                                     int top,
                                     int blend_type) {
  if (pSource->IsAlphaMask()) {
    FX_RECT clip_rect(left, top, left + pSrcRect->Width(),
                      top + pSrcRect->Height());
    return StretchDIBits(pSource, color, left - pSrcRect->left,
                         top - pSrcRect->top, pSource->GetWidth(),
                         pSource->GetHeight(), &clip_rect, 0,
                         FXDIB_BLEND_NORMAL);
  }
  ASSERT(pSource && !pSource->IsAlphaMask() && pSrcRect);
  ASSERT(blend_type == FXDIB_BLEND_NORMAL);
  if (pSource->HasAlpha())
    return FALSE;

  CFX_DIBExtractor temp(pSource);
  CFX_DIBitmap* pBitmap = temp;
  if (!pBitmap)
    return FALSE;

  return GDI_SetDIBits(pBitmap, pSrcRect, left, top, nullptr);
}

FX_BOOL CGdiPrinterDriver::StretchDIBits(const CFX_DIBSource* pSource,
                                         uint32_t color,
                                         int dest_left,
                                         int dest_top,
                                         int dest_width,
                                         int dest_height,
                                         const FX_RECT* pClipRect,
                                         uint32_t flags,
                                         int blend_type) {
  if (pSource->IsAlphaMask()) {
    int alpha = FXARGB_A(color);
    if (pSource->GetBPP() != 1 || alpha != 255)
      return FALSE;

    if (dest_width < 0 || dest_height < 0) {
      std::unique_ptr<CFX_DIBitmap> pFlipped(
          pSource->FlipImage(dest_width < 0, dest_height < 0));
      if (!pFlipped)
        return FALSE;

      if (dest_width < 0)
        dest_left += dest_width;
      if (dest_height < 0)
        dest_top += dest_height;

      return GDI_StretchBitMask(pFlipped.get(), dest_left, dest_top,
                                abs(dest_width), abs(dest_height), color, flags,
                                0, nullptr);
    }

    CFX_DIBExtractor temp(pSource);
    CFX_DIBitmap* pBitmap = temp;
    if (!pBitmap)
      return FALSE;
    return GDI_StretchBitMask(pBitmap, dest_left, dest_top, dest_width,
                              dest_height, color, flags, 0, nullptr);
  }

  if (pSource->HasAlpha())
    return FALSE;

  if (dest_width < 0 || dest_height < 0) {
    std::unique_ptr<CFX_DIBitmap> pFlipped(
        pSource->FlipImage(dest_width < 0, dest_height < 0));
    if (!pFlipped)
      return FALSE;

    if (dest_width < 0)
      dest_left += dest_width;
    if (dest_height < 0)
      dest_top += dest_height;

    return GDI_StretchDIBits(pFlipped.get(), dest_left, dest_top,
                             abs(dest_width), abs(dest_height), flags, nullptr);
  }

  CFX_DIBExtractor temp(pSource);
  CFX_DIBitmap* pBitmap = temp;
  if (!pBitmap)
    return FALSE;
  return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width,
                           dest_height, flags, nullptr);
}

FX_BOOL CGdiPrinterDriver::StartDIBits(const CFX_DIBSource* pSource,
                                       int bitmap_alpha,
                                       uint32_t color,
                                       const CFX_Matrix* pMatrix,
                                       uint32_t render_flags,
                                       void*& handle,
                                       int blend_type) {
  if (bitmap_alpha < 255 || pSource->HasAlpha() ||
      (pSource->IsAlphaMask() && (pSource->GetBPP() != 1))) {
    return FALSE;
  }
  CFX_FloatRect unit_rect = pMatrix->GetUnitRect();
  FX_RECT full_rect = unit_rect.GetOutterRect();
  if (FXSYS_fabs(pMatrix->b) < 0.5f && pMatrix->a != 0 &&
      FXSYS_fabs(pMatrix->c) < 0.5f && pMatrix->d != 0) {
    FX_BOOL bFlipX = pMatrix->a < 0;
    FX_BOOL bFlipY = pMatrix->d > 0;
    return StretchDIBits(pSource, color,
                         bFlipX ? full_rect.right : full_rect.left,
                         bFlipY ? full_rect.bottom : full_rect.top,
                         bFlipX ? -full_rect.Width() : full_rect.Width(),
                         bFlipY ? -full_rect.Height() : full_rect.Height(),
                         nullptr, 0, blend_type);
  }
  if (FXSYS_fabs(pMatrix->a) < 0.5f && FXSYS_fabs(pMatrix->d) < 0.5f) {
    std::unique_ptr<CFX_DIBitmap> pTransformed(
        pSource->SwapXY(pMatrix->c > 0, pMatrix->b < 0));
    if (!pTransformed)
      return FALSE;

    return StretchDIBits(pTransformed.get(), color, full_rect.left,
                         full_rect.top, full_rect.Width(), full_rect.Height(),
                         nullptr, 0, blend_type);
  }
  return FALSE;
}

#endif
