// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "samples/chromium_support/discardable_memory_allocator.h"

#include "base/test/test_discardable_memory_allocator.h"

namespace chromium_support {

void InitializeDiscardableMemoryAllocator() {
  static base::TestDiscardableMemoryAllocator test_memory_allocator;
  base::DiscardableMemoryAllocator::SetInstance(&test_memory_allocator);
}

}  // namespace chromium_support
