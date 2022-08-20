// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/cfx_read_only_string_stream.h"

#include <utility>

#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "third_party/base/span.h"

CFX_ReadOnlyStringStream::CFX_ReadOnlyStringStream(ByteString data)
    : data_(std::move(data)),
      stream_(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(data_.raw_span())) {}

CFX_ReadOnlyStringStream::~CFX_ReadOnlyStringStream() = default;

FX_FILESIZE CFX_ReadOnlyStringStream::GetSize() {
  return stream_->GetSize();
}

bool CFX_ReadOnlyStringStream::ReadBlockAtOffset(void* buffer,
                                                 FX_FILESIZE offset,
                                                 size_t size) {
  return stream_->ReadBlockAtOffset(buffer, offset, size);
}
