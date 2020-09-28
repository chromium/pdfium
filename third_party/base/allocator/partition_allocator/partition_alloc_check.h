// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_CHECK_H_
#define THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_CHECK_H_

#include "third_party/base/allocator/partition_allocator/page_allocator_constants.h"
#include "third_party/base/logging.h"

#if defined(PAGE_ALLOCATOR_CONSTANTS_ARE_CONSTEXPR)

// Use this macro to assert on things that are conditionally constexpr as
// determined by PAGE_ALLOCATOR_CONSTANTS_ARE_CONSTEXPR or
// PAGE_ALLOCATOR_CONSTANTS_DECLARE_CONSTEXPR. Where fixed at compile time, this
// is a static_assert. Where determined at run time, this is a CHECK.
// Therefore, this macro must only be used where both a static_assert and a
// CHECK would be viable, that is, within a function, and ideally a function
// that executes only once, early in the program, such as during initialization.
#define STATIC_ASSERT_OR_CHECK(condition, message) \
  static_assert(condition, message)

#else

#define STATIC_ASSERT_OR_CHECK(condition, message) \
  do {                                             \
    CHECK(condition);                              \
  } while (false)

#endif

#endif  // THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ALLOC_CHECK_H_
