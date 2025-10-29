// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_GIF_DECODER_H_
#define CORE_FXCODEC_GIF_GIF_DECODER_H_

#include <stdint.h>

#include <memory>
#include <utility>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_types.h"
#include "core/fxcrt/span.h"

#ifndef PDF_ENABLE_XFA_GIF
#error "GIF must be enabled"
#endif

namespace fxcodec {

class ProgressiveDecoderContext;

class GifDecoder {
 public:
  enum class Status {
    kError,
    kSuccess,
    kUnfinished,
  };

  class Delegate {
   public:
    virtual uint32_t GifCurrentPosition() const = 0;
    virtual bool GifInputRecordPositionBuf(uint32_t rcd_pos,
                                           const FX_RECT& img_rc,
                                           pdfium::span<CFX_GifPalette> pal_ptr,
                                           int32_t trans_index) = 0;
    virtual void GifReadScanline(int32_t row_num,
                                 pdfium::span<uint8_t> row_buf) = 0;
  };

  static std::unique_ptr<ProgressiveDecoderContext> StartDecode(
      Delegate* pDelegate);
  static Status ReadHeader(ProgressiveDecoderContext* context,
                           int* width,
                           int* height,
                           pdfium::span<CFX_GifPalette>* pal_pp,
                           int* bg_index);
  static std::pair<Status, size_t> LoadFrameInfo(
      ProgressiveDecoderContext* context);
  static Status LoadFrame(ProgressiveDecoderContext* context, size_t frame_num);
  static FX_FILESIZE GetAvailInput(ProgressiveDecoderContext* context);
  static bool Input(ProgressiveDecoderContext* context,
                    RetainPtr<CFX_CodecMemory> codec_memory);

  // Only `static` methods.
  GifDecoder() = delete;
  GifDecoder(const GifDecoder&) = delete;
  GifDecoder& operator=(const GifDecoder&) = delete;
};

}  // namespace fxcodec

using GifDecoder = fxcodec::GifDecoder;

#endif  // CORE_FXCODEC_GIF_GIF_DECODER_H_
