// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_stringarchivestream.h"

#include <sstream>

#include "third_party/base/notreached.h"

CPDF_StringArchiveStream::CPDF_StringArchiveStream(fxcrt::ostringstream* stream)
    : stream_(stream) {}

CPDF_StringArchiveStream::~CPDF_StringArchiveStream() = default;

FX_FILESIZE CPDF_StringArchiveStream::CurrentOffset() const {
  NOTREACHED();
  return false;
}

bool CPDF_StringArchiveStream::WriteBlock(pdfium::span<const uint8_t> buffer) {
  stream_->write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
  return true;
}
