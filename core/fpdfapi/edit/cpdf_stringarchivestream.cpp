// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_stringarchivestream.h"

#include <sstream>

#include "core/fxcrt/span_util.h"
#include "third_party/base/notreached.h"

CPDF_StringArchiveStream::CPDF_StringArchiveStream(fxcrt::ostringstream* stream)
    : stream_(stream) {}

CPDF_StringArchiveStream::~CPDF_StringArchiveStream() = default;

FX_FILESIZE CPDF_StringArchiveStream::CurrentOffset() const {
  NOTREACHED_NORETURN();
}

bool CPDF_StringArchiveStream::WriteBlock(pdfium::span<const uint8_t> buffer) {
  auto chars = fxcrt::reinterpret_span<const char>(buffer);
  stream_->write(chars.data(), chars.size());
  return true;
}
