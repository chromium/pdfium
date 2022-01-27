// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_MEMORY_H_
#define CORE_FXCRT_FX_MEMORY_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// For external C libraries to malloc through PDFium. These may return nullptr.
void* FXMEM_DefaultAlloc(size_t byte_size);
void* FXMEM_DefaultCalloc(size_t num_elems, size_t byte_size);
void* FXMEM_DefaultRealloc(void* pointer, size_t new_size);
void FXMEM_DefaultFree(void* pointer);

#ifdef __cplusplus
}  // extern "C"

#include "third_party/base/compiler_specific.h"

namespace pdfium {
namespace base {
class PartitionAllocatorGeneric;
}  // namespace base
}  // namespace pdfium

pdfium::base::PartitionAllocatorGeneric& GetArrayBufferPartitionAllocator();
pdfium::base::PartitionAllocatorGeneric& GetGeneralPartitionAllocator();
pdfium::base::PartitionAllocatorGeneric& GetStringPartitionAllocator();

void FXMEM_InitializePartitionAlloc();
NOINLINE void FX_OutOfMemoryTerminate(size_t size);

// General Partition Allocators.

// These never return nullptr, and must return cleared memory.
#define FX_Alloc(type, size) \
  static_cast<type*>(pdfium::internal::CallocOrDie(size, sizeof(type)))
#define FX_Alloc2D(type, w, h) \
  static_cast<type*>(pdfium::internal::CallocOrDie2D(w, h, sizeof(type)))
#define FX_Realloc(type, ptr, size) \
  static_cast<type*>(pdfium::internal::ReallocOrDie(ptr, size, sizeof(type)))

// May return nullptr, but returns cleared memory otherwise.
#define FX_TryAlloc(type, size) \
  static_cast<type*>(pdfium::internal::Calloc(size, sizeof(type)))
#define FX_TryRealloc(type, ptr, size) \
  static_cast<type*>(pdfium::internal::Realloc(ptr, size, sizeof(type)))

// These never return nullptr, but return uninitialized memory.
// TOOD(thestig): Add FX_TryAllocUninit() if there is a use case.
#define FX_AllocUninit(type, size) \
  static_cast<type*>(pdfium::internal::AllocOrDie(size, sizeof(type)))
#define FX_AllocUninit2D(type, w, h) \
  static_cast<type*>(pdfium::internal::AllocOrDie2D(w, h, sizeof(type)))

// String Partition Allocators.

// This never returns nullptr, but returns uninitialized memory.
#define FX_StringAlloc(type, size) \
  static_cast<type*>(pdfium::internal::StringAllocOrDie(size, sizeof(type)))

// Free accepts memory from all of the above.
void FX_Free(void* ptr);

namespace pdfium {
namespace internal {

// General partition.
void* Alloc(size_t num_members, size_t member_size);
void* AllocOrDie(size_t num_members, size_t member_size);
void* AllocOrDie2D(size_t w, size_t h, size_t member_size);
void* Calloc(size_t num_members, size_t member_size);
void* Realloc(void* ptr, size_t num_members, size_t member_size);
void* CallocOrDie(size_t num_members, size_t member_size);
void* CallocOrDie2D(size_t w, size_t h, size_t member_size);
void* ReallocOrDie(void* ptr, size_t num_members, size_t member_size);

// String partition.
void* StringAlloc(size_t num_members, size_t member_size);
void* StringAllocOrDie(size_t num_members, size_t member_size);

}  // namespace internal
}  // namespace pdfium

// Force stack allocation of a class. Classes that do complex work in a
// destructor, such as the flushing of buffers, should be declared as
// stack-allocated as possible, since future memory allocation schemes
// may not run destructors in a predictable manner if an instance is
// heap-allocated.
#define FX_STACK_ALLOCATED()           \
  void* operator new(size_t) = delete; \
  void* operator new(size_t, void*) = delete

// Round up to the power-of-two boundary N.
template <int N, typename T>
inline T FxAlignToBoundary(T size) {
  static_assert(N > 0 && (N & (N - 1)) == 0, "Not non-zero power of two");
  return (size + (N - 1)) & ~(N - 1);
}

#endif  // __cplusplus

#endif  // CORE_FXCRT_FX_MEMORY_H_
