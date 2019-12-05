// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_
#define THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_

#include <stdint.h>

#include "build/build_config.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc_constants.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/sys_byteorder.h"

namespace pdfium {
namespace base {
namespace internal {

struct EncodedPartitionFreelistEntry;

struct PartitionFreelistEntry {
  EncodedPartitionFreelistEntry* next;

  PartitionFreelistEntry() = delete;
  ~PartitionFreelistEntry() = delete;

  ALWAYS_INLINE static EncodedPartitionFreelistEntry* Encode(
      PartitionFreelistEntry* ptr) {
    return reinterpret_cast<EncodedPartitionFreelistEntry*>(Transform(ptr));
  }

 private:
  friend struct EncodedPartitionFreelistEntry;
  static ALWAYS_INLINE void* Transform(void* ptr) {
    // We use bswap on little endian as a fast mask for two reasons:
    // 1) If an object is freed and its vtable used where the attacker doesn't
    // get the chance to run allocations between the free and use, the vtable
    // dereference is likely to fault.
    // 2) If the attacker has a linear buffer overflow and elects to try and
    // corrupt a freelist pointer, partial pointer overwrite attacks are
    // thwarted.
    // For big endian, similar guarantees are arrived at with a negation.
#if defined(ARCH_CPU_BIG_ENDIAN)
    uintptr_t masked = ~reinterpret_cast<uintptr_t>(ptr);
#else
    uintptr_t masked = ByteSwapUintPtrT(reinterpret_cast<uintptr_t>(ptr));
#endif
    return reinterpret_cast<void*>(masked);
  }
};

struct EncodedPartitionFreelistEntry {
  char scrambled[sizeof(PartitionFreelistEntry*)];

  EncodedPartitionFreelistEntry() = delete;
  ~EncodedPartitionFreelistEntry() = delete;

  ALWAYS_INLINE static PartitionFreelistEntry* Decode(
      EncodedPartitionFreelistEntry* ptr) {
    return reinterpret_cast<PartitionFreelistEntry*>(
        PartitionFreelistEntry::Transform(ptr));
  }
};

static_assert(sizeof(PartitionFreelistEntry) ==
                  sizeof(EncodedPartitionFreelistEntry),
              "Should not have padding");

}  // namespace internal
}  // namespace base
}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_FREELIST_ENTRY_H_
