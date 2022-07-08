// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/cfx_defaultrenderdevice.h"

#include "core/fxge/dib/cfx_dibitmap.h"

bool CFX_DefaultRenderDevice::Attach(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  return AttachWithRgbByteOrder(pBitmap, false);
}

bool CFX_DefaultRenderDevice::AttachWithRgbByteOrder(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    bool bRgbByteOrder) {
  return AttachImpl(pBitmap, bRgbByteOrder, nullptr, false);
}

bool CFX_DefaultRenderDevice::AttachWithBackdropAndGroupKnockout(
    const RetainPtr<CFX_DIBitmap>& pBitmap,
    const RetainPtr<CFX_DIBitmap>& pBackdropBitmap,
    bool bGroupKnockout) {
  return AttachImpl(pBitmap, false, pBackdropBitmap, bGroupKnockout);
}
