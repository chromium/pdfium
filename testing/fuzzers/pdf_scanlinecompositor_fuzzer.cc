// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <memory>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_cliprgn.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "testing/fuzzers/pdfium_fuzzer_util.h"

namespace {

// Some unused formats were removed, and their slots have been filled in
// `FXDIB_Format::kInvalid` to keep the fuzzer input stable.
constexpr FXDIB_Format kFormat[] = {
    FXDIB_Format::kInvalid,
    FXDIB_Format::k1bppRgb,
    FXDIB_Format::k8bppRgb,
    FXDIB_Format::kRgb,
    FXDIB_Format::kRgb32,
    FXDIB_Format::k1bppMask,
    FXDIB_Format::k8bppMask,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::k8bppRgba */,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::kRgba */,
    FXDIB_Format::kArgb,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::k1bppCmyk */,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::k8bppCmyk */,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::kCmyk */,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::k8bppCmyka */,
    FXDIB_Format::kInvalid /* Was FXDIB_Format::kCmyka */};

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  constexpr size_t kParameterSize = 33;
  if (size < kParameterSize)
    return 0;

  int width = GetInteger(data);
  int height = GetInteger(data + 4);
  uint32_t argb = GetInteger(data + 8);
  int src_left = GetInteger(data + 12);
  int src_top = GetInteger(data + 16);
  int dest_left = GetInteger(data + 20);
  int dest_top = GetInteger(data + 24);

  BlendMode blend_mode = static_cast<BlendMode>(
      data[28] % (static_cast<int>(BlendMode::kLast) + 1));
  FXDIB_Format dest_format = kFormat[data[29] % std::size(kFormat)];
  FXDIB_Format src_format = kFormat[data[30] % std::size(kFormat)];
  bool is_clip = !(data[31] % 2);
  bool is_rgb_byte_order = !(data[32] % 2);
  size -= kParameterSize;
  data += kParameterSize;

  static constexpr uint32_t kMemLimit = 512000000;  // 512 MB
  static constexpr uint32_t kComponents = 4;
  FX_SAFE_UINT32 mem = width;
  mem *= height;
  mem *= kComponents;
  if (!mem.IsValid() || mem.ValueOrDie() > kMemLimit)
    return 0;

  auto src_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  auto dest_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!src_bitmap->Create(width, height, src_format) ||
      !dest_bitmap->Create(width, height, dest_format)) {
    return 0;
  }
  if (src_bitmap->GetBuffer().empty() || dest_bitmap->GetBuffer().empty()) {
    return 0;
  }

  std::unique_ptr<CFX_ClipRgn> clip_rgn;
  if (is_clip)
    clip_rgn = std::make_unique<CFX_ClipRgn>(width, height);
  if (src_bitmap->IsMaskFormat()) {
    dest_bitmap->CompositeMask(dest_left, dest_top, width, height, src_bitmap,
                               argb, src_left, src_top, blend_mode,
                               clip_rgn.get(), is_rgb_byte_order);
  } else {
    dest_bitmap->CompositeBitmap(dest_left, dest_top, width, height, src_bitmap,
                                 src_left, src_top, blend_mode, clip_rgn.get(),
                                 is_rgb_byte_order);
  }
  return 0;
}
