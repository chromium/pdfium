// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_jpxmodule.h"

#include "core/fxcodec/codec/cjpx_decoder.h"
#include "third_party/base/ptr_util.h"

// static
std::unique_ptr<CJPX_Decoder> CCodec_JpxModule::CreateDecoder(
    pdfium::span<const uint8_t> src_span,
    const RetainPtr<CPDF_ColorSpace>& cs) {
  auto decoder = pdfium::MakeUnique<CJPX_Decoder>(cs);
  if (!decoder->Init(src_span))
    return nullptr;

  return decoder;
}
