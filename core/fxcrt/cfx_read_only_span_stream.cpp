// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_read_only_span_stream.h"

#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/numerics/safe_conversions.h"

CFX_ReadOnlySpanStream::CFX_ReadOnlySpanStream(pdfium::span<const uint8_t> span)
    : span_(span) {}

CFX_ReadOnlySpanStream::~CFX_ReadOnlySpanStream() = default;

FX_FILESIZE CFX_ReadOnlySpanStream::GetSize() {
  return pdfium::base::checked_cast<FX_FILESIZE>(span_.size());
}

bool CFX_ReadOnlySpanStream::ReadBlockAtOffset(void* buffer,
                                               FX_FILESIZE offset,
                                               size_t size) {
  if (!buffer || offset < 0 || size == 0)
    return false;

  FX_SAFE_SIZE_T pos = size;
  pos += offset;
  if (!pos.IsValid() || pos.ValueOrDie() > span_.size())
    return false;

  auto copy_span =
      span_.subspan(pdfium::base::checked_cast<size_t>(offset), size);
  memcpy(buffer, copy_span.data(), copy_span.size());
  return true;
}
