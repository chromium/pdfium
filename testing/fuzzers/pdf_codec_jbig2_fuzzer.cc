// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_DocumentContext.h"
#include "core/fxcodec/jbig2/jbig2_decoder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/fuzzers/pdfium_fuzzer_util.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  const size_t kParameterSize = 8;
  if (size < kParameterSize)
    return 0;

  uint32_t width = GetInteger(data);
  uint32_t height = GetInteger(data + 4);
  size -= kParameterSize;
  data += kParameterSize;

  static constexpr uint32_t kMemLimit = 512000000;   // 512 MB
  static constexpr uint32_t k1bppRgbComponents = 4;  // From CFX_DIBitmap impl.
  FX_SAFE_UINT32 mem = width;
  mem *= height;
  mem *= k1bppRgbComponents;
  if (!mem.IsValid() || mem.ValueOrDie() > kMemLimit)
    return 0;

  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(width, height, FXDIB_Format::k1bppRgb))
    return 0;

  JBig2_DocumentContext document_context;
  Jbig2Context jbig2_context;
  FXCODEC_STATUS status = Jbig2Decoder::StartDecode(
      &jbig2_context, &document_context, width, height, {data, size}, 1, {}, 0,
      bitmap->GetWritableBuffer(), bitmap->GetPitch(), nullptr);

  while (status == FXCODEC_STATUS::kDecodeToBeContinued)
    status = Jbig2Decoder::ContinueDecode(&jbig2_context, nullptr);
  return 0;
}
