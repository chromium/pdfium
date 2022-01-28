// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_WIN32_CPS_PRINTER_DRIVER_H_
#define CORE_FXGE_WIN32_CPS_PRINTER_DRIVER_H_

#include <windows.h>

#include <memory>

#include "core/fxge/cfx_windowsrenderdevice.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "core/fxge/win32/cfx_psrenderer.h"

class CFX_PSFontTracker;

class CPSPrinterDriver final : public RenderDeviceDriverIface {
 public:
  CPSPrinterDriver(HDC hDC,
                   WindowsPrintMode mode,
                   CFX_PSFontTracker* ps_font_tracker,
                   const EncoderIface* encoder_iface);
  ~CPSPrinterDriver() override;

 private:
  // RenderDeviceDriverIface:
  DeviceType GetDeviceType() const override;
  int GetDeviceCaps(int caps_id) const override;
  void SaveState() override;
  void RestoreState(bool bKeepSaved) override;
  bool SetClip_PathFill(const CFX_Path& paath,
                        const CFX_Matrix* pObject2Device,
                        const CFX_FillRenderOptions& fill_options) override;
  bool SetClip_PathStroke(const CFX_Path& path,
                          const CFX_Matrix* pObject2Device,
                          const CFX_GraphStateData* pGraphState) override;
  bool DrawPath(const CFX_Path& path,
                const CFX_Matrix* pObject2Device,
                const CFX_GraphStateData* pGraphState,
                uint32_t fill_color,
                uint32_t stroke_color,
                const CFX_FillRenderOptions& fill_options,
                BlendMode blend_type) override;
  bool GetClipBox(FX_RECT* pRect) override;
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

  HDC m_hDC;
  int m_Width;
  int m_Height;
  int m_nBitsPerPixel;
  int m_HorzSize;
  int m_VertSize;
  CFX_PSRenderer m_PSRenderer;
};

#endif  // CORE_FXGE_WIN32_CPS_PRINTER_DRIVER_H_
