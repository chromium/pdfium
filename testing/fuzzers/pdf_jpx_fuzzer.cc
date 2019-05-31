// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdint>
#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fxcodec/codec/ccodec_jpxmodule.h"
#include "core/fxcodec/codec/cjpx_decoder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"

namespace {

const uint32_t kMaxJPXFuzzSize = 100 * 1024 * 1024;  // 100 MB

bool CheckImageSize(uint32_t width, uint32_t height, uint32_t components) {
  static constexpr uint32_t kMemLimitBytes = 1024 * 1024 * 1024;  // 1 GB.
  FX_SAFE_UINT32 mem = width;
  mem *= height;
  mem *= components;
  return mem.IsValid() && mem.ValueOrDie() <= kMemLimitBytes;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 1)
    return 0;

  std::unique_ptr<CJPX_Decoder> decoder = CCodec_JpxModule::CreateDecoder(
      {data + 1, size - 1},
      static_cast<CJPX_Decoder::ColorSpaceOption>(data[0] % 3));
  if (!decoder)
    return 0;

  // A call to StartDecode could be too expensive if image size is very big, so
  // check size before calling StartDecode().
  uint32_t width;
  uint32_t height;
  uint32_t components;
  decoder->GetInfo(&width, &height, &components);
  if (!CheckImageSize(width, height, components))
    return 0;

  if (!decoder->StartDecode())
    return 0;

  // StartDecode() could change image size, so check again.
  decoder->GetInfo(&width, &height, &components);
  if (!CheckImageSize(width, height, components))
    return 0;

  FXDIB_Format format;
  if (components == 1) {
    format = FXDIB_8bppRgb;
  } else if (components <= 3) {
    format = FXDIB_Rgb;
  } else if (components == 4) {
    format = FXDIB_Rgb32;
  } else {
    width = (width * components + 2) / 3;
    format = FXDIB_Rgb;
  }
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(width, height, format))
    return 0;

  if (bitmap->GetHeight() <= 0 ||
      kMaxJPXFuzzSize / bitmap->GetPitch() <
          static_cast<uint32_t>(bitmap->GetHeight()))
    return 0;

  std::vector<uint8_t> output_offsets(components);
  for (uint32_t i = 0; i < components; ++i)
    output_offsets[i] = i;

  decoder->Decode(bitmap->GetBuffer(), bitmap->GetPitch(), output_offsets);
  return 0;
}
