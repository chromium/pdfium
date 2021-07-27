// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FUZZERS_XFA_CODEC_FUZZER_H_
#define TESTING_FUZZERS_XFA_CODEC_FUZZER_H_

#include <memory>

#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/progressive_decoder.h"
#include "core/fxcrt/cfx_readonlymemorystream.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "third_party/base/span.h"

// Support up to 64 MB. This prevents trivial OOM when MSAN is on and
// time outs.
const int kXFACodecFuzzerPixelLimit = 64000000;

class XFACodecFuzzer {
 public:
  static int Fuzz(const uint8_t* data, size_t size, FXCODEC_IMAGE_TYPE type) {
    auto decoder = std::make_unique<ProgressiveDecoder>();
    auto source = pdfium::MakeRetain<CFX_ReadOnlyMemoryStream>(
        pdfium::make_span(data, size));
    CFX_DIBAttribute attr;
    FXCODEC_STATUS status = decoder->LoadImageInfo(source, type, &attr, true);
    if (status != FXCODEC_STATUS_FRAME_READY)
      return 0;

    // Skipping very large images, since they will take a long time and may lead
    // to OOM.
    FX_SAFE_UINT32 bitmap_size = decoder->GetHeight();
    bitmap_size *= decoder->GetWidth();
    bitmap_size *= 4;  // From CFX_DIBitmap impl.
    if (!bitmap_size.IsValid() ||
        bitmap_size.ValueOrDie() > kXFACodecFuzzerPixelLimit) {
      return 0;
    }

    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    bitmap->Create(decoder->GetWidth(), decoder->GetHeight(),
                   FXDIB_Format::kArgb);

    size_t frames;
    std::tie(status, frames) = decoder->GetFrames();
    if (status != FXCODEC_STATUS_DECODE_READY || frames == 0)
      return 0;

    status = decoder->StartDecode(bitmap, 0, 0, bitmap->GetWidth(),
                                  bitmap->GetHeight());
    while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE)
      status = decoder->ContinueDecode();

    return 0;
  }
};

#endif  // TESTING_FUZZERS_XFA_CODEC_FUZZER_H_
