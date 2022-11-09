// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cgdi_display_driver.h"

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/dib/cfx_dibextractor.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/render_defines.h"
#include "core/fxge/win32/cwin32_platform.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

CGdiDisplayDriver::CGdiDisplayDriver(HDC hDC)
    : CGdiDeviceDriver(hDC, DeviceType::kDisplay) {
  auto* pPlatform =
      static_cast<CWin32Platform*>(CFX_GEModule::Get()->GetPlatform());
  if (pPlatform->m_GdiplusExt.IsAvailable()) {
    m_RenderCaps |= FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE;
  }
}

CGdiDisplayDriver::~CGdiDisplayDriver() = default;

int CGdiDisplayDriver::GetDeviceCaps(int caps_id) const {
  if (caps_id == FXDC_HORZ_SIZE || caps_id == FXDC_VERT_SIZE)
    return 0;
  return CGdiDeviceDriver::GetDeviceCaps(caps_id);
}

bool CGdiDisplayDriver::GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                                  int left,
                                  int top) {
  bool ret = false;
  int width = pBitmap->GetWidth();
  int height = pBitmap->GetHeight();
  HBITMAP hbmp = CreateCompatibleBitmap(m_hDC, width, height);
  HDC hDCMemory = CreateCompatibleDC(m_hDC);
  HBITMAP holdbmp = (HBITMAP)SelectObject(hDCMemory, hbmp);
  BitBlt(hDCMemory, 0, 0, width, height, m_hDC, left, top, SRCCOPY);
  SelectObject(hDCMemory, holdbmp);
  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof bmi);
  bmi.bmiHeader.biSize = sizeof bmi.bmiHeader;
  bmi.bmiHeader.biBitCount = pBitmap->GetBPP();
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biWidth = width;
  if (pBitmap->GetBPP() > 8) {
    ret = ::GetDIBits(hDCMemory, hbmp, 0, height, pBitmap->GetBuffer().data(),
                      &bmi, DIB_RGB_COLORS) == height;
  } else {
    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (bitmap->Create(width, height, FXDIB_Format::kRgb)) {
      bmi.bmiHeader.biBitCount = 24;
      ::GetDIBits(hDCMemory, hbmp, 0, height, bitmap->GetBuffer().data(), &bmi,
                  DIB_RGB_COLORS);
      ret = pBitmap->TransferBitmap(0, 0, width, height, bitmap, 0, 0);
    } else {
      ret = false;
    }
  }
  if (ret && pBitmap->IsAlphaFormat())
    pBitmap->SetUniformOpaqueAlpha();

  DeleteObject(hbmp);
  DeleteObject(hDCMemory);
  return ret;
}

bool CGdiDisplayDriver::SetDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                  uint32_t color,
                                  const FX_RECT& src_rect,
                                  int left,
                                  int top,
                                  BlendMode blend_type) {
  DCHECK_EQ(blend_type, BlendMode::kNormal);
  if (pSource->IsMaskFormat()) {
    int width = pSource->GetWidth(), height = pSource->GetHeight();
    int alpha = FXARGB_A(color);
    if (pSource->GetBPP() != 1 || alpha != 255) {
      auto background = pdfium::MakeRetain<CFX_DIBitmap>();
      if (!background->Create(width, height, FXDIB_Format::kRgb32) ||
          !GetDIBits(background, left, top) ||
          !background->CompositeMask(0, 0, width, height, pSource, color, 0, 0,
                                     BlendMode::kNormal, nullptr, false)) {
        return false;
      }
      FX_RECT alpha_src_rect(0, 0, width, height);
      return SetDIBits(background, 0, alpha_src_rect, left, top,
                       BlendMode::kNormal);
    }
    FX_RECT clip_rect(left, top, left + src_rect.Width(),
                      top + src_rect.Height());
    return StretchDIBits(pSource, color, left - src_rect.left,
                         top - src_rect.top, width, height, &clip_rect,
                         FXDIB_ResampleOptions(), BlendMode::kNormal);
  }
  int width = src_rect.Width();
  int height = src_rect.Height();
  if (pSource->IsAlphaFormat()) {
    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!bitmap->Create(width, height, FXDIB_Format::kRgb) ||
        !GetDIBits(bitmap, left, top) ||
        !bitmap->CompositeBitmap(0, 0, width, height, pSource, src_rect.left,
                                 src_rect.top, BlendMode::kNormal, nullptr,
                                 false)) {
      return false;
    }
    FX_RECT alpha_src_rect(0, 0, width, height);
    return SetDIBits(bitmap, 0, alpha_src_rect, left, top, BlendMode::kNormal);
  }
  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;
  return GDI_SetDIBits(pBitmap, src_rect, left, top);
}

bool CGdiDisplayDriver::UseFoxitStretchEngine(
    const RetainPtr<CFX_DIBBase>& pSource,
    uint32_t color,
    int dest_left,
    int dest_top,
    int dest_width,
    int dest_height,
    const FX_RECT* pClipRect,
    const FXDIB_ResampleOptions& options) {
  FX_RECT bitmap_clip = *pClipRect;
  if (dest_width < 0)
    dest_left += dest_width;

  if (dest_height < 0)
    dest_top += dest_height;

  bitmap_clip.Offset(-dest_left, -dest_top);
  RetainPtr<CFX_DIBitmap> pStretched =
      pSource->StretchTo(dest_width, dest_height, options, &bitmap_clip);
  if (!pStretched)
    return true;

  FX_RECT src_rect(0, 0, pStretched->GetWidth(), pStretched->GetHeight());
  return SetDIBits(pStretched, color, src_rect, pClipRect->left, pClipRect->top,
                   BlendMode::kNormal);
}

bool CGdiDisplayDriver::StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                                      uint32_t color,
                                      int dest_left,
                                      int dest_top,
                                      int dest_width,
                                      int dest_height,
                                      const FX_RECT* pClipRect,
                                      const FXDIB_ResampleOptions& options,
                                      BlendMode blend_type) {
  DCHECK(pSource);
  DCHECK(pClipRect);

  if (options.HasAnyOptions() || dest_width > 10000 || dest_width < -10000 ||
      dest_height > 10000 || dest_height < -10000) {
    return UseFoxitStretchEngine(pSource, color, dest_left, dest_top,
                                 dest_width, dest_height, pClipRect, options);
  }
  if (pSource->IsMaskFormat()) {
    FX_RECT image_rect;
    image_rect.left = dest_width > 0 ? dest_left : dest_left + dest_width;
    image_rect.right = dest_width > 0 ? dest_left + dest_width : dest_left;
    image_rect.top = dest_height > 0 ? dest_top : dest_top + dest_height;
    image_rect.bottom = dest_height > 0 ? dest_top + dest_height : dest_top;
    FX_RECT clip_rect = image_rect;
    clip_rect.Intersect(*pClipRect);
    clip_rect.Offset(-image_rect.left, -image_rect.top);
    int clip_width = clip_rect.Width(), clip_height = clip_rect.Height();
    RetainPtr<CFX_DIBitmap> pStretched(pSource->StretchTo(
        dest_width, dest_height, FXDIB_ResampleOptions(), &clip_rect));
    if (!pStretched)
      return true;

    auto background = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!background->Create(clip_width, clip_height, FXDIB_Format::kRgb32) ||
        !GetDIBits(background, image_rect.left + clip_rect.left,
                   image_rect.top + clip_rect.top) ||
        !background->CompositeMask(0, 0, clip_width, clip_height, pStretched,
                                   color, 0, 0, BlendMode::kNormal, nullptr,
                                   false)) {
      return false;
    }

    FX_RECT src_rect(0, 0, clip_width, clip_height);
    return SetDIBits(background, 0, src_rect, image_rect.left + clip_rect.left,
                     image_rect.top + clip_rect.top, BlendMode::kNormal);
  }
  if (pSource->IsAlphaFormat()) {
    auto* pPlatform =
        static_cast<CWin32Platform*>(CFX_GEModule::Get()->GetPlatform());
    if (pPlatform->m_GdiplusExt.IsAvailable()) {
      CFX_DIBExtractor temp(pSource);
      RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
      if (!pBitmap)
        return false;
      return pPlatform->m_GdiplusExt.StretchDIBits(
          m_hDC, pBitmap, dest_left, dest_top, dest_width, dest_height,
          pClipRect, FXDIB_ResampleOptions());
    }
    return UseFoxitStretchEngine(pSource, color, dest_left, dest_top,
                                 dest_width, dest_height, pClipRect,
                                 FXDIB_ResampleOptions());
  }
  CFX_DIBExtractor temp(pSource);
  RetainPtr<CFX_DIBitmap> pBitmap = temp.GetBitmap();
  if (!pBitmap)
    return false;
  return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width,
                           dest_height, FXDIB_ResampleOptions());
}

bool CGdiDisplayDriver::StartDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                                    int bitmap_alpha,
                                    uint32_t color,
                                    const CFX_Matrix& matrix,
                                    const FXDIB_ResampleOptions& options,
                                    std::unique_ptr<CFX_ImageRenderer>* handle,
                                    BlendMode blend_type) {
  return false;
}
