// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cfx_v8_array_buffer_allocator.h"

#include "core/fxcrt/fx_memory.h"

CFX_V8ArrayBufferAllocator::CFX_V8ArrayBufferAllocator() = default;

CFX_V8ArrayBufferAllocator::~CFX_V8ArrayBufferAllocator() = default;

// NOTE: Under V8 sandbox mode, defer NewDefaultAllocator() call until
// first use, since V8 must be initialized first for it to succeed, but
// we need the allocator in order to initialize V8.

void* CFX_V8ArrayBufferAllocator::Allocate(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
#ifdef V8_ENABLE_SANDBOX
  if (!wrapped_) {
    wrapped_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
  }
  return wrapped_->Allocate(length);
#else   // V8_ENABLE_SANDBOX
  return FX_ArrayBufferAllocate(length);
#endif  // V8_ENABLE_SANDBOX
}

void* CFX_V8ArrayBufferAllocator::AllocateUninitialized(size_t length) {
  if (length > kMaxAllowedBytes)
    return nullptr;
#ifdef V8_ENABLE_SANDBOX
  if (!wrapped_) {
    wrapped_.reset(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
  }
  return wrapped_->AllocateUninitialized(length);
#else  // V8_ENABLE_SANDBOX
  return FX_ArrayBufferAllocateUninitialized(length);
#endif
}

void CFX_V8ArrayBufferAllocator::Free(void* data, size_t length) {
#ifdef V8_ENABLE_SANDBOX
  if (wrapped_) {
    wrapped_->Free(data, length);
  }
#else  // V8_ENABLE_SANDBOX
  FX_ArrayBufferFree(data);
#endif
}
