// Copyright 2017 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <memory>

#include "core/fxcodec/basic/basicmodule.h"
#include "core/fxcrt/fx_memory_wrappers.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::unique_ptr<uint8_t, FxFreeDeleter> dest_buf;
  uint32_t dest_size = 0;
  BasicModule::A85Encode({data, size}, &dest_buf, &dest_size);
  return 0;
}
