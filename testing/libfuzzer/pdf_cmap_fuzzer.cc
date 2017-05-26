// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "core/fpdfapi/font/cpdf_cmap.h"
#include "third_party/base/ptr_util.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  auto cmap = pdfium::MakeRetain<CPDF_CMap>();
  cmap->LoadEmbedded(data, size);
  return 0;
}
