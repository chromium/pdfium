// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/fx_codec.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/fx_dib.h"

namespace fxcodec {
namespace {

FX_SAFE_UINT32 CalculatePitch8Safely(uint32_t bpc,
                                     uint32_t components,
                                     int width) {
  FX_SAFE_UINT32 pitch = bpc;
  pitch *= components;
  pitch *= width;
  pitch += 7;
  pitch /= 8;
  return pitch;
}

FX_SAFE_UINT32 CalculatePitch32Safely(int bpp, int width) {
  FX_SAFE_UINT32 pitch = bpp;
  pitch *= width;
  pitch += 31;
  pitch /= 32;  // quantized to number of 32-bit words.
  pitch *= 4;   // and then back to bytes, (not just /8 in one step).
  return pitch;
}

}  // namespace

#ifdef PDF_ENABLE_XFA
CFX_DIBAttribute::CFX_DIBAttribute() = default;

CFX_DIBAttribute::~CFX_DIBAttribute() {
  for (const auto& pair : m_Exif)
    FX_Free(pair.second);
}
#endif  // PDF_ENABLE_XFA

void ReverseRGB(uint8_t* pDestBuf, const uint8_t* pSrcBuf, int pixels) {
  if (pDestBuf == pSrcBuf) {
    for (int i = 0; i < pixels; i++) {
      std::swap(pDestBuf[0], pDestBuf[2]);
      pDestBuf += 3;
    }
  } else {
    for (int i = 0; i < pixels; i++) {
      ReverseCopy3Bytes(pDestBuf, pSrcBuf);
      pDestBuf += 3;
      pSrcBuf += 3;
    }
  }
}

uint32_t CalculatePitch8OrDie(uint32_t bpc, uint32_t components, int width) {
  return CalculatePitch8Safely(bpc, components, width).ValueOrDie();
}

uint32_t CalculatePitch32OrDie(int bpp, int width) {
  return CalculatePitch32Safely(bpp, width).ValueOrDie();
}

Optional<uint32_t> CalculatePitch8(uint32_t bpc,
                                   uint32_t components,
                                   int width) {
  FX_SAFE_UINT32 pitch = CalculatePitch8Safely(bpc, components, width);
  if (!pitch.IsValid())
    return pdfium::nullopt;
  return pitch.ValueOrDie();
}

Optional<uint32_t> CalculatePitch32(int bpp, int width) {
  FX_SAFE_UINT32 pitch = CalculatePitch32Safely(bpp, width);
  if (!pitch.IsValid())
    return pdfium::nullopt;
  return pitch.ValueOrDie();
}

}  // namespace fxcodec
