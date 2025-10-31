// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
#define CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_

#include <stdint.h>

#include "core/fxcrt/span.h"

namespace fxcodec {

// Abstract interface used by `libpng_png_decoder.h` and `skia_png_decoder.h`.
class PngDecoderDelegate {
 public:
  // Called by `PngDecoder` after decoding the image header with
  // `width` and `height` that specify image dimensions in pixels
  //
  // Implementation should:
  //
  // * Return `true` upon success (and `false` otherwise)
  // * Set `*gamma` to the target gamma to decode with
  // * TODO(crbug.com/355630556): Add out parameter for desired alpha-premul.
  virtual bool PngReadHeader(int width,
                             int height,
                             double* gamma) = 0;

  // Called by `PngDecoder` to ask where to write decoded BGRA/8 pixels.
  // Implementation should return a span to the buffer where decoder can write
  // decoded pixels from the given `line`.
  //
  // `PngDecoder` guarantees that `0 <= line && line < height`
  // (`height` that was earlier passed to `PngReadHeader`).
  virtual pdfium::span<uint8_t> PngAskScanlineBuf(int line) = 0;

  // Called by `PngDecoder` to ask where to write decoded BGRA/8 pixels.
  // Implementation should return a span to the buffer where decoder can write
  // all the decoded pixels (i.e. covering all rows/lines of the image).
  virtual pdfium::span<uint8_t> PngAskImageBuf() = 0;

  // Called by `PngDecoder` to communicate that all pixels have been decoded.
  virtual void PngFinishedDecoding() = 0;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
