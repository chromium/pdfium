// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JPX_JPXMODULE_H_
#define CORE_FXCODEC_JPX_JPXMODULE_H_

#include <memory>
#include <vector>

#include "core/fxcodec/jpx/cjpx_decoder.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/span.h"

namespace fxcodec {

class JpxModule {
 public:
  static std::unique_ptr<CJPX_Decoder> CreateDecoder(
      pdfium::span<const uint8_t> src_span,
      CJPX_Decoder::ColorSpaceOption option);

  JpxModule() = delete;
  JpxModule(const JpxModule&) = delete;
  JpxModule& operator=(const JpxModule&) = delete;
};

}  // namespace fxcodec

using JpxModule = fxcodec::JpxModule;

#endif  // CORE_FXCODEC_JPX_JPXMODULE_H_
