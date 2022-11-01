// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/invalid_seekable_read_stream.h"

InvalidSeekableReadStream::InvalidSeekableReadStream(FX_FILESIZE data_size)
    : data_size_(data_size) {}

InvalidSeekableReadStream::~InvalidSeekableReadStream() = default;

bool InvalidSeekableReadStream::ReadBlockAtOffset(pdfium::span<uint8_t> buffer,
                                                  FX_FILESIZE offset) {
  return false;
}

FX_FILESIZE InvalidSeekableReadStream::GetSize() {
  return data_size_;
}
