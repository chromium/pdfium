// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_v8_array_buffer_allocator.h"

#include "core/fxcrt/fx_memory.h"

void* CFX_V8ArrayBufferAllocator::Allocate(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return FX_ArrayBufferAllocate(length);
}

void* CFX_V8ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return FX_ArrayBufferAllocateUninitialized(length);
}

void CFX_V8ArrayBufferAllocator::Free(void* data, size_t length) {
  FX_ArrayBufferFree(data);
}
