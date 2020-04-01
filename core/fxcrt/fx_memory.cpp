// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include <stdlib.h>  // For abort().

#include <limits>

#include "build/build_config.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/debug/alias.h"

pdfium::base::PartitionAllocatorGeneric& GetArrayBufferPartitionAllocator() {
  static pdfium::base::PartitionAllocatorGeneric s_array_buffer_allocator;
  return s_array_buffer_allocator;
}

pdfium::base::PartitionAllocatorGeneric& GetGeneralPartitionAllocator() {
  static pdfium::base::PartitionAllocatorGeneric s_general_allocator;
  return s_general_allocator;
}

pdfium::base::PartitionAllocatorGeneric& GetStringPartitionAllocator() {
  static pdfium::base::PartitionAllocatorGeneric s_string_allocator;
  return s_string_allocator;
}

void FXMEM_InitializePartitionAlloc() {
  static bool s_partition_allocators_initialized = false;
  if (!s_partition_allocators_initialized) {
    pdfium::base::PartitionAllocGlobalInit(FX_OutOfMemoryTerminate);
    GetArrayBufferPartitionAllocator().init();
    GetGeneralPartitionAllocator().init();
    GetStringPartitionAllocator().init();
    s_partition_allocators_initialized = true;
  }
}

void* FXMEM_DefaultAlloc(size_t byte_size) {
  return pdfium::base::PartitionAllocGenericFlags(
      GetGeneralPartitionAllocator().root(),
      pdfium::base::PartitionAllocReturnNull, byte_size, "GeneralPartition");
}

void* FXMEM_DefaultCalloc(size_t num_elems, size_t byte_size) {
  return internal::Calloc(num_elems, byte_size);
}

void* FXMEM_DefaultRealloc(void* pointer, size_t new_size) {
  return pdfium::base::PartitionReallocGenericFlags(
      GetGeneralPartitionAllocator().root(),
      pdfium::base::PartitionAllocReturnNull, pointer, new_size,
      "GeneralPartition");
}

void FXMEM_DefaultFree(void* pointer) {
  pdfium::base::PartitionFree(pointer);
}

NOINLINE void FX_OutOfMemoryTerminate() {
  // Convince the linker this should not be folded with similar functions using
  // Identical Code Folding.
  static int make_this_function_aliased = 0xbd;
  pdfium::base::debug::Alias(&make_this_function_aliased);

  // Termimate cleanly.
  abort();
}

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

void* AllocOrDie(size_t num_members, size_t member_size) {
  void* result = Alloc(num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate();  // Never returns.

  return result;
}

void* AllocOrDie2D(size_t w, size_t h, size_t member_size) {
  if (w >= std::numeric_limits<size_t>::max() / h)
    FX_OutOfMemoryTerminate();  // Never returns.

  return AllocOrDie(w * h, member_size);
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

void* CallocOrDie(size_t num_members, size_t member_size) {
  void* result = Calloc(num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate();  // Never returns.

  return result;
}

void* CallocOrDie2D(size_t w, size_t h, size_t member_size) {
  if (w >= std::numeric_limits<size_t>::max() / h)
    FX_OutOfMemoryTerminate();  // Never returns.

  return CallocOrDie(w * h, member_size);
}

void* ReallocOrDie(void* ptr, size_t num_members, size_t member_size) {
  void* result = Realloc(ptr, num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate();  // Never returns.

  return result;
}

}  // namespace internal

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
