// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_PNG_PNG_DECODER_H_
#define CORE_FXCODEC_PNG_PNG_DECODER_H_

#include <memory>

#include "core/fxcodec/progressive_decoder_iface.h"
#include "core/fxcrt/retain_ptr.h"

#ifndef PDF_ENABLE_XFA_PNG
#error "PNG must be enabled"
#endif

namespace fxcodec {

class CFX_DIBAttribute;

class PngDecoder {
 public:
  class Delegate {
   public:
    // Color type specified in the PNG IHDR chunk.  See also
    // https://www.w3.org/TR/png-3/#6Colour-values and
    // https://www.w3.org/TR/png-3/#11IHDR
    //
    // The enumerator values below have been picked to match 1) the PNG spec and
    // 2) `PNG_COLOR_TYPE_...` constants from `libpng/png.h`.
    enum class EncodedColorType : int {
      kGrayscale = 0,
      kTruecolor = 2,
      kIndexedColor = 3,
      kGrayscaleWithAlpha = 4,
      kTruecolorWithAlpha = 6,
    };

    // Called by `PngDecoder` after decoding the image header:
    //
    // * `width` and `height` specify image dimensions in pixels
    // * `bpc` is number of bits per component (e.g. per red, or per alpha)
    // * `*color_type` is initially set to the color type the image has been
    //   encoded with
    //
    // Implementation should:
    //
    // * Return `true` upon success (and `false` otherwise)
    // * Set `*color_type` to the color type to decode into
    //   (currently only `kTruecolor` and `kTruecolorWithAlpha` are supported)
    // * Set `*gamma` to the target gamma to decode with
    // * TODO(crbug.com/355630556): Add out parameter for desired alpha-premul.
    virtual bool PngReadHeader(int width,
                               int height,
                               int bpc,
                               int pass,
                               EncodedColorType* color_type,
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

  static std::unique_ptr<ProgressiveDecoderIface::Context> StartDecode(
      Delegate* pDelegate);

  static bool ContinueDecode(ProgressiveDecoderIface::Context* pContext,
                             RetainPtr<CFX_CodecMemory> codec_memory,
                             CFX_DIBAttribute* pAttribute);

  PngDecoder() = delete;
  PngDecoder(const PngDecoder&) = delete;
  PngDecoder& operator=(const PngDecoder&) = delete;
};

}  // namespace fxcodec

using PngDecoder = fxcodec::PngDecoder;

#endif  // CORE_FXCODEC_PNG_PNG_DECODER_H_
