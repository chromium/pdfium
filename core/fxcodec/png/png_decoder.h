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
    virtual bool PngReadHeader(int width,
                               int height,
                               int bpc,
                               int pass,
                               int* color_type,
                               double* gamma) = 0;

    // `line` must be within [0, height].
    virtual uint8_t* PngAskScanlineBuf(int line) = 0;

    virtual void PngFillScanlineBufCompleted(int pass, int line) = 0;
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
