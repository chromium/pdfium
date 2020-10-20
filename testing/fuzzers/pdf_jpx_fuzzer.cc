// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <memory>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fxcodec/jpx/cjpx_decoder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

namespace {

const uint32_t kMaxJPXFuzzSize = 100 * 1024 * 1024;  // 100 MB

bool CheckImageSize(const CJPX_Decoder::JpxImageInfo& image_info) {
  static constexpr uint32_t kMemLimitBytes = 1024 * 1024 * 1024;  // 1 GB.
  FX_SAFE_UINT32 mem = image_info.width;
  mem *= image_info.height;
  mem *= image_info.components;
  return mem.IsValid() && mem.ValueOrDie() <= kMemLimitBytes;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 1)
    return 0;

  std::unique_ptr<CJPX_Decoder> decoder = CJPX_Decoder::Create(
      {data + 1, size - 1},
      static_cast<CJPX_Decoder::ColorSpaceOption>(data[0] % 3));
  if (!decoder)
    return 0;

  // A call to StartDecode could be too expensive if image size is very big, so
  // check size before calling StartDecode().
  CJPX_Decoder::JpxImageInfo image_info = decoder->GetInfo();
  if (!CheckImageSize(image_info))
    return 0;

  if (!decoder->StartDecode())
    return 0;

  // StartDecode() could change image size, so check again.
  image_info = decoder->GetInfo();
  if (!CheckImageSize(image_info))
    return 0;

  FXDIB_Format format;
  if (image_info.components == 1) {
    format = FXDIB_Format::k8bppRgb;
  } else if (image_info.components <= 3) {
    format = FXDIB_Format::kRgb;
  } else if (image_info.components == 4) {
    format = FXDIB_Format::kRgb32;
  } else {
    image_info.width = (image_info.width * image_info.components + 2) / 3;
    format = FXDIB_Format::kRgb;
  }
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(image_info.width, image_info.height, format))
    return 0;

  if (bitmap->GetHeight() <= 0 ||
      kMaxJPXFuzzSize / bitmap->GetPitch() <
          static_cast<uint32_t>(bitmap->GetHeight()))
    return 0;

  decoder->Decode(bitmap->GetBuffer(), bitmap->GetPitch(),
                  /*swap_rgb=*/false);

  return 0;
}
