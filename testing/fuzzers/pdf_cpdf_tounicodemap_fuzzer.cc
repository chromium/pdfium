// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuzzer/FuzzedDataProvider.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_tounicodemap.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/retain_ptr.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  static constexpr size_t kParameterSize = sizeof(uint32_t) + sizeof(wchar_t);
  if (size <= kParameterSize)
    return 0;

  // Limit data size to prevent fuzzer timeout.
  static constexpr size_t kMaxDataSize = 256 * 1024;
  if (size > kParameterSize + kMaxDataSize)
    return 0;

  FuzzedDataProvider data_provider(data, size);
  uint32_t charcode_to_lookup = data_provider.ConsumeIntegral<uint32_t>();
  wchar_t char_for_reverse_lookup = data_provider.ConsumeIntegral<wchar_t>();

  std::vector<uint8_t> remaining =
      data_provider.ConsumeRemainingBytes<uint8_t>();
  auto stream = pdfium::MakeRetain<CPDF_Stream>();
  stream->SetData(remaining);

  auto to_unicode_map = std::make_unique<CPDF_ToUnicodeMap>(std::move(stream));
  to_unicode_map->Lookup(charcode_to_lookup);
  to_unicode_map->ReverseLookup(char_for_reverse_lookup);
  return 0;
}
