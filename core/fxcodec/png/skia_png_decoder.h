// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_PNG_SKIA_PNG_DECODER_H_
#define CORE_FXCODEC_PNG_SKIA_PNG_DECODER_H_

#include <memory>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/retain_ptr.h"

#ifndef PDF_ENABLE_XFA_PNG
#error "PNG must be enabled"
#endif

#ifndef PDF_USE_SKIA
#error "Skia must be enabled"
#endif

namespace fxcodec {

class PngDecoderDelegate;
class ProgressiveDecoderContext;

// PNG decoder that uses the Skia library to decode pixels.  (Whether
// `SkPngDecoder` vs `SkPngRustDecoder` is used depends on whether the build
// configuration sets certain macro definitions like `PDF_ENABLE_RUST_PNG`).
class SkiaPngDecoder {
 public:
  static std::unique_ptr<ProgressiveDecoderContext> StartDecode(
      PngDecoderDelegate* delegate);

  static bool ContinueDecode(ProgressiveDecoderContext* context,
                             RetainPtr<CFX_CodecMemory> codec_memory);

  SkiaPngDecoder() = delete;
  SkiaPngDecoder(const SkiaPngDecoder&) = delete;
  SkiaPngDecoder& operator=(const SkiaPngDecoder&) = delete;
};

}  // namespace fxcodec

using PngDecoder = fxcodec::SkiaPngDecoder;

#endif  // CORE_FXCODEC_PNG_SKIA_PNG_DECODER_H_
