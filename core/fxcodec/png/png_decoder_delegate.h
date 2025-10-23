// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
#define CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_

#include <stdint.h>

#include "core/fxcrt/span.h"

namespace fxcodec {

// Abstract interface used by the `libpng`-based decoder from `png_decoder.h`
// (in the future by Skia-based decoder from `skia_png_decoder.h`).
//
// TODO(https://crbug.com/444045690): Update the comment above once the
// Skia-based decoder is implemented and/or the `libpng`-based decoder moved
// to renamed .h/.cc files.
class PngDecoderDelegate {
 public:
  // Called by `PngDecoder` after decoding the image header:
  //
  // * `width` and `height` specify image dimensions in pixels
  //
  // Implementation should:
  //
  // * Return `true` upon success (and `false` otherwise)
  // * Set `*gamma` to the target gamma to decode with
  // * TODO(crbug.com/355630556): Add out parameter for desired alpha-premul.
  virtual bool PngReadHeader(int width,
                             int height,
                             int pass,
                             double* gamma) = 0;

  // Called by `PngDecoder` to ask where to write decoded BGRA/8 pixels.
  // Implementation should return a span to the buffer where the
  // decoded pixels should be written to.
  //
  // `PngDecoder` guarantees that `0 <= line && line < height`
  // (`height` that was earlier passed to `PngReadHeader`).
  virtual pdfium::span<uint8_t> PngAskScanlineBuf(int line) = 0;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
