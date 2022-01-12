// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_AGG_FX_AGG_DRIVER_H_
#define CORE_FXGE_AGG_FX_AGG_DRIVER_H_

#include <memory>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/renderdevicedriver_iface.h"

class CFX_ClipRgn;
class CFX_GraphStateData;
class CFX_Matrix;
class CFX_Path;

namespace pdfium {

namespace agg {
class rasterizer_scanline_aa;
}  // namespace agg

class CFX_AggDeviceDriver final : public RenderDeviceDriverIface {
 public:
  CFX_AggDeviceDriver(const RetainPtr<CFX_DIBitmap>& pBitmap,
                      bool bRgbByteOrder,
                      const RetainPtr<CFX_DIBitmap>& pBackdropBitmap,
                      bool bGroupKnockout);
  ~CFX_AggDeviceDriver() override;

  void InitPlatform();
  void DestroyPlatform();

  // RenderDeviceDriverIface:
  DeviceType GetDeviceType() const override;
  int GetDeviceCaps(int caps_id) const override;
  void SaveState() override;
  void RestoreState(bool bKeepSaved) override;
  bool SetClip_PathFill(const CFX_Path& path,
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
  bool FillRectWithBlend(const FX_RECT& rect,
                         uint32_t fill_color,
                         BlendMode blend_type) override;
  bool GetClipBox(FX_RECT* pRect) override;
  bool GetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap,
                 int left,
                 int top) override;
  RetainPtr<CFX_DIBitmap> GetBackDrop() override;
  bool SetDIBits(const RetainPtr<CFX_DIBBase>& pBitmap,
                 uint32_t argb,
                 const FX_RECT& src_rect,
                 int left,
                 int top,
                 BlendMode blend_type) override;
  bool StretchDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                     uint32_t argb,
                     int dest_left,
                     int dest_top,
                     int dest_width,
                     int dest_height,
                     const FX_RECT* pClipRect,
                     const FXDIB_ResampleOptions& options,
                     BlendMode blend_type) override;
  bool StartDIBits(const RetainPtr<CFX_DIBBase>& pSource,
                   int bitmap_alpha,
                   uint32_t argb,
                   const CFX_Matrix& matrix,
                   const FXDIB_ResampleOptions& options,
                   std::unique_ptr<CFX_ImageRenderer>* handle,
                   BlendMode blend_type) override;
  bool ContinueDIBits(CFX_ImageRenderer* handle,
                      PauseIndicatorIface* pPause) override;
  bool DrawDeviceText(int nChars,
                      const TextCharPos* pCharPos,
                      CFX_Font* pFont,
                      const CFX_Matrix& mtObject2Device,
                      float font_size,
                      uint32_t color,
                      const CFX_TextRenderOptions& options) override;
  int GetDriverType() const override;

 private:
  void RenderRasterizer(pdfium::agg::rasterizer_scanline_aa& rasterizer,
                        uint32_t color,
                        bool bFullCover,
                        bool bGroupKnockout);

  void SetClipMask(pdfium::agg::rasterizer_scanline_aa& rasterizer);

  RetainPtr<CFX_DIBitmap> const m_pBitmap;
  std::unique_ptr<CFX_ClipRgn> m_pClipRgn;
  std::vector<std::unique_ptr<CFX_ClipRgn>> m_StateStack;
#if BUILDFLAG(IS_APPLE)
  void* m_pPlatformGraphics = nullptr;
#endif
  CFX_FillRenderOptions m_FillOptions;
  const bool m_bRgbByteOrder;
  const bool m_bGroupKnockout;
  RetainPtr<CFX_DIBitmap> m_pBackdropBitmap;
};

}  // namespace pdfium

#endif  // CORE_FXGE_AGG_FX_AGG_DRIVER_H_
