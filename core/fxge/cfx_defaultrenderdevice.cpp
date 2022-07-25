// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_defaultrenderdevice.h"

#include <utility>

#include "core/fxge/dib/cfx_dibitmap.h"

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
