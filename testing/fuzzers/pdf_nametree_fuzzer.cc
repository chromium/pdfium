// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuzzer/FuzzedDataProvider.h>

#include <cstdint>
#include <vector>

#include "core/fpdfapi/page/cpdf_streamparser.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/span.h"

struct Params {
  bool delete_backwards;
  uint8_t count;
  std::vector<WideString> names;
};

std::vector<WideString> GetNames(uint8_t count,
                                 FuzzedDataProvider* data_provider) {
  std::vector<WideString> names;
  names.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    // The name is not that interesting here. Keep it short.
    constexpr size_t kMaxNameLen = 10;
    std::string str = data_provider->ConsumeRandomLengthString(kMaxNameLen);
    names.push_back(WideString::FromUTF16LE(pdfium::as_byte_span(str)));
  }
  return names;
}

Params GetParams(FuzzedDataProvider* data_provider) {
  Params params;
  params.delete_backwards = data_provider->ConsumeBool();
  params.count = data_provider->ConsumeIntegralInRange(1, 255);
  params.names = GetNames(params.count, data_provider);
  return params;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzedDataProvider data_provider(data, size);
  Params params = GetParams(&data_provider);

  // |remaining| needs to outlive |parser|.
  std::vector<uint8_t> remaining =
      data_provider.ConsumeRemainingBytes<uint8_t>();
  if (remaining.empty())
    return 0;

  CPDF_StreamParser parser(remaining);
  auto dict = pdfium::MakeRetain<CPDF_Dictionary>();
  std::unique_ptr<CPDF_NameTree> name_tree =
      CPDF_NameTree::CreateForTesting(dict.Get());
  for (const auto& name : params.names) {
    RetainPtr<CPDF_Object> obj = parser.ReadNextObject(
        /*bAllowNestedArray*/ true, /*bInArray=*/false, /*dwRecursionLevel=*/0);
    if (!obj)
      break;

    name_tree->AddValueAndName(std::move(obj), name);
  }

  if (params.delete_backwards) {
    for (size_t i = params.count; i > 0; --i)
      name_tree->DeleteValueAndName(i);
  } else {
    for (size_t i = 0; i < params.count; ++i)
      name_tree->DeleteValueAndName(0);
  }
  return 0;
}
