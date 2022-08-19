// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_memory.h"

#include <stdlib.h>

#include "build/build_config.h"
#include "core/fxcrt/fx_safe_types.h"

namespace pdfium {
namespace internal {

void* Alloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;
  return malloc(total.ValueOrDie());
}

void* Calloc(size_t num_members, size_t member_size) {
  return calloc(num_members, member_size);
}

void* Realloc(void* ptr, size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T size = num_members;
  size *= member_size;
  if (!size.IsValid())
    return nullptr;
  return realloc(ptr, size.ValueOrDie());
}

void* StringAlloc(size_t num_members, size_t member_size) {
  FX_SAFE_SIZE_T total = member_size;
  total *= num_members;
  if (!total.IsValid())
    return nullptr;
  return malloc(total.ValueOrDie());
}

}  // namespace internal
}  // namespace pdfium

void FX_InitializeMemoryAllocators() {}

void* FX_ArrayBufferAllocate(size_t length) {
  void* result = calloc(length, 1);
  if (!result)
    FX_OutOfMemoryTerminate(length);
  return result;
}

void* FX_ArrayBufferAllocateUninitialized(size_t length) {
  void* result = malloc(length);
  if (!result)
    FX_OutOfMemoryTerminate(length);
  return result;
}

void FX_ArrayBufferFree(void* data) {
  free(data);
}

void FX_Free(void* ptr) {
  free(ptr);
}
