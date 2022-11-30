// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_DEFAULTRENDERDEVICE_H_
#define CORE_FXGE_CFX_DEFAULTRENDERDEVICE_H_

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/fx_dib.h"

class SkPictureRecorder;
struct SkRect;

class CFX_DefaultRenderDevice final : public CFX_RenderDevice {
 public:
  CFX_DefaultRenderDevice();
  ~CFX_DefaultRenderDevice() override;

  bool Attach(RetainPtr<CFX_DIBitmap> pBitmap);
  bool AttachWithRgbByteOrder(RetainPtr<CFX_DIBitmap> pBitmap,
                              bool bRgbByteOrder);
  bool AttachWithBackdropAndGroupKnockout(
      RetainPtr<CFX_DIBitmap> pBitmap,
      RetainPtr<CFX_DIBitmap> pBackdropBitmap,
      bool bGroupKnockout);
  bool Create(int width,
              int height,
              FXDIB_Format format,
              RetainPtr<CFX_DIBitmap> pBackdropBitmap);

#if defined(_SKIA_SUPPORT_)
  bool AttachRecorder(SkPictureRecorder* recorder);
  void Clear(uint32_t color);
  std::unique_ptr<SkPictureRecorder> CreateRecorder(const SkRect& bounds);
  void DebugVerifyBitmapIsPreMultiplied() const override;
  bool SetBitsWithMask(const RetainPtr<CFX_DIBBase>& pBitmap,
                       const RetainPtr<CFX_DIBBase>& pMask,
                       int left,
                       int top,
                       int bitmap_alpha,
                       BlendMode blend_type) override;
#endif

  // Runtime check to see if Skia is the renderer variant in use.
  static bool SkiaIsDefaultRenderer();

#if defined(_SKIA_SUPPORT_)
  // This internal definition of renderer types must stay updated with respect
  // to the public definition of `FPDF_RENDERER_TYPE`, so that all public
  // definition values can be mapped to a value in
  // `CFX_DefaultRenderDevice::RendererType`.
  enum class RendererType {
    kAgg = 0,
    kSkia = 1,
  };

  // Update default renderer.
  static void SetDefaultRenderer(RendererType renderer_type);
#endif  // defined(_SKIA_SUPPORT_)

 private:
  bool AttachImpl(RetainPtr<CFX_DIBitmap> pBitmap,
                  bool bRgbByteOrder,
                  RetainPtr<CFX_DIBitmap> pBackdropBitmap,
                  bool bGroupKnockout);

  bool AttachAggImpl(RetainPtr<CFX_DIBitmap> pBitmap,
                     bool bRgbByteOrder,
                     RetainPtr<CFX_DIBitmap> pBackdropBitmap,
                     bool bGroupKnockout);

  bool CreateAgg(int width,
                 int height,
                 FXDIB_Format format,
                 RetainPtr<CFX_DIBitmap> pBackdropBitmap);

#ifdef _SKIA_SUPPORT_
  bool AttachSkiaImpl(RetainPtr<CFX_DIBitmap> pBitmap,
                      bool bRgbByteOrder,
                      RetainPtr<CFX_DIBitmap> pBackdropBitmap,
                      bool bGroupKnockout);

  bool CreateSkia(int width,
                  int height,
                  FXDIB_Format format,
                  RetainPtr<CFX_DIBitmap> pBackdropBitmap);
#endif
};

#endif  // CORE_FXGE_CFX_DEFAULTRENDERDEVICE_H_
