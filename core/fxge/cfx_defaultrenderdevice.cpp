// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_defaultrenderdevice.h"

#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"

namespace {

// When build variant is Skia then it is assumed as the default, but might be
// overridden at runtime.
#if defined(_SKIA_SUPPORT_)
CFX_DefaultRenderDevice::RendererType g_renderer_type =
    CFX_DefaultRenderDevice::kDefaultRenderer;
#endif

}  // namespace

// static
bool CFX_DefaultRenderDevice::UseSkiaRenderer() {
#if defined(_SKIA_SUPPORT_)
  return g_renderer_type == RendererType::kSkia;
#else
  return false;
#endif
}

#if defined(_SKIA_SUPPORT_)
// static
void CFX_DefaultRenderDevice::SetRendererType(RendererType renderer_type) {
  g_renderer_type = renderer_type;
}
#endif

CFX_DefaultRenderDevice::CFX_DefaultRenderDevice() = default;

CFX_DefaultRenderDevice::~CFX_DefaultRenderDevice() = default;

bool CFX_DefaultRenderDevice::Attach(RetainPtr<CFX_DIBitmap> pBitmap) {
  return AttachWithRgbByteOrder(std::move(pBitmap), false);
}

bool CFX_DefaultRenderDevice::AttachWithRgbByteOrder(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder) {
  return AttachImpl(std::move(pBitmap), bRgbByteOrder, nullptr, false);
}

bool CFX_DefaultRenderDevice::AttachWithBackdropAndGroupKnockout(
    RetainPtr<CFX_DIBitmap> pBitmap,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
  return AttachImpl(std::move(pBitmap), false, std::move(pBackdropBitmap),
                    bGroupKnockout);
}

bool CFX_DefaultRenderDevice::CFX_DefaultRenderDevice::AttachImpl(
    RetainPtr<CFX_DIBitmap> pBitmap,
    bool bRgbByteOrder,
    RetainPtr<CFX_DIBitmap> pBackdropBitmap,
    bool bGroupKnockout) {
#if defined(_SKIA_SUPPORT_)
  if (UseSkiaRenderer()) {
    return AttachSkiaImpl(std::move(pBitmap), bRgbByteOrder,
                          std::move(pBackdropBitmap), bGroupKnockout);
  }
#endif
  return AttachAggImpl(std::move(pBitmap), bRgbByteOrder,
                       std::move(pBackdropBitmap), bGroupKnockout);
}

bool CFX_DefaultRenderDevice::Create(int width,
                                     int height,
                                     FXDIB_Format format,
                                     RetainPtr<CFX_DIBitmap> pBackdropBitmap) {
#if defined(_SKIA_SUPPORT_)
  if (UseSkiaRenderer()) {
    return CreateSkia(width, height, format, pBackdropBitmap);
  }
#endif
  return CreateAgg(width, height, format, pBackdropBitmap);
}
