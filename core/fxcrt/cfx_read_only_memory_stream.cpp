// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_read_only_memory_stream.h"

#include <utility>

#include "core/fxcrt/cfx_read_only_span_stream.h"
#include "third_party/base/span.h"

CFX_ReadOnlyMemoryStream::CFX_ReadOnlyMemoryStream(
    std::unique_ptr<uint8_t, FxFreeDeleter> data,
    size_t size)
    : m_data(std::move(data)),
      m_stream(pdfium::MakeRetain<CFX_ReadOnlySpanStream>(
          pdfium::make_span(m_data.get(), size))) {}

CFX_ReadOnlyMemoryStream::~CFX_ReadOnlyMemoryStream() = default;

FX_FILESIZE CFX_ReadOnlyMemoryStream::GetSize() {
  return m_stream->GetSize();
}

bool CFX_ReadOnlyMemoryStream::ReadBlockAtOffset(void* buffer,
                                                 FX_FILESIZE offset,
                                                 size_t size) {
  return m_stream->ReadBlockAtOffset(buffer, offset, size);
}
