// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/codec/cfx_codec_memory.h"

#include <algorithm>

CFX_CodecMemory::CFX_CodecMemory(pdfium::span<uint8_t> buffer)
    : buffer_(buffer) {}

CFX_CodecMemory::~CFX_CodecMemory() = default;

bool CFX_CodecMemory::Seek(size_t pos) {
  if (pos > buffer_.size())
    return false;

  pos_ = pos;
  return true;
}

size_t CFX_CodecMemory::ReadBlock(void* buffer, size_t size) {
  if (!buffer || !size || IsEOF())
    return 0;

  size_t bytes_to_read = std::min(size, buffer_.size() - pos_);
  memcpy(buffer, &buffer_[pos_], bytes_to_read);
  pos_ += bytes_to_read;
  return bytes_to_read;
}
