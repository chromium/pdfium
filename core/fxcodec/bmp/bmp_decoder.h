// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_BMP_BMP_DECODER_H_
#define CORE_FXCODEC_BMP_BMP_DECODER_H_

#include <memory>

#include "core/fxcodec/progressive_decoder_iface.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/dib/fx_dib.h"

#ifndef PDF_ENABLE_XFA_BMP
#error "BMP must be enabled"
#endif

namespace fxcodec {

class CFX_DIBAttribute;

class BmpDecoder {
 public:
  class Delegate {
   public:
    virtual bool BmpInputImagePositionBuf(uint32_t rcd_pos) = 0;
    virtual void BmpReadScanline(uint32_t row_num,
                                 pdfium::span<const uint8_t> row_buf) = 0;
  };

  enum class Status : uint8_t { kFail, kSuccess, kContinue };

  static std::unique_ptr<ProgressiveDecoderIface::Context> StartDecode(
      Delegate* pDelegate);
  static Status ReadHeader(ProgressiveDecoderIface::Context* pContext,
                           int32_t* width,
                           int32_t* height,
                           bool* tb_flag,
                           int32_t* components,
                           pdfium::span<const FX_ARGB>* palette,
                           CFX_DIBAttribute* pAttribute);
  static Status LoadImage(ProgressiveDecoderIface::Context* pContext);
  static FX_FILESIZE GetAvailInput(ProgressiveDecoderIface::Context* pContext);
  static bool Input(ProgressiveDecoderIface::Context* pContext,
                    RetainPtr<CFX_CodecMemory> codec_memory);

  BmpDecoder() = delete;
  BmpDecoder(const BmpDecoder&) = delete;
  BmpDecoder& operator=(const BmpDecoder&) = delete;
};

}  // namespace fxcodec

using BmpDecoder = fxcodec::BmpDecoder;

#endif  // CORE_FXCODEC_BMP_BMP_DECODER_H_
