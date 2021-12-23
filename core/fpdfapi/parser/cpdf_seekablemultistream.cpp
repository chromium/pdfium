// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_seekablemultistream.h"

#include <algorithm>

#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "third_party/base/notreached.h"

CPDF_SeekableMultiStream::CPDF_SeekableMultiStream(
    const std::vector<const CPDF_Stream*>& streams) {
  for (const CPDF_Stream* pStream : streams) {
    m_Data.push_back(pdfium::MakeRetain<CPDF_StreamAcc>(pStream));
    m_Data.back()->LoadAllDataFiltered();
  }
}

CPDF_SeekableMultiStream::~CPDF_SeekableMultiStream() = default;

FX_FILESIZE CPDF_SeekableMultiStream::GetSize() {
  FX_SAFE_FILESIZE dwSize = 0;
  for (const auto& acc : m_Data)
    dwSize += acc->GetSize();
  return dwSize.ValueOrDie();
}

bool CPDF_SeekableMultiStream::ReadBlockAtOffset(void* buffer,
                                                 FX_FILESIZE offset,
                                                 size_t size) {
  int32_t iCount = fxcrt::CollectionSize<int32_t>(m_Data);
  int32_t index = 0;
  while (index < iCount) {
    const auto& acc = m_Data[index];
    FX_FILESIZE dwSize = acc->GetSize();
    if (offset < dwSize)
      break;

    offset -= dwSize;
    index++;
  }
  auto buffer_span = pdfium::make_span(static_cast<uint8_t*>(buffer), size);
  while (index < iCount) {
    auto acc_span = m_Data[index]->GetSpan();
    size_t dwRead =
        std::min<size_t>(buffer_span.size(), acc_span.size() - offset);
    fxcrt::spancpy(buffer_span, acc_span.subspan(offset, dwRead));
    buffer_span = buffer_span.subspan(dwRead);
    if (buffer_span.empty())
      return true;

    offset = 0;
    index++;
  }
  return false;
}

size_t CPDF_SeekableMultiStream::ReadBlock(void* buffer, size_t size) {
  NOTREACHED();
  return 0;
}

FX_FILESIZE CPDF_SeekableMultiStream::GetPosition() {
  return 0;
}

bool CPDF_SeekableMultiStream::IsEOF() {
  return false;
}

bool CPDF_SeekableMultiStream::Flush() {
  NOTREACHED();
  return false;
}

bool CPDF_SeekableMultiStream::WriteBlockAtOffset(const void* pData,
                                                  FX_FILESIZE offset,
                                                  size_t size) {
  NOTREACHED();
  return false;
}
