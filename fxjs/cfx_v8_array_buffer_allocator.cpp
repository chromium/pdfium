// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_v8_array_buffer_allocator.h"

#include "core/fxcrt/fx_memory.h"
#include "third_party/base/allocator/partition_allocator/partition_alloc.h"

void* CFX_V8ArrayBufferAllocator::Allocate(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return GetArrayBufferPartitionAllocator().root()->AllocFlags(
      pdfium::base::PartitionAllocZeroFill, length, "CFX_V8ArrayBuffer");
}

void* CFX_V8ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
  return GetArrayBufferPartitionAllocator().root()->Alloc(length,
                                                          "CFX_V8ArrayBuffer");
}

void CFX_V8ArrayBufferAllocator::Free(void* data, size_t length) {
  GetArrayBufferPartitionAllocator().root()->Free(data);
}
