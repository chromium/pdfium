// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cgdi_printer_driver.h"

#include <math.h>
#include <windows.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_windowsrenderdevice.h"
#include "core/fxge/dib/cfx_dibextractor.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/render_defines.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

#if defined(PDFIUM_PRINT_TEXT_WITH_GDI)
PDFiumEnsureTypefaceCharactersAccessible g_pdfium_typeface_accessible_func =
    nullptr;
#endif

CGdiPrinterDriver::CGdiPrinterDriver(HDC hDC)
    : CGdiDeviceDriver(hDC, DeviceType::kPrinter),
      m_HorzSize(::GetDeviceCaps(m_hDC, HORZSIZE)),
      m_VertSize(::GetDeviceCaps(m_hDC, VERTSIZE)) {}

CGdiPrinterDriver::~CGdiPrinterDriver() = default;

int CGdiPrinterDriver::GetDeviceCaps(int caps_id) const {
  if (caps_id == FXDC_HORZ_SIZE)
    return m_HorzSize;
  if (caps_id == FXDC_VERT_SIZE)
    return m_VertSize;
  return CGdiDeviceDriver::GetDeviceCaps(caps_id);
}

bool CGdiPrinterDriver::SetDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                  uint32_t color,
                                  const FX_RECT& src_rect,
                                  int left,
                                  int top,
                                  BlendMode blend_type) {
  if (pSource->IsMaskFormat()) {
    FX_RECT clip_rect(left, top, left + src_rect.Width(),
                      top + src_rect.Height());
    return StretchDIBits(pSource, color, left - src_rect.left,
                         top - src_rect.top, pSource->GetWidth(),
                         pSource->GetHeight(), &clip_rect,
                         FXDIB_ResampleOptions(), BlendMode::kNormal);
  }
  DCHECK(pSource);
  DCHECK(!pSource->IsMaskFormat());
  DCHECK_EQ(blend_type, BlendMode::kNormal);
  if (pSource->IsAlphaFormat())
    return false;

  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;

  return GDI_SetDIBits(pBitmap, src_rect, left, top);
}

bool CGdiPrinterDriver::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                      uint32_t color,
                                      int dest_left,
                                      int dest_top,
                                      int dest_width,
                                      int dest_height,
                                      const FX_RECT* pClipRect,
                                      const FXDIB_ResampleOptions& options,
                                      BlendMode blend_type) {
  if (pSource->IsMaskFormat()) {
    int alpha = FXARGB_A(color);
    if (pSource->GetBPP() != 1 || alpha != 255)
      return false;

    if (dest_width < 0 || dest_height < 0) {
      RetainPtr<CFX_DIBitmap> pFlipped =
          pSource->FlipImage(dest_width < 0, dest_height < 0);
      if (!pFlipped)
        return false;

      if (dest_width < 0)
        dest_left += dest_width;
      if (dest_height < 0)
        dest_top += dest_height;

      return GDI_StretchBitMask(pFlipped, dest_left, dest_top, abs(dest_width),
                                abs(dest_height), color);
    }

    CFX_DIBExtractor temp(pSource);
    RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
    if (!pBitmap)
      return false;
    return GDI_StretchBitMask(pBitmap, dest_left, dest_top, dest_width,
                              dest_height, color);
  }

  if (pSource->IsAlphaFormat())
    return false;

  if (dest_width < 0 || dest_height < 0) {
    RetainPtr<CFX_DIBitmap> pFlipped =
        pSource->FlipImage(dest_width < 0, dest_height < 0);
    if (!pFlipped)
      return false;

    if (dest_width < 0)
      dest_left += dest_width;
    if (dest_height < 0)
      dest_top += dest_height;

    return GDI_StretchDIBits(pFlipped, dest_left, dest_top, abs(dest_width),
                             abs(dest_height), options);
  }

  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;
  return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width,
                           dest_height, options);
}

bool CGdiPrinterDriver::StartDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                    int bitmap_alpha,
                                    uint32_t color,
                                    const CFX_Matrix& matrix,
                                    const FXDIB_ResampleOptions& options,
                                    std::unique_ptr<CFX_ImageRenderer>* handle,
                                    BlendMode blend_type) {
  if (bitmap_alpha < 255 || pSource->IsAlphaFormat() ||
      (pSource->IsMaskFormat() && (pSource->GetBPP() != 1))) {
    return false;
  }
  CFX_FloatRect unit_rect = matrix.GetUnitRect();
  FX_RECT full_rect = unit_rect.GetOuterRect();
  if (fabs(matrix.b) < 0.5f && matrix.a != 0 && fabs(matrix.c) < 0.5f &&
      matrix.d != 0) {
    bool bFlipX = matrix.a < 0;
    bool bFlipY = matrix.d > 0;
    return StretchDIBits(pSource, color,
                         bFlipX ? full_rect.right : full_rect.left,
                         bFlipY ? full_rect.bottom : full_rect.top,
                         bFlipX ? -full_rect.Width() : full_rect.Width(),
                         bFlipY ? -full_rect.Height() : full_rect.Height(),
                         nullptr, FXDIB_ResampleOptions(), blend_type);
  }
  if (fabs(matrix.a) >= 0.5f || fabs(matrix.d) >= 0.5f)
    return false;

  RetainPtr<CFX_DIBitmap> pTransformed =
      pSource->SwapXY(matrix.c > 0, matrix.b < 0);
  if (!pTransformed)
    return false;

  return StretchDIBits(pTransformed, color, full_rect.left, full_rect.top,
                       full_rect.Width(), full_rect.Height(), nullptr,
                       FXDIB_ResampleOptions(), blend_type);
}

bool CGdiPrinterDriver::DrawDeviceText(int nChars,
                                       const TextCharPos* pCharPos,
                                       CFX_Font* pFont,
                                       const CFX_Matrix& mtObject2Device,
                                       float font_size,
                                       uint32_t color,
                                       const CFX_TextRenderOptions& options) {
  return false;
}
