// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_RANDOM_H_
#define THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_RANDOM_H_

#include <stdint.h>

#include "third_party/base/base_export.h"

namespace pdfium {
namespace base {

BASE_EXPORT uint32_t RandomValue();

// TODO(crbug.com/984742): Rename this to `SetRandomSeedForTesting`.
//
// Sets the seed for the random number generator to a known value, to cause the
// RNG to generate a predictable sequence of outputs. May be called multiple
// times.
BASE_EXPORT void SetRandomPageBaseSeed(int64_t seed);

}  // namespace base
}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_RANDOM_H_
