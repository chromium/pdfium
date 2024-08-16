// Copyright 2016 The PDFium Authors
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
  mem *= image_info.channels;
  return mem.IsValid() && mem.ValueOrDie() <= kMemLimitBytes;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 3) {
    return 0;
  }

  // SAFETY: trusted arguments from fuzzer.
  auto span = UNSAFE_BUFFERS(pdfium::make_span(data, size));

  auto color_space_option =
      static_cast<CJPX_Decoder::ColorSpaceOption>(data[0] % 3);
  uint8_t resolution_levels_to_skip = data[1];
  bool strict_mode = !!data[2];

  std::unique_ptr<CJPX_Decoder> decoder =
      CJPX_Decoder::Create(span.subspan(3u), color_space_option,
                           resolution_levels_to_skip, strict_mode);
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
  if (image_info.channels == 1) {
    format = FXDIB_Format::k8bppRgb;
  } else if (image_info.channels <= 3) {
    format = FXDIB_Format::kBgr;
  } else if (image_info.channels == 4) {
    format = FXDIB_Format::kBgrx;
  } else {
    image_info.width = (image_info.width * image_info.channels + 2) / 3;
    format = FXDIB_Format::kBgr;
  }
  auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!bitmap->Create(image_info.width, image_info.height, format))
    return 0;

  if (bitmap->GetHeight() <= 0 ||
      kMaxJPXFuzzSize / bitmap->GetPitch() <
          static_cast<uint32_t>(bitmap->GetHeight()))
    return 0;

  decoder->Decode(bitmap->GetWritableBuffer(), bitmap->GetPitch(),
                  /*swap_rgb=*/false, GetCompsFromFormat(format));

  return 0;
}
