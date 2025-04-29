// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>

#include "core/fxcodec/icc/icc_transform.h"
#include "core/fxcrt/span.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::unique_ptr<fxcodec::IccTransform> transform =
      fxcodec::IccTransform::CreateTransformSRGB(pdfium::span(data, size));
  if (!transform) {
    return 0;
  }

  const float src[4] = {0.5f, 0.5f, 0.5f, 0.5f};
  float dst[4];
  transform->Translate(
      pdfium::span(src).first(static_cast<size_t>(transform->components())),
      pdfium::span(dst));
  return 0;
}
