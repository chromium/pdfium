// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_devicebuffer.h"

#include <utility>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

#if BUILDFLAG(IS_WIN)
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#else
#include "core/fxcrt/notreached.h"
#endif

namespace {

#if BUILDFLAG(IS_WIN)
constexpr bool kScaleDeviceBuffer = true;
#else
constexpr bool kScaleDeviceBuffer = false;
#endif

}  // namespace

// static
CFX_Matrix CPDF_DeviceBuffer::CalculateMatrix(CFX_RenderDevice* pDevice,
                                              const FX_RECT& rect,
                                              int max_dpi,
                                              bool scale) {
  CFX_Matrix matrix;
  matrix.Translate(-rect.left, -rect.top);
  if (scale) {
    int horz_size = pDevice->GetDeviceCaps(FXDC_HORZ_SIZE);
    int vert_size = pDevice->GetDeviceCaps(FXDC_VERT_SIZE);
    if (horz_size && vert_size && max_dpi) {
      int dpih =
          pDevice->GetDeviceCaps(FXDC_PIXEL_WIDTH) * 254 / (horz_size * 10);
      int dpiv =
          pDevice->GetDeviceCaps(FXDC_PIXEL_HEIGHT) * 254 / (vert_size * 10);
      if (dpih > max_dpi) {
        matrix.Scale(static_cast<float>(max_dpi) / dpih, 1.0f);
      }
      if (dpiv > max_dpi) {
        matrix.Scale(1.0f, static_cast<float>(max_dpi) / dpiv);
      }
    }
  }
  return matrix;
}

CPDF_DeviceBuffer::CPDF_DeviceBuffer(CPDF_RenderContext* context,
                                     CFX_RenderDevice* pDevice,
                                     const FX_RECT& rect,
                                     const CPDF_PageObject* pObj,
                                     int max_dpi)
    : device_(pDevice),
#if BUILDFLAG(IS_WIN)
      context_(context),
#endif
      object_(pObj),
      bitmap_(pdfium::MakeRetain<CFX_DIBitmap>()),
      rect_(rect),
      matrix_(CalculateMatrix(pDevice, rect, max_dpi, kScaleDeviceBuffer)) {
}

CPDF_DeviceBuffer::~CPDF_DeviceBuffer() = default;

RetainPtr<CFX_DIBitmap> CPDF_DeviceBuffer::Initialize() {
  FX_RECT bitmap_rect =
      matrix_.TransformRect(CFX_FloatRect(rect_)).GetOuterRect();
  // TODO(crbug.com/355630557): Consider adding support for
  // `FXDIB_Format::kBgraPremul`
  if (!bitmap_->Create(bitmap_rect.Width(), bitmap_rect.Height(),
                       FXDIB_Format::kBgra)) {
    return nullptr;
  }
  return bitmap_;
}

void CPDF_DeviceBuffer::OutputToDevice() {
  if (device_->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    if (matrix_.a == 1.0f && matrix_.d == 1.0f) {
      device_->SetDIBits(bitmap_, rect_.left, rect_.top);
      return;
    }

#if BUILDFLAG(IS_WIN)
    device_->StretchDIBits(bitmap_, rect_.left, rect_.top, rect_.Width(),
                           rect_.Height());
    return;
#else
    NOTREACHED();
#endif
  }

#if BUILDFLAG(IS_WIN)
  auto buffer = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!device_->CreateCompatibleBitmap(buffer, bitmap_->GetWidth(),
                                       bitmap_->GetHeight())) {
    return;
  }
  context_->GetBackgroundToBitmap(buffer, object_, matrix_);
  buffer->CompositeBitmap(0, 0, buffer->GetWidth(), buffer->GetHeight(),
                          bitmap_, 0, 0, BlendMode::kNormal, nullptr, false);
  device_->StretchDIBits(std::move(buffer), rect_.left, rect_.top,
                         rect_.Width(), rect_.Height());
#else
  NOTREACHED();
#endif
}
