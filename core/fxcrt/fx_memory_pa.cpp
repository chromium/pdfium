// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include "base/allocator/partition_allocator/partition_alloc.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/no_destructor.h"

#if !defined(PDF_USE_PARTITION_ALLOC)
#error "File compiled under wrong build option."
#endif

namespace {

constexpr partition_alloc::PartitionOptions kOptions = {
    partition_alloc::PartitionOptions::AlignedAlloc::kDisallowed,
    partition_alloc::PartitionOptions::ThreadCache::kDisabled,
    partition_alloc::PartitionOptions::Quarantine::kDisallowed,
    partition_alloc::PartitionOptions::Cookie::kAllowed,
    partition_alloc::PartitionOptions::BackupRefPtr::kDisabled,
    partition_alloc::PartitionOptions::BackupRefPtrZapping::kDisabled,
    partition_alloc::PartitionOptions::UseConfigurablePool::kNo,
};

#ifndef V8_ENABLE_SANDBOX
partition_alloc::PartitionAllocator& GetArrayBufferPartitionAllocator() {
  static pdfium::base::NoDestructor<partition_alloc::PartitionAllocator>
      s_array_buffer_allocator;
  return *s_array_buffer_allocator;
}
#endif  //  V8_ENABLE_SANDBOX

partition_alloc::PartitionAllocator& GetGeneralPartitionAllocator() {
  static pdfium::base::NoDestructor<partition_alloc::PartitionAllocator>
      s_general_allocator;
  return *s_general_allocator;
}

partition_alloc::PartitionAllocator& GetStringPartitionAllocator() {
  static pdfium::base::NoDestructor<partition_alloc::PartitionAllocator>
      s_string_allocator;
  return *s_string_allocator;
}

}  // namespace

namespace pdfium {
namespace internal {

void* Alloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;

  return GetGeneralPartitionAllocator().root()->AllocWithFlags(
      partition_alloc::AllocFlags::kReturnNull, total.ValueOrDie(),
      "GeneralPartition");
}

void* Calloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;

  return GetGeneralPartitionAllocator().root()->AllocWithFlags(
      partition_alloc::AllocFlags::kReturnNull |
          partition_alloc::AllocFlags::kZeroFill,
      total.ValueOrDie(), "GeneralPartition");
}

void* Realloc(void* ptr, size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T size = num_members;
  size *= member_size;
  if (!size.IsValid())
    return nullptr;

  return GetGeneralPartitionAllocator().root()->ReallocWithFlags(
      partition_alloc::AllocFlags::kReturnNull, ptr, size.ValueOrDie(),
      "GeneralPartition");
}

void* StringAlloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;

  return GetStringPartitionAllocator().root()->AllocWithFlags(
      partition_alloc::AllocFlags::kReturnNull, total.ValueOrDie(),
      "StringPartition");
}

}  // namespace internal
}  // namespace pdfium

void FX_InitializeMemoryAllocators() {
  static bool s_partition_allocators_initialized = false;
  if (!s_partition_allocators_initialized) {
    partition_alloc::PartitionAllocGlobalInit(FX_OutOfMemoryTerminate);
#ifndef V8_ENABLE_SANDBOX
    GetArrayBufferPartitionAllocator().init(kOptions);
#endif  // V8_ENABLE_SANDBOX
    GetGeneralPartitionAllocator().init(kOptions);
    GetStringPartitionAllocator().init(kOptions);
    s_partition_allocators_initialized = true;
  }
}

#ifndef V8_ENABLE_SANDBOX
void* FX_ArrayBufferAllocate(size_t length) {
  return GetArrayBufferPartitionAllocator().root()->AllocWithFlags(
      partition_alloc::AllocFlags::kZeroFill, length, "FXArrayBuffer");
}

void* FX_ArrayBufferAllocateUninitialized(size_t length) {
  return GetArrayBufferPartitionAllocator().root()->Alloc(length,
                                                          "FXArrayBuffer");
}

void FX_ArrayBufferFree(void* data) {
  GetArrayBufferPartitionAllocator().root()->Free(data);
}
#endif  // V8_ENABLE_SANDBOX

void FX_Free(void* ptr) {
  // TODO(palmer): Removing this check exposes crashes when PDFium callers
  // attempt to free |nullptr|. Although libc's |free| allows freeing |NULL|, no
  // other Partition Alloc callers need this tolerant behavior. Additionally,
  // checking for |nullptr| adds a branch to |PartitionFree|, and it's nice to
  // not have to have that.
  //
  // So this check is hiding (what I consider to be) bugs, and we should try to
  // fix them. https://bugs.chromium.org/p/pdfium/issues/detail?id=690
  if (ptr)
    partition_alloc::ThreadSafePartitionRoot::Free(ptr);
}
