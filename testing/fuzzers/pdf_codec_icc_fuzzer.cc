// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "core/fxcodec/icc/iccmodule.h"
#include "third_party/base/span.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::unique_ptr<CLcmsCmm> transform =
      IccModule::CreateTransformSRGB(pdfium::make_span(data, size));

  if (transform) {
    float src[4];
    float dst[4];
    for (int i = 0; i < 4; i++)
      src[i] = 0.5f;
    IccModule::Translate(transform.get(), transform->components(), src, dst);
  }

  return 0;
}
