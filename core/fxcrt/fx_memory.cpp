// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stdlib.h>  // For abort().

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/debug/alias.h"

pdfium::base::PartitionAllocatorGeneric gArrayBufferPartitionAllocator;
pdfium::base::PartitionAllocatorGeneric gGeneralPartitionAllocator;
pdfium::base::PartitionAllocatorGeneric gStringPartitionAllocator;

void FXMEM_InitializePartitionAlloc() {
  static bool s_gPartitionAllocatorsInitialized = false;
  if (!s_gPartitionAllocatorsInitialized) {
    pdfium::base::PartitionAllocGlobalInit(FX_OutOfMemoryTerminate);
    gArrayBufferPartitionAllocator.init();
    gGeneralPartitionAllocator.init();
    gStringPartitionAllocator.init();
    s_gPartitionAllocatorsInitialized = true;
  }
}

void* FXMEM_DefaultAlloc(size_t byte_size) {
  return pdfium::base::PartitionAllocGenericFlags(
      gGeneralPartitionAllocator.root(), pdfium::base::PartitionAllocReturnNull,
      byte_size, "GeneralPartition");
}

void* FXMEM_DefaultCalloc(size_t num_elems, size_t byte_size) {
  return FX_SafeAlloc(num_elems, byte_size);
}

void* FXMEM_DefaultRealloc(void* pointer, size_t new_size) {
  return pdfium::base::PartitionReallocGenericFlags(
      gGeneralPartitionAllocator.root(), pdfium::base::PartitionAllocReturnNull,
      pointer, new_size, "GeneralPartition");
}

void FXMEM_DefaultFree(void* pointer) {
  pdfium::base::PartitionFree(pointer);
}

NEVER_INLINE void FX_OutOfMemoryTerminate() {
  // Convince the linker this should not be folded with similar functions using
  // Identical Code Folding.
  static int make_this_function_aliased = 0xbd;
  pdfium::base::debug::Alias(&make_this_function_aliased);

  // Termimate cleanly if we can, else crash at a specific address (0xbd).
  abort();
#ifndef _WIN32
  reinterpret_cast<void (*)()>(0xbd)();
#endif
}
