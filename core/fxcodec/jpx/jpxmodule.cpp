// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jpx/jpxmodule.h"

#include "third_party/base/ptr_util.h"

namespace fxcodec {

// static
std::unique_ptr<CJPX_Decoder> JpxModule::CreateDecoder(
    pdfium::span<const uint8_t> src_span,
    CJPX_Decoder::ColorSpaceOption option) {
  auto decoder = pdfium::MakeUnique<CJPX_Decoder>(option);
  if (!decoder->Init(src_span))
    return nullptr;

  return decoder;
}

}  // namespace fxcodec
