// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "core/fxcodec/basic/basicmodule.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  BasicModule::A85Encode({data, size});
  return 0;
}
