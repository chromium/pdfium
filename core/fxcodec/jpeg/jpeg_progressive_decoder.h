// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_
#define CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_

#include <memory>

#include "core/fxcodec/progressive_decoder_iface.h"

namespace fxcodec {

class CFX_DIBAttribute;

class JpegProgressiveDecoder final : public ProgressiveDecoderIface {
 public:
  static void InitializeGlobals();
  static void DestroyGlobals();
  static JpegProgressiveDecoder* GetInstance();
  static std::unique_ptr<Context> Start();

  // Result codes for ReadHeader()/ReadScanline():
  static constexpr int kFatal = -1;
  static constexpr int kOk = 0;
  static constexpr int kError = 1;
  static constexpr int kNeedsMoreInput = 2;

  static int ReadHeader(Context* pContext,
                        int* width,
                        int* height,
                        int* nComps,
                        CFX_DIBAttribute* pAttribute);

  static bool StartScanline(Context* pContext);
  static int ReadScanline(Context* pContext, uint8_t* dest_buf);

  // ProgressiveDecoderIface:
  FX_FILESIZE GetAvailInput(Context* pContext) const override;
  bool Input(Context* pContext,
             RetainPtr<CFX_CodecMemory> codec_memory) override;

 private:
  JpegProgressiveDecoder();
  ~JpegProgressiveDecoder() override;
};

}  // namespace fxcodec

using JpegProgressiveDecoder = fxcodec::JpegProgressiveDecoder;

#endif  // CORE_FXCODEC_JPEG_JPEG_PROGRESSIVE_DECODER_H_
