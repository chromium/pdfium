// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/fx_codec.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include "core/fxcodec/codec/ccodec_iccmodule.h"
#include "core/fxcodec/codec/ccodec_jbig2module.h"
#include "core/fxcodec/codec/ccodec_jpegmodule.h"
#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"

CCodec_ModuleMgr::CCodec_ModuleMgr()
    : m_pJpegModule(pdfium::MakeUnique<CCodec_JpegModule>()),
      m_pJbig2Module(pdfium::MakeUnique<CCodec_Jbig2Module>()),
      m_pIccModule(pdfium::MakeUnique<CCodec_IccModule>()) {}

CCodec_ModuleMgr::~CCodec_ModuleMgr() = default;

#ifdef PDF_ENABLE_XFA
CFX_DIBAttribute::CFX_DIBAttribute() = default;

CFX_DIBAttribute::~CFX_DIBAttribute() {
  for (const auto& pair : m_Exif)
    FX_Free(pair.second);
}
#endif  // PDF_ENABLE_XFA

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
