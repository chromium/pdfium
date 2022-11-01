// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_read_only_span_stream.h"

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "third_party/base/numerics/safe_conversions.h"

CFX_ReadOnlySpanStream::CFX_ReadOnlySpanStream(pdfium::span<const uint8_t> span)
    : span_(span) {}

CFX_ReadOnlySpanStream::~CFX_ReadOnlySpanStream() = default;

FX_FILESIZE CFX_ReadOnlySpanStream::GetSize() {
  return pdfium::base::checked_cast<FX_FILESIZE>(span_.size());
}

bool CFX_ReadOnlySpanStream::ReadBlockAtOffset(pdfium::span<uint8_t> buffer,
                                               FX_FILESIZE offset) {
  if (buffer.empty() || offset < 0)
    return false;

  FX_SAFE_SIZE_T pos = buffer.size();
  pos += offset;
  if (!pos.IsValid() || pos.ValueOrDie() > span_.size())
    return false;

  fxcrt::spancpy(
      buffer,
      span_.subspan(pdfium::base::checked_cast<size_t>(offset), buffer.size()));
  return true;
}
