// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "core/fxcodec/gif/lzw_decompressor.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span.h"

// Between 2x and 5x is a standard range for LZW according to a quick
// search of papers. Running up to 10x to catch any niche cases.
constexpr uint32_t kMinCompressionRatio = 2;
constexpr uint32_t kMaxCompressionRatio = 10;

static constexpr size_t kMaxFuzzBytes = 1024 * 1024 * 1024;  // 1 GB.

void LZWFuzz(pdfium::span<const uint8_t> src_buf,
             uint8_t color_exp,
             uint8_t code_exp) {
  std::unique_ptr<LZWDecompressor> decompressor =
      LZWDecompressor::Create(color_exp, code_exp);
  if (!decompressor)
    return;

  for (uint32_t compressions_ratio = kMinCompressionRatio;
       compressions_ratio <= kMaxCompressionRatio; compressions_ratio++) {
    FX_SAFE_UINT32 safe_dest_size = src_buf.size();
    safe_dest_size *= compressions_ratio;
    uint32_t dest_size = safe_dest_size.ValueOrDie();
    std::vector<uint8_t> dest_buf(dest_size);
    decompressor->SetSource(src_buf);
    if (LZWDecompressor::Status::kInsufficientDestSize !=
        decompressor->Decode(dest_buf.data(), &dest_size)) {
      return;
    }
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // SAFETY: required from fuzzer
  auto data_span = UNSAFE_BUFFERS(pdfium::make_span(data, size));

  // Need at least 3 bytes to do anything.
  if (data_span.size() < 3 || data_span.size() > kMaxFuzzBytes) {
    return 0;
  }

  // Normally the GIF would provide the code and color sizes, instead, going
  // to assume they are the first two bytes of data provided.
  uint8_t color_exp = data_span[0];
  uint8_t code_exp = data_span[1];
  pdfium::span<const uint8_t> lzw_data = data_span.subspan(2);
  // Check that there isn't going to be an overflow in the destination buffer
  // size.
  if (lzw_data.size() >
      std::numeric_limits<uint32_t>::max() / kMaxCompressionRatio) {
    return 0;
  }

  LZWFuzz(lzw_data, color_exp, code_exp);
  return 0;
}
