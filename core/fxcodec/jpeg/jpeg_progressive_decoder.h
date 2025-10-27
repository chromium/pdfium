// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_
#define CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_

#include <stdint.h>

#include <memory>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/progressive_decoder_iface.h"
#include "core/fxcrt/fx_types.h"

namespace fxcodec {

class CFX_DIBAttribute;

class JpegProgressiveDecoder {
 public:
  static std::unique_ptr<ProgressiveDecoderIface::Context> Start();

  // Result codes for ReadHeader()/ReadScanline():
  static constexpr int kFatal = -1;
  static constexpr int kOk = 0;
  static constexpr int kError = 1;
  static constexpr int kNeedsMoreInput = 2;

  static int ReadHeader(ProgressiveDecoderIface::Context* pContext,
                        int* width,
                        int* height,
                        int* nComps,
                        CFX_DIBAttribute* pAttribute);

  static bool StartScanline(ProgressiveDecoderIface::Context* pContext);
  static int ReadScanline(ProgressiveDecoderIface::Context* pContext,
                          uint8_t* dest_buf);

  static FX_FILESIZE GetAvailInput(ProgressiveDecoderIface::Context* pContext);
  static bool Input(ProgressiveDecoderIface::Context* pContext,
                    RetainPtr<CFX_CodecMemory> codec_memory);

  // Only `static` methods.
  JpegProgressiveDecoder() = delete;
  JpegProgressiveDecoder(const JpegProgressiveDecoder&) = delete;
  JpegProgressiveDecoder& operator=(const JpegProgressiveDecoder&) = delete;
};

}  // namespace fxcodec

using JpegProgressiveDecoder = fxcodec::JpegProgressiveDecoder;

#endif  // CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_
