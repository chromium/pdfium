// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include <stdlib.h>  // For abort().

#include <limits>

#include "build/build_config.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"
#include "third_party/base/debug/alias.h"
#include "third_party/base/no_destructor.h"

#if BUILDFLAG(IS_WIN)
#include <windows.h>
#endif

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
  return pdfium::internal::Calloc(num_elems, byte_size);
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

NOINLINE void FX_OutOfMemoryTerminate(size_t size) {
  // Convince the linker this should not be folded with similar functions using
  // Identical Code Folding.
  static int make_this_function_aliased = 0xbd;
  pdfium::base::debug::Alias(&make_this_function_aliased);

#if BUILDFLAG(IS_WIN)
  // The same custom Windows exception code used in Chromium and Breakpad.
  constexpr DWORD kOomExceptionCode = 0xe0000008;
  ULONG_PTR exception_args[] = {size};
  ::RaiseException(kOomExceptionCode, EXCEPTION_NONCONTINUABLE,
                   pdfium::size(exception_args), exception_args);
#endif

  // Terminate cleanly.
  abort();
}

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

void* AllocOrDie(size_t num_members, size_t member_size) {
  void* result = Alloc(num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate(0);  // Never returns.

  return result;
}

void* AllocOrDie2D(size_t w, size_t h, size_t member_size) {
  if (w >= std::numeric_limits<size_t>::max() / h)
    FX_OutOfMemoryTerminate(0);  // Never returns.

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
    FX_OutOfMemoryTerminate(0);  // Never returns.

  return result;
}

void* CallocOrDie2D(size_t w, size_t h, size_t member_size) {
  if (w >= std::numeric_limits<size_t>::max() / h)
    FX_OutOfMemoryTerminate(0);  // Never returns.

  return CallocOrDie(w * h, member_size);
}

void* ReallocOrDie(void* ptr, size_t num_members, size_t member_size) {
  void* result = Realloc(ptr, num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate(0);  // Never returns.

  return result;
}

void* StringAllocOrDie(size_t num_members, size_t member_size) {
  void* result = StringAlloc(num_members, member_size);
  if (!result)
    FX_OutOfMemoryTerminate(0);  // Never returns.

  return result;
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
