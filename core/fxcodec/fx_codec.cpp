// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/fx_codec.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "core/fxcodec/jbig2/jbig2module.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

namespace {

ModuleMgr* g_ModuleMgr = nullptr;

}  // namespace

// static
void ModuleMgr::Create() {
  ASSERT(!g_ModuleMgr);
  g_ModuleMgr = new ModuleMgr();
}

// static
void ModuleMgr::Destroy() {
  ASSERT(g_ModuleMgr);
  delete g_ModuleMgr;
  g_ModuleMgr = nullptr;
}

// static
ModuleMgr* ModuleMgr::GetInstance() {
  ASSERT(g_ModuleMgr);
  return g_ModuleMgr;
}

ModuleMgr::ModuleMgr()
    : m_pJpegModule(pdfium::MakeUnique<JpegModule>()),
      m_pJbig2Module(pdfium::MakeUnique<Jbig2Module>()) {
#ifdef PDF_ENABLE_XFA_BMP
  SetBmpModule(pdfium::MakeUnique<BmpModule>());
#endif

#ifdef PDF_ENABLE_XFA_GIF
  SetGifModule(pdfium::MakeUnique<GifModule>());
#endif

#ifdef PDF_ENABLE_XFA_PNG
  SetPngModule(pdfium::MakeUnique<PngModule>());
#endif

#ifdef PDF_ENABLE_XFA_TIFF
  SetTiffModule(pdfium::MakeUnique<TiffModule>());
#endif
}

ModuleMgr::~ModuleMgr() = default;

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
      uint8_t temp = pDestBuf[2];
      pDestBuf[2] = pDestBuf[0];
      pDestBuf[0] = temp;
      pDestBuf += 3;
    }
  } else {
    for (int i = 0; i < pixels; i++) {
      *pDestBuf++ = pSrcBuf[2];
      *pDestBuf++ = pSrcBuf[1];
      *pDestBuf++ = pSrcBuf[0];
      pSrcBuf += 3;
    }
  }
}

FX_SAFE_UINT32 CalculatePitch8(uint32_t bpc, uint32_t components, int width) {
  FX_SAFE_UINT32 pitch = bpc;
  pitch *= components;
  pitch *= width;
  pitch += 7;
  pitch /= 8;
  return pitch;
}

FX_SAFE_UINT32 CalculatePitch32(int bpp, int width) {
  FX_SAFE_UINT32 pitch = bpp;
  pitch *= width;
  pitch += 31;
  pitch /= 32;  // quantized to number of 32-bit words.
  pitch *= 4;   // and then back to bytes, (not just /8 in one step).
  return pitch;
}

}  // namespace fxcodec
