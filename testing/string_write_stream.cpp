// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/string_write_stream.h"

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/widestring.h"

StringWriteStream::StringWriteStream() = default;

StringWriteStream::~StringWriteStream() = default;

bool StringWriteStream::WriteBlock(pdfium::span<const uint8_t> buffer) {
  stream_.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
  return true;
}
