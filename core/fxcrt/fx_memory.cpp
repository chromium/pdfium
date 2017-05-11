// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include <stdlib.h>  // For abort().

pdfium::base::PartitionAllocatorGeneric gArrayBufferPartitionAllocator;
pdfium::base::PartitionAllocatorGeneric gStringPartitionAllocator;

void FXMEM_InitializePartitionAlloc() {
  static bool s_gPartitionAllocatorsInitialized = false;
  if (!s_gPartitionAllocatorsInitialized) {
    pdfium::base::PartitionAllocGlobalInit(FX_OutOfMemoryTerminate);
    gArrayBufferPartitionAllocator.init();
    gStringPartitionAllocator.init();
    s_gPartitionAllocatorsInitialized = true;
  }
}

void* FXMEM_DefaultAlloc(size_t byte_size, int flags) {
  return (void*)malloc(byte_size);
}

void* FXMEM_DefaultCalloc(size_t num_elems, size_t byte_size) {
  return calloc(num_elems, byte_size);
}

void* FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags) {
  return realloc(pointer, new_size);
}

void FXMEM_DefaultFree(void* pointer, int flags) {
  free(pointer);
}

NEVER_INLINE void FX_OutOfMemoryTerminate() {
  // Termimate cleanly if we can, else crash at a specific address (0xbd).
  abort();
#ifndef _WIN32
  reinterpret_cast<void (*)()>(0xbd)();
#endif
}
