// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_defaultrenderdevice.h"

#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"

// static
bool CFX_DefaultRenderDevice::SkiaIsDefaultRenderer() {
#if defined(_SKIA_SUPPORT_)
  // TODO(crbug.com/pdfium/1878) This will become variable-based once a method
  // is provided to set the default at runtime.
  return true;
#else
  return false;
#endif
}

// static
bool CFX_DefaultRenderDevice::SkiaPathsIsDefaultRenderer() {
#if defined(_SKIA_SUPPORT_PATHS_)
  // TODO(crbug.com/pdfium/1878) This will become variable-based once a method
  // is provided to set the default at runtime.
  return true;
#else
  return false;
#endif
}

CFX_DefaultRenderDevice::CFX_DefaultRenderDevice() = default;

CFX_DefaultRenderDevice::~CFX_DefaultRenderDevice() {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  Flush(true);
#endif
}

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
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  return AttachSkiaImpl(pBitmap, bRgbByteOrder, pBackdropBitmap,
                        bGroupKnockout);
#else
  return AttachAggImpl(pBitmap, bRgbByteOrder, pBackdropBitmap, bGroupKnockout);
#endif
}

bool CFX_DefaultRenderDevice::Create(int width,
                                     int height,
                                     FXDIB_Format format,
                                     RetainPtr<CFX_DIBitmap> pBackdropBitmap) {
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  return CreateSkia(width, height, format, pBackdropBitmap);
#else
  return CreateAgg(width, height, format, pBackdropBitmap);
#endif
}
