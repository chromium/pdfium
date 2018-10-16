// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <memory>

#include "core/fxcodec/codec/ccodec_basicmodule.h"
#include "core/fxcrt/fx_memory.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf;
  uint32_t dest_size = 0;
  CCodec_BasicModule encoder_module;
  encoder_module.RunLengthEncode({data, size}, &dest_buf, &dest_size);
  return 0;
}
