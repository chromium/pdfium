// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_GIF_PROGRESSIVE_DECODER_H_
#define CORE_FXCODEC_GIF_GIF_PROGRESSIVE_DECODER_H_

#include "core/fxcodec/progressive_decoder_iface.h"
#include "third_party/base/no_destructor.h"

#ifndef PDF_ENABLE_XFA_GIF
#error "GIF must be enabled"
#endif

namespace fxcodec {

class GifProgressiveDecoder final : public ProgressiveDecoderIface {
 public:
  static GifProgressiveDecoder* GetInstance();

  // ProgressiveDecoderIface:
  FX_FILESIZE GetAvailInput(Context* context) const override;
  bool Input(Context* context,
             RetainPtr<CFX_CodecMemory> codec_memory) override;

 private:
  friend pdfium::base::NoDestructor<GifProgressiveDecoder>;

  GifProgressiveDecoder();
  ~GifProgressiveDecoder() override;
};

}  // namespace fxcodec

using GifProgressiveDecoder = fxcodec::GifProgressiveDecoder;

#endif  // CORE_FXCODEC_GIF_GIF_PROGRESSIVE_DECODER_H_
