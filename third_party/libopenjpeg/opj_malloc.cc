// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Deliberately not including opj_malloc.h, which has poisoned malloc and
// friends.

#include "core/fxcrt/fx_memory.h"
#include "third_party/base/memory/aligned_memory.h"

extern "C" {

void* opj_malloc(size_t size) {
  return FXMEM_DefaultAlloc(size);
}

void* opj_calloc(size_t numOfElements, size_t sizeOfElements) {
  return FXMEM_DefaultCalloc(numOfElements, sizeOfElements);
}

void* opj_aligned_malloc(size_t size) {
  return size ? pdfium::base::AlignedAlloc(size, 16) : nullptr;
}

void opj_aligned_free(void* ptr) {
  pdfium::base::AlignedFree(ptr);
}

void* opj_aligned_32_malloc(size_t size) {
  return size ? pdfium::base::AlignedAlloc(size, 32) : nullptr;
}

void* opj_realloc(void* m, size_t s) {
  return FXMEM_DefaultRealloc(m, s);
}

void opj_free(void* m) {
  if (m)
    FXMEM_DefaultFree(m);
}

}  // extern "C"
