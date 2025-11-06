// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_PNG_LIBPNG_PNG_DECODER_H_
#define CORE_FXCODEC_PNG_LIBPNG_PNG_DECODER_H_

#include <memory>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/retain_ptr.h"

#ifndef PDF_ENABLE_XFA_PNG
#error "PNG must be enabled"
#endif

namespace fxcodec {

class PngDecoderDelegate;
class ProgressiveDecoderContext;

// PNG decoder that uses the `libpng` library to decode pixels.
class LibpngPngDecoder {
 public:
  static std::unique_ptr<ProgressiveDecoderContext> StartDecode(
      PngDecoderDelegate* pDelegate);

  static bool ContinueDecode(ProgressiveDecoderContext* context,
                             RetainPtr<CFX_CodecMemory> codec_memory);

  LibpngPngDecoder() = delete;
  LibpngPngDecoder(const LibpngPngDecoder&) = delete;
  LibpngPngDecoder& operator=(const LibpngPngDecoder&) = delete;
};

}  // namespace fxcodec

using PngDecoder = fxcodec::LibpngPngDecoder;

#endif  // CORE_FXCODEC_PNG_LIBPNG_PNG_DECODER_H_
