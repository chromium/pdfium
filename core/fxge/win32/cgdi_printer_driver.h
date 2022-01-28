// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CGDI_PRINTER_DRIVER_H_
#define CORE_FXGE_WIN32_CGDI_PRINTER_DRIVER_H_

#include <windows.h>

#include <memory>

#include "core/fxge/win32/cgdi_device_driver.h"

class CGdiPrinterDriver final : public CGdiDeviceDriver {
 public:
  explicit CGdiPrinterDriver(HDC hDC);
  ~CGdiPrinterDriver() override;

 private:
  // CGdiPrinterDriver:
  int GetDeviceCaps(int caps_id) const override;
  bool SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                 uint32_t color,
                 const FX_RECT& src_rect,
                 int left,
                 int top,
                 BlendMode blend_type) override;
  bool StretchDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                     uint32_t color,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options,
                     BlendMode blend_type) override;
  bool StartDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                   int bitmap_alpha,
                   uint32_t color,
                   const CFX_Matrix& matrix,
                   const FXDIB_ResampleOptions& options,
                   std::unique_ptr<CFX_ImageRenderer>* handle,
                   BlendMode blend_type) override;
  bool DrawDeviceText(pdfium::span<const TextCharPos> pCharPos,
                      CFX_Font* pFont,
                      const CFX_Matrix& mtObject2Device,
                      float font_size,
                      uint32_t color,
                      const CFX_TextRenderOptions& options) override;

  const int m_HorzSize;
  const int m_VertSize;
};

#endif  // CORE_FXGE_WIN32_CGDI_PRINTER_DRIVER_H_
