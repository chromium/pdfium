// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/invalid_seekable_read_stream.h"

InvalidSeekableReadStream::InvalidSeekableReadStream(FX_FILESIZE data_size)
    : data_size_(data_size) {}

InvalidSeekableReadStream::~InvalidSeekableReadStream() = default;

bool InvalidSeekableReadStream::ReadBlockAtOffset(void* buffer,
                                                  FX_FILESIZE offset,
                                                  size_t size) {
  return false;
}

FX_FILESIZE InvalidSeekableReadStream::GetSize() {
  return data_size_;
}
