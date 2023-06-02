// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <new>

#include "base/allocator/partition_allocator/partition_alloc.h"
#include "build/build_config.h"
#include "third_party/base/no_destructor.h"

#if !defined(PDF_USE_PARTITION_ALLOC)
#error "Malloc shim must use partition alloc."
#endif

namespace {

constexpr partition_alloc::PartitionOptions kOptions = {
    partition_alloc::PartitionOptions::AlignedAlloc::kDisallowed,
    partition_alloc::PartitionOptions::ThreadCache::kDisabled,
    partition_alloc::PartitionOptions::Quarantine::kDisallowed,
    partition_alloc::PartitionOptions::Cookie::kAllowed,
#if BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
    partition_alloc::PartitionOptions::BackupRefPtr::kEnabled,
#else
    partition_alloc::PartitionOptions::BackupRefPtr::kDisabled,
#endif
    partition_alloc::PartitionOptions::UseConfigurablePool::kNo,
};

#if BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
void DanglingDetected(uintptr_t id) {
  abort();  // Must die for unit test death tests.
}
#endif

partition_alloc::PartitionAllocator& GetTestBrpPartitionAllocator() {
  static bool s_allocator_initialized = false;
  static pdfium::base::NoDestructor<partition_alloc::PartitionAllocator>
      s_allocator;

  if (!s_allocator_initialized) {
    s_allocator_initialized = true;  // Set flag first ...
    s_allocator->init(kOptions);     // ... may re-enter.
#if BUILDFLAG(ENABLE_BACKUP_REF_PTR_SUPPORT)
    partition_alloc::SetDanglingRawPtrDetectedFn(DanglingDetected);
    partition_alloc::SetUnretainedDanglingRawPtrDetectedFn(DanglingDetected);
    partition_alloc::SetUnretainedDanglingRawPtrCheckEnabled(true);
#endif
  }
  return *s_allocator;
}

void* TestBrpPartitionNewOrDie(size_t size) noexcept {
  return GetTestBrpPartitionAllocator().root()->AllocWithFlags(0, size,
                                                               "BRP Partition");
}

void TestBrpPartitionDelete(void* p) noexcept {
  GetTestBrpPartitionAllocator().root()->Free(p);
}

}  // namespace

void* operator new(size_t size) {
  return TestBrpPartitionNewOrDie(size);
}

void* operator new[](size_t size) {
  return TestBrpPartitionNewOrDie(size);
}

void operator delete(void* p) {
  TestBrpPartitionDelete(p);
}

void operator delete[](void* p) {
  TestBrpPartitionDelete(p);
}

void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return TestBrpPartitionNewOrDie(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return TestBrpPartitionNewOrDie(size);
}

void operator delete(void* p, const std::nothrow_t&) noexcept {
  TestBrpPartitionDelete(p);
}

void operator delete[](void* p, const std::nothrow_t&) noexcept {
  TestBrpPartitionDelete(p);
}
