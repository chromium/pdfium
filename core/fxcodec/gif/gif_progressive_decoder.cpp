// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/gif_progressive_decoder.h"

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/gif/gif_decoder.h"

namespace fxcodec {

// static
GifProgressiveDecoder* GifProgressiveDecoder::GetInstance() {
  static pdfium::base::NoDestructor<GifProgressiveDecoder> s;
  return s.get();
}

GifProgressiveDecoder::GifProgressiveDecoder() = default;

GifProgressiveDecoder::~GifProgressiveDecoder() = default;

FX_FILESIZE GifProgressiveDecoder::GetAvailInput(Context* context) const {
  return GifDecoder::GetAvailInput(context);
}

bool GifProgressiveDecoder::Input(Context* context,
                                  RetainPtr<CFX_CodecMemory> codec_memory,
                                  CFX_DIBAttribute* pAttribute) {
  return GifDecoder::Input(context, codec_memory, pAttribute);
}

}  // namespace fxcodec
