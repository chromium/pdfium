// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "testing/fuzzers/pdfium_fuzzer_helper.h"

#if defined(BUILD_WITH_CHROMIUM)
#include "base/memory/discardable_memory_allocator.h"
#include "base/no_destructor.h"
#include "base/test/test_discardable_memory_allocator.h"
#endif

class PDFiumFuzzer : public PDFiumFuzzerHelper {
 public:
  PDFiumFuzzer() = default;
  ~PDFiumFuzzer() override = default;

  int GetFormCallbackVersion() const override { return 1; }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
#if defined(BUILD_WITH_CHROMIUM)
  static base::NoDestructor<base::TestDiscardableMemoryAllocator>
      test_memory_allocator;
  base::DiscardableMemoryAllocator::SetInstance(test_memory_allocator.get());
#endif

  PDFiumFuzzer fuzzer;
  fuzzer.RenderPdf(reinterpret_cast<const char*>(data), size);
  return 0;
}
