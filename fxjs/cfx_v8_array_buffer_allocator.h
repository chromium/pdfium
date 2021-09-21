// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CFX_V8_ARRAY_BUFFER_ALLOCATOR_H_
#define FXJS_CFX_V8_ARRAY_BUFFER_ALLOCATOR_H_

#include <stddef.h>

#include "v8/include/v8-array-buffer.h"

class CFX_V8ArrayBufferAllocator final : public v8::ArrayBuffer::Allocator {
  static const size_t kMaxAllowedBytes = 0x10000000;
  void* Allocate(size_t length) override;
  void* AllocateUninitialized(size_t length) override;
  void Free(void* data, size_t length) override;
};

#endif  // FXJS_CFX_V8_ARRAY_BUFFER_ALLOCATOR_H_
