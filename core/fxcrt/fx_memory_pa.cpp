// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"
#include "third_party/base/no_destructor.h"

namespace {

pdfium::base::PartitionAllocatorGeneric& GetArrayBufferPartitionAllocator() {
  static pdfium::base::NoDestructor<pdfium::base::PartitionAllocatorGeneric>
      s_array_buffer_allocator;
  return *s_array_buffer_allocator;
}

pdfium::base::PartitionAllocatorGeneric& GetGeneralPartitionAllocator() {
  static pdfium::base::NoDestructor<pdfium::base::PartitionAllocatorGeneric>
      s_general_allocator;
  return *s_general_allocator;
}

pdfium::base::PartitionAllocatorGeneric& GetStringPartitionAllocator() {
  static pdfium::base::NoDestructor<pdfium::base::PartitionAllocatorGeneric>
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
  constexpr int kFlags = pdfium::base::PartitionAllocReturnNull;
  return pdfium::base::PartitionAllocGenericFlags(
      GetGeneralPartitionAllocator().root(), kFlags, total.ValueOrDie(),
      "GeneralPartition");
}

void* Calloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;

  constexpr int kFlags = pdfium::base::PartitionAllocReturnNull |
                         pdfium::base::PartitionAllocZeroFill;
  return pdfium::base::PartitionAllocGenericFlags(
      GetGeneralPartitionAllocator().root(), kFlags, total.ValueOrDie(),
      "GeneralPartition");
}

void* Realloc(void* ptr, size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T size = num_members;
  size *= member_size;
  if (!size.IsValid())
    return nullptr;

  return pdfium::base::PartitionReallocGenericFlags(
      GetGeneralPartitionAllocator().root(),
      pdfium::base::PartitionAllocReturnNull, ptr, size.ValueOrDie(),
      "GeneralPartition");
}

void* StringAlloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;

  constexpr int kFlags = pdfium::base::PartitionAllocReturnNull;
  return pdfium::base::PartitionAllocGenericFlags(
      GetStringPartitionAllocator().root(), kFlags, total.ValueOrDie(),
      "StringPartition");
}

}  // namespace internal
}  // namespace pdfium

void FX_InitializeMemoryAllocators() {
  static bool s_partition_allocators_initialized = false;
  if (!s_partition_allocators_initialized) {
    pdfium::base::PartitionAllocGlobalInit(FX_OutOfMemoryTerminate);
    GetArrayBufferPartitionAllocator().init();
    GetGeneralPartitionAllocator().init();
    GetStringPartitionAllocator().init();
    s_partition_allocators_initialized = true;
  }
}

void* FX_ArrayBufferAllocate(size_t length) {
  return GetArrayBufferPartitionAllocator().root()->AllocFlags(
      pdfium::base::PartitionAllocZeroFill, length, "FXArrayBuffer");
}

void* FX_ArrayBufferAllocateUninitialized(size_t length) {
  return GetArrayBufferPartitionAllocator().root()->Alloc(length,
                                                          "FXArrayBuffer");
}

void FX_ArrayBufferFree(void* data) {
  GetArrayBufferPartitionAllocator().root()->Free(data);
}

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
    pdfium::base::PartitionFree(ptr);
}
