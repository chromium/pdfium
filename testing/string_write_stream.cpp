// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/string_write_stream.h"

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/widestring.h"

StringWriteStream::StringWriteStream() = default;

StringWriteStream::~StringWriteStream() = default;

bool StringWriteStream::WriteBlock(pdfium::span<const uint8_t> buffer) {
  auto chars = pdfium::as_chars(buffer);
  stream_.write(chars.data(), chars.size());
  return true;
}
