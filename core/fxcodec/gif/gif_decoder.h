// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_GIF_DECODER_H_
#define CORE_FXCODEC_GIF_GIF_DECODER_H_

#include <memory>
#include <utility>

#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcodec/progressive_decoder_iface.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/span.h"

#ifndef PDF_ENABLE_XFA_GIF
#error "GIF must be enabled"
#endif

namespace fxcodec {

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
                                           int32_t trans_index,
                                           bool interlace) = 0;
    virtual void GifReadScanline(int32_t row_num,
                                 pdfium::span<uint8_t> row_buf) = 0;
  };

  static std::unique_ptr<ProgressiveDecoderIface::Context> StartDecode(
      Delegate* pDelegate);
  static Status ReadHeader(ProgressiveDecoderIface::Context* context,
                           int* width,
                           int* height,
                           pdfium::span<CFX_GifPalette>* pal_pp,
                           int* bg_index);
  static std::pair<Status, size_t> LoadFrameInfo(
      ProgressiveDecoderIface::Context* context);
  static Status LoadFrame(ProgressiveDecoderIface::Context* context,
                          size_t frame_num);
  static FX_FILESIZE GetAvailInput(ProgressiveDecoderIface::Context* context);
  static bool Input(ProgressiveDecoderIface::Context* context,
                    RetainPtr<CFX_CodecMemory> codec_memory);

  GifDecoder() = delete;
  GifDecoder(const GifDecoder&) = delete;
  GifDecoder& operator=(const GifDecoder&) = delete;
};

}  // namespace fxcodec

using GifDecoder = fxcodec::GifDecoder;

#endif  // CORE_FXCODEC_GIF_GIF_DECODER_H_
