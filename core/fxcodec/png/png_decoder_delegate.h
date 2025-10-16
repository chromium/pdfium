// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
#define CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_

#include <stdint.h>

namespace fxcodec {

// Abstract interface used by the `libpng`-based decoder from `png_decoder.h`
// (in the future by Skia-based decoder from `skia_png_decoder.h`).
//
// TODO(https://crbug.com/444045690): Update the comment above once the
// Skia-based decoder is implemented and/or the `libpng`-based decoder moved
// to renamed .h/.cc files.
class PngDecoderDelegate {
 public:
  // Color format to decode into.
  enum class DecodedColorType {
    kBgr,
    kBgra,
  };

  // Called by `PngDecoder` after decoding the image header:
  //
  // * `width` and `height` specify image dimensions in pixels
  // * `bits_per_component` is number of bits per component (e.g. per red,
  //   or per alpha)
  // * `components_count` describes the number of channels that
  //   the encoded pixels are composed of.  For example it would be 3
  //   for a PNG containing RGB pixels.
  //
  // Implementation should:
  //
  // * Return `true` upon success (and `false` otherwise)
  // * Set `*dst_color_type` to the color type to decode into
  // * Set `*gamma` to the target gamma to decode with
  // * TODO(crbug.com/355630556): Add out parameter for desired alpha-premul.
  virtual bool PngReadHeader(int width,
                             int height,
                             int bits_per_component,
                             int components_count,
                             int pass,
                             DecodedColorType* dst_color_type,
                             double* gamma) = 0;

  // Called by `PngDecoder` to ask where to write decoded pixels.
  // Implementation should return a pointer to the buffer where the
  // decoded pixels should be written to.
  //
  // `PngDecoder` guarantees that `0 <= line && line < height`
  // (`height` that was earlier passed to `PngReadHeader`).
  virtual uint8_t* PngAskScanlineBuf(int line) = 0;

  // Called by `PngDecoder` Communicates that `line`th row has been decoded
  // enough to be displayed.
  virtual void PngFillScanlineBufCompleted(int line) = 0;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_PNG_PNG_DECODER_DELEGATE_H_
