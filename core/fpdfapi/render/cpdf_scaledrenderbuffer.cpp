// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_scaledrenderbuffer.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/render/cpdf_devicebuffer.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"

namespace {

constexpr size_t kImageSizeLimitBytes = 30 * 1024 * 1024;

}  // namespace

CPDF_ScaledRenderBuffer::CPDF_ScaledRenderBuffer() = default;

CPDF_ScaledRenderBuffer::~CPDF_ScaledRenderBuffer() = default;

bool CPDF_ScaledRenderBuffer::Initialize(CPDF_RenderContext* pContext,
                                         CFX_RenderDevice* pDevice,
                                         const FX_RECT& rect,
                                         const CPDF_PageObject* pObj,
                                         const CPDF_RenderOptions* pOptions,
                                         int max_dpi) {
  device_ = pDevice;
  if (device_->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    return true;
  }

  rect_ = rect;
  matrix_ = CPDF_DeviceBuffer::CalculateMatrix(pDevice, rect, max_dpi,
                                               /*scale=*/true);
  bitmap_device_ = std::make_unique<CFX_DefaultRenderDevice>();
  bool bIsAlpha =
      !!(device_->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_ALPHA_OUTPUT);
  FXDIB_Format dibFormat = bIsAlpha ? FXDIB_Format::kArgb : FXDIB_Format::kRgb;
  while (true) {
    FX_RECT bitmap_rect =
        matrix_.TransformRect(CFX_FloatRect(rect)).GetOuterRect();
    int32_t width = bitmap_rect.Width();
    int32_t height = bitmap_rect.Height();
    // Set to 0 to make CalculatePitchAndSize() calculate it.
    constexpr uint32_t kNoPitch = 0;
    std::optional<CFX_DIBitmap::PitchAndSize> pitch_size =
        CFX_DIBitmap::CalculatePitchAndSize(width, height, dibFormat, kNoPitch);
    if (!pitch_size.has_value())
      return false;

    if (pitch_size.value().size <= kImageSizeLimitBytes &&
        bitmap_device_->Create(width, height, dibFormat)) {
      break;
    }
    matrix_.Scale(0.5f, 0.5f);
  }
  pContext->GetBackgroundToDevice(bitmap_device_.get(), pObj, pOptions,
                                  matrix_);
  return true;
}

CFX_RenderDevice* CPDF_ScaledRenderBuffer::GetDevice() const {
  return bitmap_device_ ? static_cast<CFX_RenderDevice*>(bitmap_device_.get())
                        : device_.get();
}

void CPDF_ScaledRenderBuffer::OutputToDevice() {
  if (bitmap_device_) {
#if defined(PDF_USE_SKIA)
    bitmap_device_->SyncInternalBitmaps();
#endif
    device_->StretchDIBits(bitmap_device_->GetBitmap(), rect_.left, rect_.top,
                           rect_.Width(), rect_.Height());
  }
}
